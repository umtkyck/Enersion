/**
 ******************************************************************************
 * @file           : ad4114.c
 * @brief          : AD4114BCPZ 24-bit Sigma-Delta ADC Driver Implementation
 ******************************************************************************
 */

#include "ad4114.h"
#include "debug_uart.h"
#include <string.h>

/* Private defines */
#define AD4114_SPI_TIMEOUT  100  /* SPI timeout in ms */
#define AD4114_RESET_DELAY  10   /* Reset delay in ms */

/**
 * @brief  Chip Select Low - NOT USED (CS tied to GND)
 */
void AD4114_CS_Low(AD4114_Device_t *dev)
{
    (void)dev;
    /* CS is hardwired to GND - no action needed */
}

/**
 * @brief  Chip Select High - NOT USED (CS tied to GND)
 */
void AD4114_CS_High(AD4114_Device_t *dev)
{
    (void)dev;
    /* CS is hardwired to GND - no action needed */
}

/**
 * @brief  Reset AD4114 via SPI
 */
HAL_StatusTypeDef AD4114_Reset(AD4114_Device_t *dev)
{
    uint8_t reset_sequence[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    AD4114_CS_Low(dev);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(dev->hspi, reset_sequence, 8, AD4114_SPI_TIMEOUT);
    AD4114_CS_High(dev);
    
    HAL_Delay(AD4114_RESET_DELAY);
    
    return status;
}

/**
 * @brief  Write to AD4114 register
 * @note   Communication byte format: [WEN=0][R/W=0][RA5:RA0]
 */
HAL_StatusTypeDef AD4114_WriteRegister(AD4114_Device_t *dev, uint8_t reg, uint32_t value, uint8_t bytes)
{
    uint8_t tx_buf[5] = {0};
    uint8_t rx_buf[5] = {0};
    
    /* Communication register: Write command (WEN=0, R/W=0, RA=reg) */
    tx_buf[0] = AD4114_COMM_REG_WEN | AD4114_COMM_REG_WR | (reg & 0x3F);  /* WEN=0, R/W=0 */
    
    /* Pack data bytes (MSB first) */
    for (uint8_t i = 0; i < bytes; i++) {
        tx_buf[i + 1] = (uint8_t)((value >> (8 * (bytes - 1 - i))) & 0xFF);
    }
    
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(dev->hspi, tx_buf, rx_buf, bytes + 1, AD4114_SPI_TIMEOUT);
    
    return status;
}

/**
 * @brief  Read from AD4114 register
 * @note   Communication byte format: [WEN=0][R/W=1][RA5:RA0]
 */
HAL_StatusTypeDef AD4114_ReadRegister(AD4114_Device_t *dev, uint8_t reg, uint32_t *value, uint8_t bytes)
{
    uint8_t tx_buf[5] = {0};
    uint8_t rx_buf[5] = {0};
    
    /* Communication register: Read command (WEN=0, R/W=1, RA=reg) */
    tx_buf[0] = AD4114_COMM_REG_RD | (reg & 0x3F);  /* Read: bit6=1 */
    
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(dev->hspi, tx_buf, rx_buf, bytes + 1, AD4114_SPI_TIMEOUT);
    
    if (status == HAL_OK) {
        /* Unpack data bytes (MSB first) */
        *value = 0;
        for (uint8_t i = 0; i < bytes; i++) {
            *value |= ((uint32_t)rx_buf[i + 1] << (8 * (bytes - 1 - i)));
        }
    }
    
    return status;
}

/**
 * @brief  Read AD4114 ID register
 * @note   ID register is 2 bytes: [Device ID][Silicon Revision]
 *         AD4114 Device ID = 0x30xx
 */
HAL_StatusTypeDef AD4114_ReadID(AD4114_Device_t *dev, uint16_t *id)
{
    uint32_t value = 0;
    HAL_StatusTypeDef status = AD4114_ReadRegister(dev, AD4114_REG_ID, &value, 2);
    
    if (status == HAL_OK) {
        *id = (uint16_t)value;
    }
    
    return status;
}

/**
 * @brief  Initialize AD4114
 * @note   Register sizes (from AD4114 datasheet):
 *         - ADCMODE (0x01): 2 bytes
 *         - IFMODE (0x02): 2 bytes
 *         - ID (0x07): 2 bytes
 *         - CHx (0x10-0x1F): 2 bytes
 *         - SETUPCONx (0x20-0x27): 2 bytes
 *         - FILTCONx (0x28-0x2F): 2 bytes (NOT 3!)
 *         - DATA (0x04): 3 bytes
 */
HAL_StatusTypeDef AD4114_Init(AD4114_Device_t *dev)
{
    HAL_StatusTypeDef status;
    uint16_t chip_id;
    uint32_t reg_value;
    
    DEBUG_INFO("[AD4114-%d] Starting initialization...", dev->device_id);
    DEBUG_INFO("[AD4114-%d] SPI Instance: %s", dev->device_id, 
               (dev->hspi->Instance == SPI1) ? "SPI1" : 
               (dev->hspi->Instance == SPI4) ? "SPI4" : "Unknown");
    
    /* Reset the device - send 64 SCLKs with DIN high */
    DEBUG_INFO("[AD4114-%d] Sending reset sequence (64 clocks, DIN=1)...", dev->device_id);
    status = AD4114_Reset(dev);
    if (status != HAL_OK) {
        DEBUG_ERROR("[AD4114-%d] Reset FAILED (SPI error)", dev->device_id);
        return status;
    }
    DEBUG_INFO("[AD4114-%d] Reset OK", dev->device_id);
    
    /* Longer delay after reset for AD4114 to stabilize */
    HAL_Delay(50);
    
    /* Read and verify chip ID (2 bytes) */
    DEBUG_INFO("[AD4114-%d] Reading Chip ID...", dev->device_id);
    status = AD4114_ReadID(dev, &chip_id);
    if (status != HAL_OK) {
        DEBUG_ERROR("[AD4114-%d] Read ID FAILED", dev->device_id);
        return status;
    }
    
    /* AD4114 ID should be 0x30xx (0x30 = AD4114 device ID) */
    DEBUG_INFO("[AD4114-%d] Chip ID: 0x%04X (Expected: 0x30xx)", dev->device_id, chip_id);
    
    if (chip_id == 0xFFFF) {
        DEBUG_ERROR("[AD4114-%d] Chip ID = 0xFFFF - SPI NOT WORKING! Check wiring.", dev->device_id);
        DEBUG_ERROR("[AD4114-%d] Possible causes: MISO not connected, wrong SPI pins", dev->device_id);
        /* Continue anyway for debugging */
    } else if (chip_id == 0x0000) {
        DEBUG_ERROR("[AD4114-%d] Chip ID = 0x0000 - SPI NOT WORKING! Check wiring.", dev->device_id);
        DEBUG_ERROR("[AD4114-%d] Possible causes: CLK not working, chip not powered", dev->device_id);
    } else if ((chip_id & 0xFF00) != 0x3000) {
        DEBUG_WARNING("[AD4114-%d] Unexpected ID! Expected 0x30xx for AD4114", dev->device_id);
    } else {
        DEBUG_INFO("[AD4114-%d] Chip ID VALID - AD4114 detected!", dev->device_id);
    }
    
    /* Read STATUS register to check for errors */
    status = AD4114_ReadRegister(dev, AD4114_REG_STATUS, &reg_value, 1);
    DEBUG_INFO("[AD4114-%d] STATUS reg: 0x%02lX (RDY=%d, ADC_ERR=%d, REG_ERR=%d)", 
               dev->device_id, reg_value,
               (int)((reg_value >> 7) & 1),
               (int)((reg_value >> 6) & 1),
               (int)((reg_value >> 4) & 1));
    
    /* Configure ADC Mode Register (0x01, 2 bytes) */
    uint32_t adcmode = (1 << 15);  /* REF_EN = 1, MODE = continuous */
    DEBUG_INFO("[AD4114-%d] Writing ADCMODE=0x%04lX...", dev->device_id, adcmode);
    status = AD4114_WriteRegister(dev, AD4114_REG_ADCMODE, adcmode, 2);
    if (status != HAL_OK) {
        DEBUG_ERROR("[AD4114-%d] ADCMODE write FAILED", dev->device_id);
        return status;
    }
    /* Verify by reading back */
    status = AD4114_ReadRegister(dev, AD4114_REG_ADCMODE, &reg_value, 2);
    DEBUG_INFO("[AD4114-%d] ADCMODE readback: 0x%04lX", dev->device_id, reg_value);
    
    /* Configure Interface Mode Register (0x02, 2 bytes) */
    uint32_t ifmode = (1 << 6);  /* DATA_STAT = 1 */
    DEBUG_INFO("[AD4114-%d] Writing IFMODE=0x%04lX...", dev->device_id, ifmode);
    status = AD4114_WriteRegister(dev, AD4114_REG_IFMODE, ifmode, 2);
    if (status != HAL_OK) {
        DEBUG_ERROR("[AD4114-%d] IFMODE write FAILED", dev->device_id);
        return status;
    }
    status = AD4114_ReadRegister(dev, AD4114_REG_IFMODE, &reg_value, 2);
    DEBUG_INFO("[AD4114-%d] IFMODE readback: 0x%04lX", dev->device_id, reg_value);
    
    /* Configure Setup 0 Register (0x20, 2 bytes) */
    uint32_t setupcon0 = (2 << 4) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0);
    DEBUG_INFO("[AD4114-%d] Writing SETUP0=0x%04lX...", dev->device_id, setupcon0);
    status = AD4114_WriteRegister(dev, AD4114_REG_SETUPCON0, setupcon0, 2);
    if (status != HAL_OK) {
        DEBUG_ERROR("[AD4114-%d] SETUP0 write FAILED", dev->device_id);
        return status;
    }
    status = AD4114_ReadRegister(dev, AD4114_REG_SETUPCON0, &reg_value, 2);
    DEBUG_INFO("[AD4114-%d] SETUP0 readback: 0x%04lX", dev->device_id, reg_value);
    
    /* Configure Filter 0 Register (0x28, 2 bytes) */
    uint32_t filtcon0 = (0 << 8) | 20;  /* SINC5+SINC1, ODR=50 SPS */
    DEBUG_INFO("[AD4114-%d] Writing FILTCON0=0x%04lX...", dev->device_id, filtcon0);
    status = AD4114_WriteRegister(dev, AD4114_REG_FILTCON0, filtcon0, 2);
    if (status != HAL_OK) {
        DEBUG_ERROR("[AD4114-%d] FILTCON0 write FAILED", dev->device_id);
        return status;
    }
    status = AD4114_ReadRegister(dev, AD4114_REG_FILTCON0, &reg_value, 2);
    DEBUG_INFO("[AD4114-%d] FILTCON0 readback: 0x%04lX", dev->device_id, reg_value);
    
    /* Disable ALL channels first (important!) */
    DEBUG_INFO("[AD4114-%d] Disabling all channels...", dev->device_id);
    for (uint8_t i = 0; i < 16; i++) {
        AD4114_WriteRegister(dev, AD4114_REG_CH0 + i, 0x0000, 2);
    }
    
    /* Enable Channel 0 as default for testing */
    uint32_t ch0_config = (1 << 15) | (0 << 12) | (0 << 5) | 17;  /* EN=1, SETUP=0, AINP=0, AINM=17(COM) */
    DEBUG_INFO("[AD4114-%d] Writing CH0=0x%04lX (EN=1, AINP=0, AINM=COM)...", dev->device_id, ch0_config);
    status = AD4114_WriteRegister(dev, AD4114_REG_CH0, ch0_config, 2);
    if (status != HAL_OK) {
        DEBUG_ERROR("[AD4114-%d] CH0 write FAILED", dev->device_id);
        return status;
    }
    status = AD4114_ReadRegister(dev, AD4114_REG_CH0, &reg_value, 2);
    DEBUG_INFO("[AD4114-%d] CH0 readback: 0x%04lX (expected: 0x%04lX)", dev->device_id, reg_value, ch0_config);
    
    if (reg_value != ch0_config) {
        DEBUG_ERROR("[AD4114-%d] CH0 VERIFY FAILED! SPI write not working!", dev->device_id);
    }
    
    /* Read back a few other registers to verify SPI is working */
    status = AD4114_ReadRegister(dev, AD4114_REG_CH15, &reg_value, 2);
    DEBUG_INFO("[AD4114-%d] CH15 readback: 0x%04lX (should be 0x0000)", dev->device_id, reg_value);
    
    DEBUG_INFO("[AD4114-%d] Init COMPLETE", dev->device_id);
    
    return HAL_OK;
}

