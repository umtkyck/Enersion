/**
 ******************************************************************************
 * @file           : analog_input_handler.c
 * @brief          : Analog Input Handler Implementation
 ******************************************************************************
 */

#include "analog_input_handler.h"
#include "debug_uart.h"
#include <string.h>
#include <math.h>

/* External ADC Handle - Note: May need multiple ADCs for 36 channels */
extern ADC_HandleTypeDef hadc1;

/* Private Variables */
static AnalogData_t analogData;
static uint32_t update_rate_ms = 100;  // Default 100ms update rate
static float calibration_420_offset[NUM_420MA_CHANNELS] = {0};
static float calibration_420_gain[NUM_420MA_CHANNELS] = {1.0f};
static float calibration_voltage_offset[NUM_VOLTAGE_CHANNELS] = {0};
static float calibration_voltage_gain[NUM_VOLTAGE_CHANNELS] = {1.0f};

/* Private Function Prototypes */
static float Convert_ADC_To_420mA(uint16_t adc_value);
static float Convert_ADC_To_Voltage(uint16_t adc_value);
static float Convert_ADC_To_NTC_Temperature(uint16_t adc_value);
static float Calculate_NTC_Resistance(uint16_t adc_value);
static AnalogStatus_t Check_420mA_Status(float current_mA);
static AnalogStatus_t Check_Voltage_Status(float voltage_V);
static AnalogStatus_t Check_NTC_Status(uint16_t adc_value);

/**
 * @brief  Initialize analog input handler
 * @retval None
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
    
    /* Calibrate ADC */
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    
    DEBUG_INFO("Analog Input Handler initialized");
    DEBUG_INFO("  - 26x 4-20mA channels");
    DEBUG_INFO("  - 6x 0-10V channels");
    DEBUG_INFO("  - 4x NTC channels");
}

/**
 * @brief  Update all analog inputs
 * @retval None
 */
void AnalogInput_Update(void)
{
    /* Note: This is a simplified implementation
     * In production, you would:
     * 1. Use DMA for ADC conversion
     * 2. Multiplex between channels
     * 3. Apply averaging/filtering
     * 4. Handle multiple ADC peripherals
     */
    
    static uint8_t current_channel = 0;
    uint16_t adc_value = 0;
    
    /* Start ADC conversion */
    HAL_ADC_Start(&hadc1);
    
    /* Wait for conversion (blocking - use DMA in production) */
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        adc_value = HAL_ADC_GetValue(&hadc1);
        
        /* Route to appropriate channel based on sequencer */
        if (current_channel < NUM_420MA_CHANNELS) {
            /* 4-20mA Channel */
            analogData.analog_420[current_channel].raw_adc = adc_value;
            analogData.analog_420[current_channel].current_mA = 
                Convert_ADC_To_420mA(adc_value);
            
            /* Apply calibration */
            analogData.analog_420[current_channel].current_mA = 
                (analogData.analog_420[current_channel].current_mA + 
                 calibration_420_offset[current_channel]) * 
                calibration_420_gain[current_channel];
            
            /* Scale to percentage */
            analogData.analog_420[current_channel].scaled_percent = 
                ((analogData.analog_420[current_channel].current_mA - CURRENT_MIN_MA) / 
                 (CURRENT_MAX_MA - CURRENT_MIN_MA)) * 100.0f;
            
            /* Check status */
            analogData.analog_420[current_channel].status = 
                Check_420mA_Status(analogData.analog_420[current_channel].current_mA);
        }
        else if (current_channel < (NUM_420MA_CHANNELS + NUM_VOLTAGE_CHANNELS)) {
            /* 0-10V Channel */
            uint8_t v_ch = current_channel - NUM_420MA_CHANNELS;
            analogData.analog_voltage[v_ch].raw_adc = adc_value;
            analogData.analog_voltage[v_ch].voltage_V = 
                Convert_ADC_To_Voltage(adc_value);
            
            /* Apply calibration */
            analogData.analog_voltage[v_ch].voltage_V = 
                (analogData.analog_voltage[v_ch].voltage_V + 
                 calibration_voltage_offset[v_ch]) * 
                calibration_voltage_gain[v_ch];
            
            /* Scale to percentage */
            analogData.analog_voltage[v_ch].scaled_percent = 
                (analogData.analog_voltage[v_ch].voltage_V / VOLTAGE_MAX_V) * 100.0f;
            
            /* Check status */
            analogData.analog_voltage[v_ch].status = 
                Check_Voltage_Status(analogData.analog_voltage[v_ch].voltage_V);
        }
        else {
            /* NTC Channel */
            uint8_t ntc_ch = current_channel - NUM_420MA_CHANNELS - NUM_VOLTAGE_CHANNELS;
            analogData.ntc[ntc_ch].raw_adc = adc_value;
            analogData.ntc[ntc_ch].resistance_ohm = 
                Calculate_NTC_Resistance(adc_value);
            analogData.ntc[ntc_ch].temperature_C = 
                Convert_ADC_To_NTC_Temperature(adc_value);
            
            /* Check status */
            analogData.ntc[ntc_ch].status = Check_NTC_Status(adc_value);
        }
    }
    
    HAL_ADC_Stop(&hadc1);
    
    /* Move to next channel */
    current_channel++;
    if (current_channel >= TOTAL_ANALOG_CHANNELS) {
        current_channel = 0;
        analogData.last_update_time = HAL_GetTick();
        analogData.update_count++;
    }
}

