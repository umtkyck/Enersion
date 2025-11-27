/***************************************************************************//**
 *   @file   no_os_spi.c
 *   @brief  Platform-independent SPI interface - STM32 HAL implementation
 *           Optimized for AD4114 in 3-wire mode (CS tied to GND)
 *******************************************************************************/

#include "no_os_spi.h"
#include "no_os_alloc.h"
#include "no_os_error.h"
#include <string.h>

/* Debug output - uncomment to enable */
// #define SPI_DEBUG

#ifdef SPI_DEBUG
#include <stdio.h>
extern UART_HandleTypeDef huart1;
static char spi_dbg[128];
#define SPI_DBG(...) do { \
    int len = snprintf(spi_dbg, sizeof(spi_dbg), __VA_ARGS__); \
    HAL_UART_Transmit(&huart1, (uint8_t*)spi_dbg, len, 100); \
} while(0)
#else
#define SPI_DBG(...)
#endif

/**
 * @brief Initialize SPI
 * 
 * For AD4114 in 3-wire mode:
 * - CS is tied to GND (always active)
 * - Must send 64 SCLK cycles with DIN=HIGH to reset
 * - Wait 500us after reset for LDO to stabilize
 */
int32_t no_os_spi_init(struct no_os_spi_desc **desc,
                       const struct no_os_spi_init_param *param)
{
    struct no_os_spi_desc *spi_desc;
    uint8_t reset_buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (!desc || !param || !param->hspi)
        return -EINVAL;

    spi_desc = (struct no_os_spi_desc *)no_os_malloc(sizeof(*spi_desc));
    if (!spi_desc)
        return -ENOMEM;

    spi_desc->hspi = param->hspi;
    spi_desc->chip_select_port = param->chip_select_port;
    spi_desc->chip_select_pin = param->chip_select_pin;

    /* AD4114 3-wire mode reset sequence:
     * Send 64 SCLK cycles with DIN=HIGH (0xFF bytes)
     * This resets the serial interface to a known state
     */
    SPI_DBG("[SPI] Sending reset sequence (64 clocks with DIN=1)...\r\n");
    
    /* Send 8 bytes x 8 bits = 64 clock cycles with MOSI HIGH */
    HAL_SPI_Transmit(spi_desc->hspi, reset_buf, 8, 100);
    
    /* Wait 500us for LDO to power up (per datasheet) */
    HAL_Delay(1);
    
    /* Send another reset sequence to ensure sync */
    HAL_SPI_Transmit(spi_desc->hspi, reset_buf, 8, 100);
    
    /* Wait for device to stabilize */
    HAL_Delay(10);
    
    SPI_DBG("[SPI] Reset complete\r\n");

    *desc = spi_desc;

    return 0;
}

/**
 * @brief Free SPI resources
 */
int32_t no_os_spi_remove(struct no_os_spi_desc *desc)
{
    if (!desc)
        return -EINVAL;

    no_os_free(desc);

    return 0;
}

/**
 * @brief Write and read data via SPI
 * 
 * For AD4114 in 3-wire mode (CS tied to GND):
 * - No CS toggling needed
 * - Full-duplex transfer
 * - Data is clocked on SCLK rising edge (Mode 3)
 */
int32_t no_os_spi_write_and_read(struct no_os_spi_desc *desc,
                                 uint8_t *data,
                                 uint16_t bytes_number)
{
    HAL_StatusTypeDef ret;
    uint8_t tx_buf[16];
    uint8_t rx_buf[16];

    if (!desc || !data || bytes_number > 16)
        return -EINVAL;

    /* Copy TX data */
    memcpy(tx_buf, data, bytes_number);

    SPI_DBG("[SPI] TX(%d): ", bytes_number);
    #ifdef SPI_DEBUG
    for (int i = 0; i < bytes_number; i++) SPI_DBG("%02X ", tx_buf[i]);
    SPI_DBG("\r\n");
    #endif

    /* Perform SPI transfer */
    ret = HAL_SPI_TransmitReceive(desc->hspi, tx_buf, rx_buf, bytes_number, 1000);

    if (ret != HAL_OK) {
        SPI_DBG("[SPI] ERROR: HAL_SPI_TransmitReceive failed (%d)\r\n", ret);
        return -EIO;
    }

    SPI_DBG("[SPI] RX(%d): ", bytes_number);
    #ifdef SPI_DEBUG
    for (int i = 0; i < bytes_number; i++) SPI_DBG("%02X ", rx_buf[i]);
    SPI_DBG("\r\n");
    #endif

    /* Copy RX data back */
    memcpy(data, rx_buf, bytes_number);

    return 0;
}

