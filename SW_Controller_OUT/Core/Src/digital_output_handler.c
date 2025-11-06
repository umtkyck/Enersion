/**
 ******************************************************************************
 * @file           : digital_output_handler.c
 * @brief          : Digital Output Handler Implementation
 ******************************************************************************
 */

#include "digital_output_handler.h"
#include "debug_uart.h"
#include <string.h>

/* Digital Output Configuration */
static DigitalOutput_t digitalOutputs[NUM_DIGITAL_OUTPUTS];
static uint8_t outputStates[NUM_DIGITAL_OUTPUTS];

/* Output pin mapping based on main.c GPIO configuration */
static const struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} outputPinMap[] = {
    /* Port F outputs */
    {GPIOF, GPIO_PIN_0}, {GPIOF, GPIO_PIN_1}, {GPIOF, GPIO_PIN_2}, {GPIOF, GPIO_PIN_3},
    {GPIOF, GPIO_PIN_4}, {GPIOF, GPIO_PIN_5}, {GPIOF, GPIO_PIN_6}, {GPIOF, GPIO_PIN_7},
    {GPIOF, GPIO_PIN_8}, {GPIOF, GPIO_PIN_9}, {GPIOF, GPIO_PIN_10}, {GPIOF, GPIO_PIN_11},
    {GPIOF, GPIO_PIN_12},
    /* Port C outputs */
    {GPIOC, GPIO_PIN_0}, {GPIOC, GPIO_PIN_1}, {GPIOC, GPIO_PIN_2}, {GPIOC, GPIO_PIN_3},
    {GPIOC, GPIO_PIN_4}, {GPIOC, GPIO_PIN_5}, {GPIOC, GPIO_PIN_6}, {GPIOC, GPIO_PIN_7},
    {GPIOC, GPIO_PIN_8}, {GPIOC, GPIO_PIN_9},
    /* Port A outputs */
    {GPIOA, GPIO_PIN_0}, {GPIOA, GPIO_PIN_1}, {GPIOA, GPIO_PIN_2}, {GPIOA, GPIO_PIN_3},
    {GPIOA, GPIO_PIN_4}, {GPIOA, GPIO_PIN_5}, {GPIOA, GPIO_PIN_6}, {GPIOA, GPIO_PIN_7},
    {GPIOA, GPIO_PIN_8}, {GPIOA, GPIO_PIN_9},
    /* Port B outputs */
    {GPIOB, GPIO_PIN_0}, {GPIOB, GPIO_PIN_1}, {GPIOB, GPIO_PIN_2}, {GPIOB, GPIO_PIN_10},
    {GPIOB, GPIO_PIN_11}, {GPIOB, GPIO_PIN_12}, {GPIOB, GPIO_PIN_13},
    /* Port E outputs */
    {GPIOE, GPIO_PIN_15},
    /* Port D outputs */
    {GPIOD, GPIO_PIN_8}, {GPIOD, GPIO_PIN_9}, {GPIOD, GPIO_PIN_10}, {GPIOD, GPIO_PIN_11},
    {GPIOD, GPIO_PIN_12}, {GPIOD, GPIO_PIN_13}, {GPIOD, GPIO_PIN_14}, {GPIOD, GPIO_PIN_15},
    {GPIOD, GPIO_PIN_1}, {GPIOD, GPIO_PIN_2},
    /* Port G outputs */
    {GPIOG, GPIO_PIN_2}, {GPIOG, GPIO_PIN_3}, {GPIOG, GPIO_PIN_4}, {GPIOG, GPIO_PIN_5},
    {GPIOG, GPIO_PIN_6}, {GPIOG, GPIO_PIN_7}, {GPIOG, GPIO_PIN_8}
};

#define NUM_OUTPUT_PINS (sizeof(outputPinMap) / sizeof(outputPinMap[0]))

/**
 * @brief  Initialize digital output handler
 * @retval None
 */
void DigitalOutput_Init(void)
{
    memset(digitalOutputs, 0, sizeof(digitalOutputs));
    memset(outputStates, 0, sizeof(outputStates));
    
    /* Configure output structures */
    for (uint8_t i = 0; i < NUM_OUTPUT_PINS && i < NUM_DIGITAL_OUTPUTS; i++) {
        digitalOutputs[i].port = outputPinMap[i].port;
        digitalOutputs[i].pin = outputPinMap[i].pin;
        digitalOutputs[i].currentState = 0;
        
        /* Initialize outputs to low */
        HAL_GPIO_WritePin(digitalOutputs[i].port, digitalOutputs[i].pin, GPIO_PIN_RESET);
    }
    
    DEBUG_INFO("Digital Output Handler initialized, %d outputs", NUM_OUTPUT_PINS);
}

/**
 * @brief  Set single digital output
 * @param  outputNum: Output number (0-63)
 * @param  state: Output state (0 or 1)
 * @retval None
 */
void DigitalOutput_Set(uint8_t outputNum, uint8_t state)
{
    if (outputNum < NUM_OUTPUT_PINS && outputNum < NUM_DIGITAL_OUTPUTS) {
        GPIO_PinState pinState = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(digitalOutputs[outputNum].port, 
                         digitalOutputs[outputNum].pin, 
                         pinState);
        
        digitalOutputs[outputNum].currentState = state;
        outputStates[outputNum] = state;
    }
}

/**
 * @brief  Set all digital outputs from byte array
 * @param  buffer: Buffer containing output states
 * @param  bufferSize: Buffer size
 * @retval None
 */
void DigitalOutput_SetAll(const uint8_t* buffer, uint16_t bufferSize)
{
    uint16_t numBytes = (NUM_DIGITAL_OUTPUTS + 7) / 8;
    
    if (numBytes > bufferSize) {
        numBytes = bufferSize;
    }
    
    /* Unpack bits from bytes and set outputs */
    for (uint16_t i = 0; i < NUM_DIGITAL_OUTPUTS && i < (numBytes * 8); i++) {
        uint8_t state = (buffer[i / 8] >> (i % 8)) & 0x01;
        DigitalOutput_Set(i, state);
    }
    
    DEBUG_DEBUG("All outputs set");
}

/**
 * @brief  Get single digital output state
 * @param  outputNum: Output number
 * @retval Output state (0 or 1)
 */
uint8_t DigitalOutput_Get(uint8_t outputNum)
{
    if (outputNum < NUM_DIGITAL_OUTPUTS) {
        return outputStates[outputNum];
    }
    return 0;
}

/**
 * @brief  Get all digital outputs as byte array
 * @param  buffer: Buffer to store output states
 * @param  bufferSize: Buffer size
 * @retval None
 */
void DigitalOutput_GetAll(uint8_t* buffer, uint16_t bufferSize)
{
    uint16_t numBytes = 7;  // 56 outputs = 7 bytes
    
    if (numBytes > bufferSize) {
        numBytes = bufferSize;
    }
    
    memset(buffer, 0, numBytes);
    
    /* Pack bits into bytes (56 outputs) */
    for (uint16_t i = 0; i < NUM_DIGITAL_OUTPUTS && i < (numBytes * 8); i++) {
        if (outputStates[i]) {
            buffer[i / 8] |= (1 << (i % 8));
        }
    }
}

/**
 * @brief  Toggle digital output
 * @param  outputNum: Output number
 * @retval None
 */
void DigitalOutput_Toggle(uint8_t outputNum)
{
    if (outputNum < NUM_DIGITAL_OUTPUTS) {
        DigitalOutput_Set(outputNum, !outputStates[outputNum]);
    }
}