/**
 * @brief  Start ADC conversion
 * @retval None
 */
void AnalogInput_StartConversion(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&analogData, TOTAL_ANALOG_CHANNELS);
}

/**
 * @brief  Get 4-20mA current value
 * @param  channel: Channel number (0-25)
 * @retval Current in mA
 */
float AnalogInput_Get420mA_Current(uint8_t channel)
{
    if (channel < NUM_420MA_CHANNELS) {
        return analogData.analog_420[channel].current_mA;
    }
    return 0.0f;
}

/**
 * @brief  Get 4-20mA scaled percentage
 * @param  channel: Channel number (0-25)
 * @retval Percentage (0-100%)
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
 * @param  channel: Channel number (0-25)
 * @retval Status code
 */
AnalogStatus_t AnalogInput_Get420mA_Status(uint8_t channel)
{
    if (channel < NUM_420MA_CHANNELS) {
        return analogData.analog_420[channel].status;
    }
    return ANALOG_STATUS_ERROR;
}

/**
 * @brief  Get all 4-20mA data
 * @param  buffer: Buffer to store data
 * @param  bufferSize: Buffer size
 * @retval None
 */
void AnalogInput_GetAll420mA(uint8_t* buffer, uint16_t bufferSize)
{
    if (bufferSize < (NUM_420MA_CHANNELS * 6)) {  // 2 bytes raw + 4 bytes float
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
 * @brief  Get voltage value
 * @param  channel: Channel number (0-5)
 * @retval Voltage in V
 */
float AnalogInput_GetVoltage_V(uint8_t channel)
{
    if (channel < NUM_VOLTAGE_CHANNELS) {
        return analogData.analog_voltage[channel].voltage_V;
    }
    return 0.0f;
}

/**
 * @brief  Get voltage scaled percentage
 * @param  channel: Channel number (0-5)
 * @retval Percentage (0-100%)
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
 * @param  channel: Channel number (0-5)
 * @retval Status code
 */
AnalogStatus_t AnalogInput_GetVoltage_Status(uint8_t channel)
{
    if (channel < NUM_VOLTAGE_CHANNELS) {
        return analogData.analog_voltage[channel].status;
    }
    return ANALOG_STATUS_ERROR;
}

/**
 * @brief  Get all voltage data
 * @param  buffer: Buffer to store data
 * @param  bufferSize: Buffer size
 * @retval None
 */
void AnalogInput_GetAllVoltage(uint8_t* buffer, uint16_t bufferSize)
{
    if (bufferSize < (NUM_VOLTAGE_CHANNELS * 6)) {
        return;
    }
    
    uint16_t offset = 0;
    for (uint8_t i = 0; i < NUM_VOLTAGE_CHANNELS; i++) {
        memcpy(&buffer[offset], &analogData.analog_voltage[i].raw_adc, 2);
        offset += 2;
        memcpy(&buffer[offset], &analogData.analog_voltage[i].voltage_V, 4);
        offset += 4;
    }
}

/**
 * @brief  Get NTC temperature
 * @param  channel: Channel number (0-3)
 * @retval Temperature in °C
 */
float AnalogInput_GetNTC_Temperature(uint8_t channel)
{
    if (channel < NUM_NTC_CHANNELS) {
        return analogData.ntc[channel].temperature_C;
    }
    return 0.0f;
}

/**
 * @brief  Get NTC resistance
 * @param  channel: Channel number (0-3)
 * @retval Resistance in Ohms
 */
float AnalogInput_GetNTC_Resistance(uint8_t channel)
{
    if (channel < NUM_NTC_CHANNELS) {
        return analogData.ntc[channel].resistance_ohm;
    }
    return 0.0f;
}

/**
 * @brief  Get NTC status
 * @param  channel: Channel number (0-3)
 * @retval Status code
 */
AnalogStatus_t AnalogInput_GetNTC_Status(uint8_t channel)
{
    if (channel < NUM_NTC_CHANNELS) {
        return analogData.ntc[channel].status;
    }
    return ANALOG_STATUS_ERROR;
}

/**
 * @brief  Get all NTC data
 * @param  buffer: Buffer to store data
 * @param  bufferSize: Buffer size
 * @retval None
 */
void AnalogInput_GetAllNTC(uint8_t* buffer, uint16_t bufferSize)
{
    if (bufferSize < (NUM_NTC_CHANNELS * 6)) {
        return;
    }
    
    uint16_t offset = 0;
    for (uint8_t i = 0; i < NUM_NTC_CHANNELS; i++) {
        memcpy(&buffer[offset], &analogData.ntc[i].raw_adc, 2);
        offset += 2;
        memcpy(&buffer[offset], &analogData.ntc[i].temperature_C, 4);
        offset += 4;
    }
}

/**
 * @brief  Get all analog data
 * @param  buffer: Buffer to store data
 * @param  bufferSize: Buffer size
 * @retval None
 */
void AnalogInput_GetAllData(uint8_t* buffer, uint16_t bufferSize)
{
    uint16_t required_size = (NUM_420MA_CHANNELS * 6) + 
                             (NUM_VOLTAGE_CHANNELS * 6) + 
                             (NUM_NTC_CHANNELS * 6);
    
    if (bufferSize < required_size) {
        return;
    }
    
    uint16_t offset = 0;
    
    /* Copy 4-20mA data */
    AnalogInput_GetAll420mA(&buffer[offset], bufferSize - offset);
    offset += (NUM_420MA_CHANNELS * 6);
    
    /* Copy voltage data */
    AnalogInput_GetAllVoltage(&buffer[offset], bufferSize - offset);
    offset += (NUM_VOLTAGE_CHANNELS * 6);
    
    /* Copy NTC data */
    AnalogInput_GetAllNTC(&buffer[offset], bufferSize - offset);
}

/**
 * @brief  Get analog data structure pointer
 * @retval Pointer to analog data
 */
AnalogData_t* AnalogInput_GetDataStructure(void)
{
    return &analogData;
}

/**
 * @brief  Set update rate
 * @param  rate_ms: Update rate in milliseconds
 * @retval None
 */
void AnalogInput_SetUpdateRate(uint32_t rate_ms)
{
    update_rate_ms = rate_ms;
}

/**
 * @brief  Calibrate 4-20mA channel
 * @param  channel: Channel number
 * @param  offset: Offset value
 * @param  gain: Gain value
 * @retval None
 */
void AnalogInput_Calibrate420mA(uint8_t channel, float offset, float gain)
{
    if (channel < NUM_420MA_CHANNELS) {
        calibration_420_offset[channel] = offset;
        calibration_420_gain[channel] = gain;
    }
}

/**
 * @brief  Calibrate voltage channel
 * @param  channel: Channel number
 * @param  offset: Offset value
 * @param  gain: Gain value
 * @retval None
 */
void AnalogInput_CalibrateVoltage(uint8_t channel, float offset, float gain)
{
    if (channel < NUM_VOLTAGE_CHANNELS) {
        calibration_voltage_offset[channel] = offset;
        calibration_voltage_gain[channel] = gain;
    }
}

/* Private Functions */

/**
 * @brief  Convert ADC value to 4-20mA current
 * @param  adc_value: Raw ADC value
 * @retval Current in mA
 */
static float Convert_ADC_To_420mA(uint16_t adc_value)
{
    /* Voltage at ADC input */
    float voltage = ((float)adc_value / ADC_RESOLUTION) * ADC_VREF;
    
    /* Current through sense resistor */
    float current_mA = (voltage / CURRENT_SENSE_RESISTOR) * 1000.0f;
    
    return current_mA;
}

/**
 * @brief  Convert ADC value to voltage
 * @param  adc_value: Raw ADC value
 * @retval Voltage in V
 */
static float Convert_ADC_To_Voltage(uint16_t adc_value)
{
    /* Voltage at ADC input */
    float voltage = ((float)adc_value / ADC_RESOLUTION) * ADC_VREF;
    
    /* Compensate for voltage divider */
    voltage *= VOLTAGE_DIVIDER_RATIO;
    
    return voltage;
}

/**
 * @brief  Calculate NTC resistance from ADC value
 * @param  adc_value: Raw ADC value
 * @retval Resistance in Ohms
 */
static float Calculate_NTC_Resistance(uint16_t adc_value)
{
    /* Voltage at ADC input */
    float voltage = ((float)adc_value / ADC_RESOLUTION) * ADC_VREF;
    
    /* Calculate NTC resistance using voltage divider formula */
    float resistance = NTC_SERIES_RESISTOR * (ADC_VREF / voltage - 1.0f);
    
    return resistance;
}

/**
 * @brief  Convert ADC value to NTC temperature using Beta equation
 * @param  adc_value: Raw ADC value
 * @retval Temperature in °C
 */
static float Convert_ADC_To_NTC_Temperature(uint16_t adc_value)
{
    float resistance = Calculate_NTC_Resistance(adc_value);
    
    /* Avoid division by zero */
    if (resistance <= 0.0f) {
        return -273.15f;
    }
    
    /* Beta parameter equation for NTC */
    float steinhart = 1.0f / (NTC_NOMINAL_TEMP + 273.15f);
    steinhart += (1.0f / NTC_BETA_COEFFICIENT) * logf(resistance / NTC_NOMINAL_RESISTANCE);
    float temperature_K = 1.0f / steinhart;
    float temperature_C = temperature_K - 273.15f;
    
    return temperature_C;
}

/**
 * @brief  Check 4-20mA status
 * @param  current_mA: Current value
 * @retval Status code
 */
static AnalogStatus_t Check_420mA_Status(float current_mA)
{
    if (current_mA < CURRENT_UNDERRANGE_MA) {
        return ANALOG_STATUS_UNDERRANGE;  // Wire break
    }
    else if (current_mA > CURRENT_OVERRANGE_MA) {
        return ANALOG_STATUS_OVERRANGE;
    }
    return ANALOG_STATUS_OK;
}

/**
 * @brief  Check voltage status
 * @param  voltage_V: Voltage value
 * @retval Status code
 */
static AnalogStatus_t Check_Voltage_Status(float voltage_V)
{
    if (voltage_V < VOLTAGE_MIN_V) {
        return ANALOG_STATUS_UNDERRANGE;
    }
    else if (voltage_V > (VOLTAGE_MAX_V + 1.0f)) {
        return ANALOG_STATUS_OVERRANGE;
    }
    return ANALOG_STATUS_OK;
}

/**
 * @brief  Check NTC status
 * @param  adc_value: Raw ADC value
 * @retval Status code
 */
static AnalogStatus_t Check_NTC_Status(uint16_t adc_value)
{
    /* Check for open circuit (very high resistance) */
    if (adc_value > 64000) {
        return ANALOG_STATUS_OPEN_CIRCUIT;
    }
    /* Check for short circuit (very low resistance) */
    else if (adc_value < 500) {
        return ANALOG_STATUS_SHORT_CIRCUIT;
    }
    return ANALOG_STATUS_OK;
}