/**
 * @brief  Configure AD4114 channel
 * @param  channel: Channel number (0-15)
 * @param  ainp: Positive input (0-15)
 * @param  ainm: Negative input (16 = AINCOM, 17 = internal)
 * @param  setup: Setup number (0-7)
 */
HAL_StatusTypeDef AD4114_ConfigureChannel(AD4114_Device_t *dev, uint8_t channel, 
                                          uint8_t ainp, uint8_t ainm, uint8_t setup)
{
    if (channel > 15) return HAL_ERROR;
    
    uint32_t ch_config = AD4114_CH_EN | AD4114_CH_SETUP(setup) | 
                         ((ainp & 0x1F) << 5) | (ainm & 0x1F);
    
    return AD4114_WriteRegister(dev, AD4114_REG_CH0 + channel, ch_config, 2);
}

/**
 * @brief  Enable/Disable AD4114 channel
 */
HAL_StatusTypeDef AD4114_EnableChannel(AD4114_Device_t *dev, uint8_t channel, uint8_t enable)
{
    if (channel > 15) return HAL_ERROR;
    
    uint32_t ch_config;
    HAL_StatusTypeDef status = AD4114_ReadRegister(dev, AD4114_REG_CH0 + channel, &ch_config, 2);
    
    if (status == HAL_OK) {
        if (enable) {
            ch_config |= AD4114_CH_EN;
        } else {
            ch_config &= ~AD4114_CH_EN;
        }
        status = AD4114_WriteRegister(dev, AD4114_REG_CH0 + channel, ch_config, 2);
    }
    
    return status;
}

