/**
 ******************************************************************************
 * @file           : ad4114.h
 * @brief          : AD4114BCPZ 24-bit Sigma-Delta ADC Driver
 ******************************************************************************
 * @attention
 *
 * Driver for Analog Devices AD4114BCPZ
 * - 16-channel 24-bit ADC
 * - SPI interface
 * - Multiple AD4114 chips on SPI1 and SPI4
 *
 ******************************************************************************
 */

#ifndef AD4114_H
#define AD4114_H

#include "main.h"

/* AD4114 Register Addresses */
#define AD4114_REG_COMMS         0x00
#define AD4114_REG_STATUS        0x00
#define AD4114_REG_ADCMODE       0x01
#define AD4114_REG_IFMODE        0x02
#define AD4114_REG_REGCHECK      0x03
#define AD4114_REG_DATA          0x04
#define AD4114_REG_GPIOCON       0x06
#define AD4114_REG_ID            0x07

#define AD4114_REG_CH0           0x10
#define AD4114_REG_CH1           0x11
#define AD4114_REG_CH2           0x12
#define AD4114_REG_CH3           0x13
#define AD4114_REG_CH4           0x14
#define AD4114_REG_CH5           0x15
#define AD4114_REG_CH6           0x16
#define AD4114_REG_CH7           0x17
#define AD4114_REG_CH8           0x18
#define AD4114_REG_CH9           0x19
#define AD4114_REG_CH10          0x1A
#define AD4114_REG_CH11          0x1B
#define AD4114_REG_CH12          0x1C
#define AD4114_REG_CH13          0x1D
#define AD4114_REG_CH14          0x1E
#define AD4114_REG_CH15          0x1F

#define AD4114_REG_SETUPCON0     0x20
#define AD4114_REG_SETUPCON1     0x21
#define AD4114_REG_SETUPCON2     0x22
#define AD4114_REG_SETUPCON3     0x23
#define AD4114_REG_SETUPCON4     0x24
#define AD4114_REG_SETUPCON5     0x25
#define AD4114_REG_SETUPCON6     0x26
#define AD4114_REG_SETUPCON7     0x27

#define AD4114_REG_FILTCON0      0x28
#define AD4114_REG_FILTCON1      0x29
#define AD4114_REG_FILTCON2      0x2A
#define AD4114_REG_FILTCON3      0x2B
#define AD4114_REG_FILTCON4      0x2C
#define AD4114_REG_FILTCON5      0x2D
#define AD4114_REG_FILTCON6      0x2E
#define AD4114_REG_FILTCON7      0x2F

#define AD4114_REG_OFFSET0       0x30
#define AD4114_REG_OFFSET1       0x31
#define AD4114_REG_OFFSET2       0x32
#define AD4114_REG_OFFSET3       0x33
#define AD4114_REG_OFFSET4       0x34
#define AD4114_REG_OFFSET5       0x35
#define AD4114_REG_OFFSET6       0x36
#define AD4114_REG_OFFSET7       0x37

#define AD4114_REG_GAIN0         0x38
#define AD4114_REG_GAIN1         0x39
#define AD4114_REG_GAIN2         0x3A
#define AD4114_REG_GAIN3         0x3B
#define AD4114_REG_GAIN4         0x3C
#define AD4114_REG_GAIN5         0x3D
#define AD4114_REG_GAIN6         0x3E
#define AD4114_REG_GAIN7         0x3F

/* Communication Register Bits 
 * Bit 7: WEN (Write Enable) - must be 0 for valid command
 * Bit 6: R/W - 0=Write, 1=Read
 * Bit 5-0: Register Address
 */
#define AD4114_COMM_REG_WEN      (0 << 7)  /* Write enable (must be 0) */
#define AD4114_COMM_REG_RD       (1 << 6)  /* Read operation */
#define AD4114_COMM_REG_WR       (0 << 6)  /* Write operation */

/* ADC Mode Register (0x01) Bits - 16-bit register */
#define AD4114_ADCMODE_REF_EN       (1 << 15) /* Internal 2.5V reference enable */
#define AD4114_ADCMODE_HIDE_DELAY   (1 << 14) /* Hide delay */
#define AD4114_ADCMODE_SING_CYC     (1 << 13) /* Single cycle mode */
#define AD4114_ADCMODE_MODE_CONT    (0 << 4)  /* Continuous conversion */
#define AD4114_ADCMODE_MODE_SINGLE  (1 << 4)  /* Single conversion */
#define AD4114_ADCMODE_MODE_STANDBY (2 << 4)  /* Standby mode */
#define AD4114_ADCMODE_MODE_PWRDN   (3 << 4)  /* Power-down mode */
#define AD4114_ADCMODE_MODE_IDLE    (4 << 4)  /* Idle mode */
#define AD4114_ADCMODE_MODE_INTOFF  (5 << 4)  /* Internal offset calibration */
#define AD4114_ADCMODE_MODE_INTGAIN (6 << 4)  /* Internal gain calibration */
#define AD4114_ADCMODE_MODE_SYSOFF  (7 << 4)  /* System offset calibration */
#define AD4114_ADCMODE_MODE_SYSGAIN (8 << 4)  /* System gain calibration */

