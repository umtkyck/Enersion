/***************************************************************************//**
*   @file   ad411x_regs.h
*   @brief  AD4114 Registers Definitions - Configured for 16-channel single-ended
*           voltage measurement with internal 2.5V reference.
*   @author Based on Analog Devices no-OS driver
*           Modified for STM32 + AD4114 3-wire mode
********************************************************************************
* Configuration Summary:
* - ADCMODE: REF_EN=1 (internal 2.5V ref), MODE=0 (continuous conversion)
* - IFMODE: DATA_STAT=1 (append channel info to data)
* - SETUPCON0: BI_UNIPOLAR=1 (bipolar for ±10V), REF_SEL=2 (internal ref),
*              INBUF=11 (input buffers enabled - required for voltage inputs!)
* - FILTCON0: SINC5+SINC1 filter, 50 SPS
* - All channels use VINx vs VINCOM (single-ended)
******************************************************************************/

#ifndef AD4111_CFG_H_
#define AD4111_CFG_H_

#include "ad717x.h"

/* 
 * AD4114 Register Configuration for 16-channel single-ended voltage measurement
 * 
 * Key Settings:
 * - Internal 2.5V reference enabled
 * - Bipolar mode for ±10V input range (with external 4:1 divider)
 * - Input buffers enabled (CRITICAL for voltage measurements!)
 * - DATA_STAT enabled to identify which channel data belongs to
 * - All 16 channels configured for VINx vs VINCOM
 */
