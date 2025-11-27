/**
 ******************************************************************************
 * @file           : analog_input_handler.c
 * @brief          : Analog Input Handler Implementation for AD4114 ADCs
 ******************************************************************************
 * @attention
 *
 * Hardware Configuration:
 *   - AD4114_0 (SPI1): 4-20mA inputs + 0-10V V0/V2/V4
 *   - AD4114_1 (SPI4): 4-20mA inputs + 0-10V V1/V3/V5
 *
 * 0-10V Channel Mapping (from schematic HW_ENERSION_CONTROLLER_R1M1):
 *   V0 = AD4114_0, PIN3  = CH1
 *   V1 = AD4114_1, PIN28 = CH6
 *   V2 = AD4114_0, PIN4  = CH2
 *   V3 = AD4114_1, PIN27 = CH5
 *   V4 = AD4114_0, PIN5  = CH3
 *   V5 = AD4114_1, PIN26 = CH4
 *
 ******************************************************************************
 */

#include "analog_input_handler.h"
#include "ad4114.h"
#include "debug_uart.h"
#include <string.h>
#include <math.h>

/* ==========================================================================
 * HARDWARE CONFIGURATION
 * ========================================================================== */

/* External SPI Handles */
extern SPI_HandleTypeDef hspi1;  /* AD4114_0 */
extern SPI_HandleTypeDef hspi4;  /* AD4114_1 */

/* AD4114 Device Instances */
static AD4114_Device_t ad4114_dev1;  /* AD4114_0 on SPI1 */
static AD4114_Device_t ad4114_dev2;  /* AD4114_1 on SPI4 */

/* 0-10V Channel Mapping Table */
typedef struct {
    AD4114_Device_t *device;
    uint8_t channel;
    const char *name;
} VoltageChannelMap_t;

static VoltageChannelMap_t voltage_channel_map[NUM_VOLTAGE_CHANNELS];

/* ==========================================================================
 * PRIVATE VARIABLES
 * ========================================================================== */

static AnalogData_t analogData;
static float calibration_420_offset[NUM_420MA_CHANNELS] = {0};
static float calibration_420_gain[NUM_420MA_CHANNELS] = {1.0f};
static float calibration_voltage_offset[NUM_VOLTAGE_CHANNELS] = {0};
static float calibration_voltage_gain[NUM_VOLTAGE_CHANNELS] = {1.0f};

/* ==========================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ========================================================================== */

static float Convert_ADC_To_420mA(uint16_t adc_value);
static float Convert_ADC_To_Voltage(uint16_t adc_value);
static AnalogStatus_t Check_420mA_Status(float current_mA);
static AnalogStatus_t Check_Voltage_Status(float voltage_V);

/* ==========================================================================
 * PUBLIC FUNCTIONS
 * ========================================================================== */

/**
 * @brief  Initialize analog input handler and AD4114 ADCs
 */
