/**
 ******************************************************************************
 * @file           : analog_input_handler.h
 * @brief          : Analog Input Handler Layer
 ******************************************************************************
 * @attention
 *
 * Application Layer for reading analog inputs via AD4114BCPZ:
 * - 26x 4-20mA current loop inputs
 * - 6x 0-10V voltage inputs
 *
 * Hardware: 2x AD4114BCPZ (24-bit Sigma-Delta ADC)
 * - AD4114 #1 on SPI1: 16 channels (4-20mA CH0-CH15)
 * - AD4114 #2 on SPI4: 16 channels (4-20mA CH16-CH25 + 0-10V CH0-CH5)
 *
 ******************************************************************************
 */

#ifndef ANALOG_INPUT_HANDLER_H
#define ANALOG_INPUT_HANDLER_H

#include "main.h"

/* Analog Channel Counts */
#define NUM_420MA_CHANNELS      26
#define NUM_VOLTAGE_CHANNELS    6
#define TOTAL_ANALOG_CHANNELS   (NUM_420MA_CHANNELS + NUM_VOLTAGE_CHANNELS)

/* ADC Configuration (AD4114 - 24-bit Sigma-Delta) */
#define ADC_RESOLUTION_24BIT    16777216.0f // 24-bit (2^24)
#define ADC_RESOLUTION_16BIT    65536.0f    // 16-bit (2^16) for scaled values
#define ADC_VREF                2.5f        // AD4114 Internal Reference (V)

/* 4-20mA Configuration */
#define CURRENT_MIN_MA          4.0f
#define CURRENT_MAX_MA          20.0f
#define CURRENT_UNDERRANGE_MA   3.8f        // Wire break detection
#define CURRENT_OVERRANGE_MA    21.0f
#define CURRENT_SENSE_RESISTOR  100.0f      // Ohms (check schematic!)

/* 0-10V Configuration */
#define VOLTAGE_MIN_V           0.0f
#define VOLTAGE_MAX_V           10.0f
#define VOLTAGE_DIVIDER_RATIO   4.0f        // 10V → 2.5V (4:1 divider)

/* Status Codes */
typedef enum {
    ANALOG_STATUS_OK = 0,
    ANALOG_STATUS_UNDERRANGE = 1,
    ANALOG_STATUS_OVERRANGE = 2,
    ANALOG_STATUS_OPEN_CIRCUIT = 3,
    ANALOG_STATUS_SHORT_CIRCUIT = 4,
    ANALOG_STATUS_ERROR = 5
} AnalogStatus_t;

/* 4-20mA Data Structure */
typedef struct {
    uint16_t raw_adc;
    float current_mA;
    float scaled_percent;
    AnalogStatus_t status;
} Analog420_Channel_t;

/* 0-10V Data Structure */
typedef struct {
    uint16_t raw_adc;
    float voltage_V;
    float scaled_percent;
    AnalogStatus_t status;
} AnalogVoltage_Channel_t;

/* Complete Analog Data Structure */
typedef struct {
    Analog420_Channel_t analog_420[NUM_420MA_CHANNELS];
    AnalogVoltage_Channel_t analog_voltage[NUM_VOLTAGE_CHANNELS];
    uint32_t last_update_time;
    uint32_t update_count;
} AnalogData_t;

/* Function Prototypes */
void AnalogInput_Init(void);
void AnalogInput_Update(void);
void AnalogInput_StartConversion(void);

/* 4-20mA Functions */
uint16_t AnalogInput_Get420mA_Raw(uint8_t channel);
float AnalogInput_Get420mA_Current(uint8_t channel);
float AnalogInput_Get420mA_Percent(uint8_t channel);
AnalogStatus_t AnalogInput_Get420mA_Status(uint8_t channel);
void AnalogInput_GetAll420mA(uint8_t* buffer, uint16_t bufferSize);

/* 0-10V Functions */
uint16_t AnalogInput_GetVoltage_Raw(uint8_t channel);
float AnalogInput_GetVoltage_V(uint8_t channel);
float AnalogInput_GetVoltage_Percent(uint8_t channel);
AnalogStatus_t AnalogInput_GetVoltage_Status(uint8_t channel);
void AnalogInput_GetAllVoltage(uint8_t* buffer, uint16_t bufferSize);

/* Bulk Read */
void AnalogInput_GetAllData(uint8_t* buffer, uint16_t bufferSize);
AnalogData_t* AnalogInput_GetDataStructure(void);

/* Configuration */
void AnalogInput_SetUpdateRate(uint32_t rate_ms);
void AnalogInput_Calibrate420mA(uint8_t channel, float offset, float gain);
void AnalogInput_CalibrateVoltage(uint8_t channel, float offset, float gain);

#endif /* ANALOG_INPUT_HANDLER_H */