/* Interface Mode Register (0x02) Bits - 16-bit register */
#define AD4114_IFMODE_DATA_STAT     (1 << 6)  /* Append status to data */
#define AD4114_IFMODE_REG_CHECK     (1 << 4)  /* Enable CRC on register */
#define AD4114_IFMODE_DOUT_RESET    (1 << 2)  /* DOUT reset */
#define AD4114_IFMODE_CONTREAD      (1 << 1)  /* Continuous read mode */
#define AD4114_IFMODE_WL16          (1 << 0)  /* 16-bit data output */

/* Setup Configuration Register (0x20-0x27) Bits - 16-bit register */
#define AD4114_SETUP_BIPOLAR        (1 << 12) /* Bipolar mode */
#define AD4114_SETUP_REFSEL_EXT     (0 << 4)  /* External reference REFIN */
#define AD4114_SETUP_REFSEL_EXT2    (1 << 4)  /* External reference REFIN2 */
#define AD4114_SETUP_REFSEL_INT     (2 << 4)  /* Internal 2.5V reference */
#define AD4114_SETUP_REFSEL_AVDD    (3 << 4)  /* AVDD reference */
#define AD4114_SETUP_AIN_BUFP       (1 << 3)  /* Input buffer+ enable */
#define AD4114_SETUP_AIN_BUFM       (1 << 2)  /* Input buffer- enable */
#define AD4114_SETUP_REF_BUFP       (1 << 1)  /* Ref buffer+ enable */
#define AD4114_SETUP_REF_BUFM       (1 << 0)  /* Ref buffer- enable */

/* Channel Register (0x10-0x1F) Bits - 16-bit register */
#define AD4114_CH_EN                (1 << 15) /* Channel enable */
#define AD4114_CH_SETUP(x)          (((x) & 0x07) << 12) /* Setup selection (0-7) */
#define AD4114_CH_AINPOS(x)         (((x) & 0x1F) << 5)  /* Positive input (0-16) */
#define AD4114_CH_AINNEG(x)         ((x) & 0x1F)         /* Negative input (0-17) */
#define AD4114_AINCOM               17        /* Common input for single-ended */

/* Filter Configuration Register (0x28-0x2F) Bits - 16-bit register */
#define AD4114_FILT_SINC3_MAP       (1 << 15) /* SINC3 filter map */
#define AD4114_FILT_ENHFILTEN       (1 << 12) /* Enhanced filter enable */
#define AD4114_FILT_ORDER_SINC5     (0 << 8)  /* SINC5 + SINC1 */
#define AD4114_FILT_ORDER_SINC3     (3 << 8)  /* SINC3 */
#define AD4114_FILT_ODR(x)          ((x) & 0x1F) /* Output data rate selection */

/* AD4114 Device Instance */
typedef struct {
    SPI_HandleTypeDef *hspi;    /* SPI handle */
    uint8_t num_channels;       /* Number of active channels */
    uint8_t current_channel;    /* Current channel being read */
    uint8_t device_id;          /* Device ID for debugging */
} AD4114_Device_t;

/* Function Prototypes */
HAL_StatusTypeDef AD4114_Init(AD4114_Device_t *dev);
HAL_StatusTypeDef AD4114_Reset(AD4114_Device_t *dev);
HAL_StatusTypeDef AD4114_ReadID(AD4114_Device_t *dev, uint16_t *id);

HAL_StatusTypeDef AD4114_WriteRegister(AD4114_Device_t *dev, uint8_t reg, uint32_t value, uint8_t bytes);
HAL_StatusTypeDef AD4114_ReadRegister(AD4114_Device_t *dev, uint8_t reg, uint32_t *value, uint8_t bytes);

HAL_StatusTypeDef AD4114_ConfigureChannel(AD4114_Device_t *dev, uint8_t channel, 
                                          uint8_t ainp, uint8_t ainm, uint8_t setup);
HAL_StatusTypeDef AD4114_EnableChannel(AD4114_Device_t *dev, uint8_t channel, uint8_t enable);

HAL_StatusTypeDef AD4114_StartConversion(AD4114_Device_t *dev);
HAL_StatusTypeDef AD4114_ReadData(AD4114_Device_t *dev, int32_t *data);
HAL_StatusTypeDef AD4114_ReadChannel(AD4114_Device_t *dev, uint8_t channel, int32_t *data);

/* Helper Functions */
void AD4114_CS_Low(AD4114_Device_t *dev);
void AD4114_CS_High(AD4114_Device_t *dev);

#endif /* AD4114_H */