void AnalogInput_Init(void)
{
    memset(&analogData, 0, sizeof(analogData));
    
    /* Initialize calibration to unity */
    for (uint8_t i = 0; i < NUM_420MA_CHANNELS; i++) {
        calibration_420_offset[i] = 0.0f;
        calibration_420_gain[i] = 1.0f;
    }
    for (uint8_t i = 0; i < NUM_VOLTAGE_CHANNELS; i++) {
        calibration_voltage_offset[i] = 0.0f;
        calibration_voltage_gain[i] = 1.0f;
    }
    
    /* Print configuration */
    DEBUG_INFO("[ADC] AD4114 Configuration:");
    DEBUG_INFO("[ADC]   Resolution: 24-bit Sigma-Delta");
    DEBUG_INFO("[ADC]   Reference: 2.5V Internal");
    DEBUG_INFO("[ADC]   Filter: SINC5+SINC1, 50 SPS");
    DEBUG_INFO("");
    
    /* ===== Initialize AD4114_0 (SPI1) ===== */
    DEBUG_INFO("[ADC] Initializing AD4114_0 on SPI1...");
    ad4114_dev1.hspi = &hspi1;
    ad4114_dev1.num_channels = 16;
    ad4114_dev1.current_channel = 0;
    ad4114_dev1.device_id = 1;
    
    if (AD4114_Init(&ad4114_dev1) != HAL_OK) {
        DEBUG_ERROR("[ADC] AD4114_0 (SPI1) INIT FAILED!");
    } else {
        DEBUG_INFO("[ADC] AD4114_0 OK");
    }
    
    /* ===== Initialize AD4114_1 (SPI4) ===== */
    DEBUG_INFO("[ADC] Initializing AD4114_1 on SPI4...");
    ad4114_dev2.hspi = &hspi4;
    ad4114_dev2.num_channels = 16;
    ad4114_dev2.current_channel = 0;
    ad4114_dev2.device_id = 2;
    
    if (AD4114_Init(&ad4114_dev2) != HAL_OK) {
        DEBUG_ERROR("[ADC] AD4114_1 (SPI4) INIT FAILED!");
    } else {
        DEBUG_INFO("[ADC] AD4114_1 OK");
    }
    
    /* ===== Setup 0-10V Channel Mapping ===== */
    /* 
     * AD4114 Pin to Channel mapping (from datasheet):
     * PIN3 = AIN0 = CH0, PIN4 = AIN1 = CH1, PIN5 = AIN2 = CH2
     * PIN26 = AIN13, PIN27 = AIN14, PIN28 = AIN15
     * 
     * For single-ended measurement: AINPOS = channel, AINNEG = AINCOM (17)
     */
    
    /* V0 = AD4114_0, PIN3 = AIN0 = CH0 */
    voltage_channel_map[0].device = &ad4114_dev1;
    voltage_channel_map[0].channel = 0;
    voltage_channel_map[0].name = "V0";
    
    /* V1 = AD4114_1, PIN28 = AIN15 = CH15 */
    voltage_channel_map[1].device = &ad4114_dev2;
    voltage_channel_map[1].channel = 15;
    voltage_channel_map[1].name = "V1";
    
    /* V2 = AD4114_0, PIN4 = AIN1 = CH1 */
    voltage_channel_map[2].device = &ad4114_dev1;
    voltage_channel_map[2].channel = 1;
    voltage_channel_map[2].name = "V2";
    
    /* V3 = AD4114_1, PIN27 = AIN14 = CH14 */
    voltage_channel_map[3].device = &ad4114_dev2;
    voltage_channel_map[3].channel = 14;
    voltage_channel_map[3].name = "V3";
    
    /* V4 = AD4114_0, PIN5 = AIN2 = CH2 */
    voltage_channel_map[4].device = &ad4114_dev1;
    voltage_channel_map[4].channel = 2;
    voltage_channel_map[4].name = "V4";
    
    /* V5 = AD4114_1, PIN26 = AIN13 = CH13 */
    voltage_channel_map[5].device = &ad4114_dev2;
    voltage_channel_map[5].channel = 13;
    voltage_channel_map[5].name = "V5";
    
    DEBUG_INFO("");
    DEBUG_INFO("[ADC] 0-10V Channel Mapping:");
    DEBUG_INFO("[ADC]   V0: AD4114_0 CH0  | V1: AD4114_1 CH15");
    DEBUG_INFO("[ADC]   V2: AD4114_0 CH1  | V3: AD4114_1 CH14");
    DEBUG_INFO("[ADC]   V4: AD4114_0 CH2  | V5: AD4114_1 CH13");
    DEBUG_INFO("");
    DEBUG_INFO("[ADC] Analog Input Handler Ready");
}

/**
 * @brief  Update analog inputs - TEST MODE: Scan all 16 channels of AD4114_1
 * @note   Prints all channel readings every 2 seconds for debugging
 */
void AnalogInput_Update(void)
{
    static uint32_t last_scan_time = 0;
    static uint8_t scan_in_progress = 0;
    static uint8_t current_scan_ch = 0;
    static int32_t scan_results[16] = {0};
    
    int32_t adc_data_24bit = 0;
    HAL_StatusTypeDef status;
    
    /* Start new scan every 2 seconds */
    if (!scan_in_progress && (HAL_GetTick() - last_scan_time >= 2000)) {
        scan_in_progress = 1;
        current_scan_ch = 0;
        DEBUG_INFO("");
        DEBUG_INFO("========== AD4114_1 CHANNEL SCAN ==========");
    }
    
    /* Scan one channel per call */
    if (scan_in_progress && current_scan_ch < 16) {
        /* Read channel from AD4114_1 (SPI4) */
        status = AD4114_ReadChannel(&ad4114_dev2, current_scan_ch, &adc_data_24bit);
        
        if (status == HAL_OK) {
            scan_results[current_scan_ch] = adc_data_24bit;
            
            /* Convert to voltage: (RAW / 2^24) * 2.5V * 4 (divider) */
            float voltage = ((float)adc_data_24bit / 16777216.0f) * 2.5f * 4.0f;
            
            DEBUG_INFO("[AD4114_1] CH%02d: RAW=0x%06lX (%7ld) = %6.3fV", 
                       current_scan_ch, 
                       (unsigned long)adc_data_24bit,
                       (long)adc_data_24bit,
                       voltage);
        } else {
            DEBUG_ERROR("[AD4114_1] CH%02d: READ FAILED!", current_scan_ch);
            scan_results[current_scan_ch] = -1;
        }
        
        current_scan_ch++;
        
        /* Scan complete */
        if (current_scan_ch >= 16) {
            DEBUG_INFO("============================================");
            DEBUG_INFO("");
            scan_in_progress = 0;
            last_scan_time = HAL_GetTick();
            analogData.update_count++;
        }
    }
    
    /* Also update voltage data for GUI (using first 6 channels as test) */
    for (uint8_t i = 0; i < NUM_VOLTAGE_CHANNELS && i < 16; i++) {
        if (scan_results[i] >= 0) {
            uint16_t raw16 = (uint16_t)(scan_results[i] >> 8);
            analogData.analog_voltage[i].raw_adc = raw16;
            analogData.analog_voltage[i].voltage_V = 
                ((float)scan_results[i] / 16777216.0f) * 2.5f * 4.0f;
        }
    }
    
    analogData.last_update_time = HAL_GetTick();
}