/**
 * @brief  Start continuous conversion
 */
HAL_StatusTypeDef AD4114_StartConversion(AD4114_Device_t *dev)
{
    /* Conversion starts automatically in continuous mode after channel config */
    return HAL_OK;
}

/**
 * @brief  Read conversion data with status
 * @param  data: Pointer to store 24-bit data (unsigned, unipolar mode)
 * @note   When DATA_STAT is enabled, data is 4 bytes: [DATA_MSB][DATA_MID][DATA_LSB][STATUS]
 *         Status byte: bit7=RDY, bit6=ADC_ERR, bit4=REG_ERR, bits3:0=channel
 */
HAL_StatusTypeDef AD4114_ReadData(AD4114_Device_t *dev, int32_t *data)
{
    uint8_t tx_buf[5] = {0};
    uint8_t rx_buf[5] = {0};
    
    /* Read command for DATA register (0x04) */
    tx_buf[0] = AD4114_COMM_REG_RD | AD4114_REG_DATA;
    
    /* Read 4 bytes: 3 bytes data + 1 byte status (DATA_STAT enabled) */
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(dev->hspi, tx_buf, rx_buf, 5, AD4114_SPI_TIMEOUT);
    
    if (status == HAL_OK) {
        /* Data is in rx_buf[1], rx_buf[2], rx_buf[3] (MSB first) */
        uint32_t raw_data = ((uint32_t)rx_buf[1] << 16) | 
                            ((uint32_t)rx_buf[2] << 8) | 
                            (uint32_t)rx_buf[3];
        
        /* For unipolar mode, data is unsigned 24-bit */
        *data = (int32_t)raw_data;
    } else {
        DEBUG_ERROR("[SPI-%d] ReadData FAILED", dev->device_id);
    }
    
    return status;
}