ad717x_st_reg ad4111_regs[] = {
    /* Status Register (0x00) - Read only, 1 byte */
    { AD717X_STATUS_REG, 0x00, 1 },
    
    /* ADC Mode Register (0x01) - 2 bytes
     * Bit 15: REF_EN = 1 (Enable internal 2.5V reference)
     * Bit 14: HIDE_DELAY = 0
     * Bit 13: SING_CYC = 0
     * Bit 12:8: DELAY = 0
     * Bit 6:4: MODE = 000 (Continuous conversion mode)
     * Bit 3:2: CLOCKSEL = 00 (Internal oscillator)
     * 
     * Value: 0x8000 = REF_EN | MODE(0) = Internal ref + Continuous mode
     */
    {
        AD717X_ADCMODE_REG,
        AD717X_ADCMODE_REG_REF_EN | AD717X_ADCMODE_REG_MODE(0),
        2
    },
    
    /* Interface Mode Register (0x02) - 2 bytes
     * Bit 12: ALT_SYNC = 0
     * Bit 11: IOSTRENGTH = 0
     * Bit 8: DOUT_RESET = 0
     * Bit 7: CONTREAD = 0 (We'll use single reads)
     * Bit 6: DATA_STAT = 1 (Append status byte to data - IMPORTANT!)
     * Bit 5: REG_CHECK = 0
     * Bit 2:1: CRC/XOR = 00 (Disabled)
     * Bit 0: WL16 = 0 (24-bit data)
     * 
     * Value: 0x0040 = DATA_STAT enabled
     */
    { AD717X_IFMODE_REG, AD717X_IFMODE_REG_DATA_STAT, 2 },
    
    /* Register Check (0x03) - 3 bytes */
    { AD717X_REGCHECK_REG, 0x0000, 3 },
    
    /* Data Register (0x04) - 3 bytes (or 4 with DATA_STAT) */
    { AD717X_DATA_REG, 0x0000, 3 },
    
    /* GPIO Configuration Register (0x06) - 2 bytes
     * SYNC_EN = 1 (Enable SYNC functionality)
     */
    {
        AD717X_GPIOCON_REG,
        AD717X_GPIOCON_REG_SYNC_EN,
        2
    },
    
    /* ID Register (0x07) - 2 bytes, Read only
     * Expected value for AD4114: 0x31Dx
     */
    { AD717X_ID_REG, 0x0000, 2 },
    
    /* Channel Registers (0x10-0x1F) - 2 bytes each
     * All channels disabled initially (enabled by driver)
     * Will be configured as VINx vs VINCOM
     */
    { AD717X_CHMAP0_REG, 0x0000, 2 },
    { AD717X_CHMAP1_REG, 0x0000, 2 },
    { AD717X_CHMAP2_REG, 0x0000, 2 },
    { AD717X_CHMAP3_REG, 0x0000, 2 },
    { AD717X_CHMAP4_REG, 0x0000, 2 },
    { AD717X_CHMAP5_REG, 0x0000, 2 },
    { AD717X_CHMAP6_REG, 0x0000, 2 },
    { AD717X_CHMAP7_REG, 0x0000, 2 },
    { AD717X_CHMAP8_REG, 0x0000, 2 },
    { AD717X_CHMAP9_REG, 0x0000, 2 },
    { AD717X_CHMAP10_REG, 0x0000, 2 },
    { AD717X_CHMAP11_REG, 0x0000, 2 },
    { AD717X_CHMAP12_REG, 0x0000, 2 },
    { AD717X_CHMAP13_REG, 0x0000, 2 },
    { AD717X_CHMAP14_REG, 0x0000, 2 },
    { AD717X_CHMAP15_REG, 0x0000, 2 },
    
    /* Setup Configuration Registers (0x20-0x27) - 2 bytes each
     * 
     * For AD4114 voltage measurement (±10V with 4:1 divider):
     * Bit 12: BI_UNIPOLAR = 1 (Bipolar mode - REQUIRED for voltage!)
     * Bit 11: REFPOS_BUF = 0 (Internal ref doesn't need buffer)
     * Bit 10: REFNEG_BUF = 0
     * Bit 9:8: AIN_BUF = 11 (Input buffers ENABLED - CRITICAL!)
     * Bit 6: BUFCHOPMAX = 0
     * Bit 5:4: REF_SEL = 10 (Internal 2.5V reference)
     * 
     * Value: 0x1320 = BI_UNIPOLAR(1) | AIN_BUF(11) | REF_SEL(2)
     *      = (1<<12) | (3<<8) | (2<<4)
     *      = 0x1000 | 0x0300 | 0x0020 = 0x1320
     */
    { AD717X_SETUPCON0_REG, 
      AD717X_SETUP_CONF_REG_BI_UNIPOLAR |  /* Bipolar mode */
      AD4111_SETUP_CONF_REG_AIN_BUF(3) |   /* Input buffers enabled */
      AD717X_SETUP_CONF_REG_REF_SEL(2),    /* Internal 2.5V reference */
      2 },
    { AD717X_SETUPCON1_REG, 
      AD717X_SETUP_CONF_REG_BI_UNIPOLAR | AD4111_SETUP_CONF_REG_AIN_BUF(3) | AD717X_SETUP_CONF_REG_REF_SEL(2), 2 },
    { AD717X_SETUPCON2_REG, 
      AD717X_SETUP_CONF_REG_BI_UNIPOLAR | AD4111_SETUP_CONF_REG_AIN_BUF(3) | AD717X_SETUP_CONF_REG_REF_SEL(2), 2 },
    { AD717X_SETUPCON3_REG, 
      AD717X_SETUP_CONF_REG_BI_UNIPOLAR | AD4111_SETUP_CONF_REG_AIN_BUF(3) | AD717X_SETUP_CONF_REG_REF_SEL(2), 2 },
    { AD717X_SETUPCON4_REG, 
      AD717X_SETUP_CONF_REG_BI_UNIPOLAR | AD4111_SETUP_CONF_REG_AIN_BUF(3) | AD717X_SETUP_CONF_REG_REF_SEL(2), 2 },
    { AD717X_SETUPCON5_REG, 
      AD717X_SETUP_CONF_REG_BI_UNIPOLAR | AD4111_SETUP_CONF_REG_AIN_BUF(3) | AD717X_SETUP_CONF_REG_REF_SEL(2), 2 },
    { AD717X_SETUPCON6_REG, 
      AD717X_SETUP_CONF_REG_BI_UNIPOLAR | AD4111_SETUP_CONF_REG_AIN_BUF(3) | AD717X_SETUP_CONF_REG_REF_SEL(2), 2 },
    { AD717X_SETUPCON7_REG, 
      AD717X_SETUP_CONF_REG_BI_UNIPOLAR | AD4111_SETUP_CONF_REG_AIN_BUF(3) | AD717X_SETUP_CONF_REG_REF_SEL(2), 2 },
    
    /* Filter Configuration Registers (0x28-0x2F) - 2 bytes each
     * 
     * Bit 15: SINC3_MAP = 0
     * Bit 11: ENHFILTEN = 0 (No enhanced filter)
     * Bit 10:8: ENHFILT = 000
     * Bit 6:5: ORDER = 00 (SINC5 + SINC1 - recommended for multiplexed)
     * Bit 4:0: ODR = 10100 (20 = ~50 SPS)
     * 
     * Value: 0x0014 = ODR(20) for ~50 SPS
     */
    { AD717X_FILTCON0_REG, AD717X_FILT_CONF_REG_ODR(20), 2 },
    { AD717X_FILTCON1_REG, AD717X_FILT_CONF_REG_ODR(20), 2 },
    { AD717X_FILTCON2_REG, AD717X_FILT_CONF_REG_ODR(20), 2 },
    { AD717X_FILTCON3_REG, AD717X_FILT_CONF_REG_ODR(20), 2 },
    { AD717X_FILTCON4_REG, AD717X_FILT_CONF_REG_ODR(20), 2 },
    { AD717X_FILTCON5_REG, AD717X_FILT_CONF_REG_ODR(20), 2 },
    { AD717X_FILTCON6_REG, AD717X_FILT_CONF_REG_ODR(20), 2 },
    { AD717X_FILTCON7_REG, AD717X_FILT_CONF_REG_ODR(20), 2 },
    
    /* Offset Registers (0x30-0x37) - 3 bytes each */
    { AD717X_OFFSET0_REG, 0x800000, 3 },  /* Default mid-scale for bipolar */
    { AD717X_OFFSET1_REG, 0x800000, 3 },
    { AD717X_OFFSET2_REG, 0x800000, 3 },
    { AD717X_OFFSET3_REG, 0x800000, 3 },
    { AD717X_OFFSET4_REG, 0x800000, 3 },
    { AD717X_OFFSET5_REG, 0x800000, 3 },
    { AD717X_OFFSET6_REG, 0x800000, 3 },
    { AD717X_OFFSET7_REG, 0x800000, 3 },
    
    /* Gain Registers (0x38-0x3F) - 3 bytes each
     * Default gain = 0x555555 (datasheet default)
     */
    { AD717X_GAIN0_REG, 0x555555, 3 },
    { AD717X_GAIN1_REG, 0x555555, 3 },
    { AD717X_GAIN2_REG, 0x555555, 3 },
    { AD717X_GAIN3_REG, 0x555555, 3 },
    { AD717X_GAIN4_REG, 0x555555, 3 },
    { AD717X_GAIN5_REG, 0x555555, 3 },
    { AD717X_GAIN6_REG, 0x555555, 3 },
    { AD717X_GAIN7_REG, 0x555555, 3 },
};

#endif /* AD4111_CFG_H_ */