/**
 * @brief  Get raw ADC value for 4-20mA channel
 */
uint16_t AnalogInput_Get420mA_Raw(uint8_t channel)
{
    if (channel < NUM_420MA_CHANNELS) {
        return analogData.analog_420[channel].raw_adc;
    }
    return 0;
}

/**
 * @brief  Get 4-20mA current value in mA
 */
float AnalogInput_Get420mA_Current(uint8_t channel)
{
    if (channel < NUM_420MA_CHANNELS) {
        return analogData.analog_420[channel].current_mA;
    }
    return 0.0f;
}

/**
 * @brief  Get 4-20mA scaled percentage (4mA=0%, 20mA=100%)
 */
float AnalogInput_Get420mA_Percent(uint8_t channel)
{
    if (channel < NUM_420MA_CHANNELS) {
        return analogData.analog_420[channel].scaled_percent;
    }
    return 0.0f;
}

/**
 * @brief  Get 4-20mA channel status
 */
AnalogStatus_t AnalogInput_Get420mA_Status(uint8_t channel)
{
    if (channel < NUM_420MA_CHANNELS) {
        return analogData.analog_420[channel].status;
    }
    return ANALOG_STATUS_ERROR;
}

/**
 * @brief  Get all 4-20mA data packed for RS485 transmission
 * @param  buffer: Output buffer (needs NUM_420MA_CHANNELS * 6 bytes)
 * @param  bufferSize: Buffer size
 */
void AnalogInput_GetAll420mA(uint8_t* buffer, uint16_t bufferSize)
{
    if (bufferSize < (NUM_420MA_CHANNELS * 6)) {
        return;
    }
    
    uint16_t offset = 0;
    for (uint8_t i = 0; i < NUM_420MA_CHANNELS; i++) {
        /* Pack: raw_adc (2 bytes) + current_mA (4 bytes float) */
        memcpy(&buffer[offset], &analogData.analog_420[i].raw_adc, 2);
        offset += 2;
        memcpy(&buffer[offset], &analogData.analog_420[i].current_mA, 4);
        offset += 4;
    }
}

/**
 * @brief  Get raw ADC value for 0-10V channel
 */
uint16_t AnalogInput_GetVoltage_Raw(uint8_t channel)
{
    if (channel < NUM_VOLTAGE_CHANNELS) {
        return analogData.analog_voltage[channel].raw_adc;
    }
    return 0;
}

/**
 * @brief  Get voltage value in Volts
 */
float AnalogInput_GetVoltage_V(uint8_t channel)
{
    if (channel < NUM_VOLTAGE_CHANNELS) {
        return analogData.analog_voltage[channel].voltage_V;
    }
    return 0.0f;
}

/**
 * @brief  Get voltage scaled percentage (0V=0%, 10V=100%)
 */
float AnalogInput_GetVoltage_Percent(uint8_t channel)
{
    if (channel < NUM_VOLTAGE_CHANNELS) {
        return analogData.analog_voltage[channel].scaled_percent;
    }
    return 0.0f;
}

/**
 * @brief  Get voltage channel status
 */
AnalogStatus_t AnalogInput_GetVoltage_Status(uint8_t channel)
{
    if (channel < NUM_VOLTAGE_CHANNELS) {
        return analogData.analog_voltage[channel].status;
    }
    return ANALOG_STATUS_ERROR;
}

/**
 * @brief  Get all voltage data packed for RS485 transmission
 * @param  buffer: Output buffer (needs NUM_VOLTAGE_CHANNELS * 6 bytes)
 * @param  bufferSize: Buffer size
 */
void AnalogInput_GetAllVoltage(uint8_t* buffer, uint16_t bufferSize)
{
    if (bufferSize < (NUM_VOLTAGE_CHANNELS * 6)) {
        return;
    }
    
    uint16_t offset = 0;
    for (uint8_t i = 0; i < NUM_VOLTAGE_CHANNELS; i++) {
        /* Pack: raw_adc (2 bytes) + voltage_V (4 bytes float) */
        memcpy(&buffer[offset], &analogData.analog_voltage[i].raw_adc, 2);
        offset += 2;
        memcpy(&buffer[offset], &analogData.analog_voltage[i].voltage_V, 4);
        offset += 4;
    }
}