/**
 * @brief  Read specific channel (single conversion mode)
 * @param  channel: Channel number (0-15)
 * @param  data: Pointer to store conversion result
 * @note   This function enables only the requested channel, triggers conversion,
 *         waits for completion, and reads the result.
 */
HAL_StatusTypeDef AD4114_ReadChannel(AD4114_Device_t *dev, uint8_t channel, int32_t *data)
{
    HAL_StatusTypeDef status;
    
    if (channel > 15) {
        return HAL_ERROR;
    }
    
    /* Disable all channels first (write 0x0000 to disable) */
    for (uint8_t i = 0; i < 16; i++) {
        AD4114_WriteRegister(dev, AD4114_REG_CH0 + i, 0x0000, 2);
    }
    
    /* Configure and enable only the requested channel
     * CH_EN = 1, SETUP = 0, AINPOS = channel, AINNEG = 17 (VINCOM)
     */
    uint32_t ch_config = AD4114_CH_EN | AD4114_CH_SETUP(0) | 
                         AD4114_CH_AINPOS(channel) | AD4114_CH_AINNEG(AD4114_AINCOM);
    
    status = AD4114_WriteRegister(dev, AD4114_REG_CH0, ch_config, 2);
    if (status != HAL_OK) {
        return status;
    }
    
    /* Set ADC to single conversion mode to trigger one conversion */
    uint32_t adcmode = (1 << 15) | (1 << 4);  /* REF_EN=1, MODE=1 (single conversion) */
    status = AD4114_WriteRegister(dev, AD4114_REG_ADCMODE, adcmode, 2);
    if (status != HAL_OK) {
        return status;
    }
    
    /* Wait for conversion to complete (~25ms for 50 SPS filter setting) */
    HAL_Delay(30);
    
    /* Read the conversion data */
    status = AD4114_ReadData(dev, data);
    
    /* Return to continuous conversion mode for next reading */
    adcmode = (1 << 15) | (0 << 4);  /* REF_EN=1, MODE=0 (continuous) */
    AD4114_WriteRegister(dev, AD4114_REG_ADCMODE, adcmode, 2);
    
    return status;
}