/**
 * @brief  Get all NTC data (NOT SUPPORTED - placeholder for compatibility)
 */
void AnalogInput_GetAllNTC(uint8_t* buffer, uint16_t bufferSize)
{
    (void)buffer;
    (void)bufferSize;
}

/**
 * @brief  Get all analog data (4-20mA + Voltage)
 */
void AnalogInput_GetAllData(uint8_t* buffer, uint16_t bufferSize)
{
    uint16_t required_size = (NUM_420MA_CHANNELS * 6) + (NUM_VOLTAGE_CHANNELS * 6);
    
    if (bufferSize < required_size) {
        return;
    }
    
    uint16_t offset = 0;
    
    /* Copy 4-20mA data */
    AnalogInput_GetAll420mA(&buffer[offset], bufferSize - offset);
    offset += (NUM_420MA_CHANNELS * 6);
    
    /* Copy voltage data */
    AnalogInput_GetAllVoltage(&buffer[offset], bufferSize - offset);
}

/**
 * @brief  Get pointer to analog data structure
 */
AnalogData_t* AnalogInput_GetDataStructure(void)
{
    return &analogData;
}

/**
 * @brief  Start ADC conversion (placeholder)
 */
void AnalogInput_StartConversion(void)
{
    DEBUG_INFO("[ADC] Conversion started");
}

/**
 * @brief  Set update rate (placeholder)
 */
void AnalogInput_SetUpdateRate(uint32_t rate_ms)
{
    (void)rate_ms;
}

/**
 * @brief  Calibrate 4-20mA channel
 */
void AnalogInput_Calibrate420mA(uint8_t channel, float offset, float gain)
{
    if (channel < NUM_420MA_CHANNELS) {
        calibration_420_offset[channel] = offset;
        calibration_420_gain[channel] = gain;
        DEBUG_INFO("[ADC] 4-20mA CH%d calibrated: offset=%.3f gain=%.3f", 
                   channel, offset, gain);
    }
}

/**
 * @brief  Calibrate voltage channel
 */
void AnalogInput_CalibrateVoltage(uint8_t channel, float offset, float gain)
{
    if (channel < NUM_VOLTAGE_CHANNELS) {
        calibration_voltage_offset[channel] = offset;
        calibration_voltage_gain[channel] = gain;
        DEBUG_INFO("[ADC] Voltage CH%d calibrated: offset=%.3f gain=%.3f", 
                   channel, offset, gain);
    }
}

/* ==========================================================================
 * PRIVATE FUNCTIONS
 * ========================================================================== */

/**
 * @brief  Convert ADC value to 4-20mA current
 * @param  adc_value: Raw ADC value (16-bit scaled from 24-bit)
 * @retval Current in mA
 * 
 * @note   Hardware: 4-20mA through sense resistor
 *         V = I × R, so I = V / R
 *         With 100Ω: 4mA → 0.4V, 20mA → 2.0V
 */
static float Convert_ADC_To_420mA(uint16_t adc_value)
{
    float voltage = ((float)adc_value / ADC_RESOLUTION_16BIT) * ADC_VREF;
    float current_mA = (voltage / CURRENT_SENSE_RESISTOR) * 1000.0f;
    return current_mA;
}

/**
 * @brief  Convert ADC value to voltage
 * @param  adc_value: Raw ADC value (16-bit scaled from 24-bit)
 * @retval Voltage in V
 * 
 * @note   Hardware: 0-10V input through voltage divider (4:1)
 *         10V input → 2.5V at ADC
 */
static float Convert_ADC_To_Voltage(uint16_t adc_value)
{
    float voltage_at_adc = ((float)adc_value / ADC_RESOLUTION_16BIT) * ADC_VREF;
    float voltage = voltage_at_adc * VOLTAGE_DIVIDER_RATIO;
    return voltage;
}

/**
 * @brief  Check 4-20mA channel status
 */
static AnalogStatus_t Check_420mA_Status(float current_mA)
{
    if (current_mA < CURRENT_UNDERRANGE_MA) {
        return ANALOG_STATUS_UNDERRANGE;  /* Wire break */
    }
    if (current_mA > CURRENT_OVERRANGE_MA) {
        return ANALOG_STATUS_OVERRANGE;
    }
    return ANALOG_STATUS_OK;
}

/**
 * @brief  Check voltage channel status
 */
static AnalogStatus_t Check_Voltage_Status(float voltage_V)
{
    if (voltage_V < VOLTAGE_MIN_V) {
        return ANALOG_STATUS_UNDERRANGE;
    }
    if (voltage_V > (VOLTAGE_MAX_V + 1.0f)) {
        return ANALOG_STATUS_OVERRANGE;
    }
    return ANALOG_STATUS_OK;
}
