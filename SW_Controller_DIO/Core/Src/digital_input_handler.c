/**
 ******************************************************************************
 * @file           : digital_input_handler.c
 * @brief          : Digital Input Handler Implementation
 ******************************************************************************
 */

#include "digital_input_handler.h"
#include "debug_uart.h"
#include <string.h>

/* Digital Input Configuration */
static DigitalInput_t digitalInputs[NUM_DIGITAL_INPUTS];
static uint8_t inputStates[NUM_DIGITAL_INPUTS];

/* Input pin mapping based on main.c GPIO configuration */
static const struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} inputPinMap[] = {
    /* Port F inputs */
    {GPIOF, GPIO_PIN_12}, {GPIOF, GPIO_PIN_13}, {GPIOF, GPIO_PIN_14}, {GPIOF, GPIO_PIN_15},
    /* Port G inputs */
    {GPIOG, GPIO_PIN_0}, {GPIOG, GPIO_PIN_1}, {GPIOG, GPIO_PIN_2}, {GPIOG, GPIO_PIN_3},
    {GPIOG, GPIO_PIN_4}, {GPIOG, GPIO_PIN_5}, {GPIOG, GPIO_PIN_6}, {GPIOG, GPIO_PIN_7},
    {GPIOG, GPIO_PIN_8}, {GPIOG, GPIO_PIN_9}, {GPIOG, GPIO_PIN_10}, {GPIOG, GPIO_PIN_11},
    {GPIOG, GPIO_PIN_12}, {GPIOG, GPIO_PIN_13}, {GPIOG, GPIO_PIN_14}, {GPIOG, GPIO_PIN_15},
    /* Port E inputs */
    {GPIOE, GPIO_PIN_7}, {GPIOE, GPIO_PIN_8}, {GPIOE, GPIO_PIN_9}, {GPIOE, GPIO_PIN_10},
    {GPIOE, GPIO_PIN_11}, {GPIOE, GPIO_PIN_12}, {GPIOE, GPIO_PIN_13}, {GPIOE, GPIO_PIN_14},
    {GPIOE, GPIO_PIN_15},
    /* Port B inputs */
    {GPIOB, GPIO_PIN_10}, {GPIOB, GPIO_PIN_11}, {GPIOB, GPIO_PIN_12}, {GPIOB, GPIO_PIN_13},
    {GPIOB, GPIO_PIN_3}, {GPIOB, GPIO_PIN_4},
    /* Port D inputs */
    {GPIOD, GPIO_PIN_8}, {GPIOD, GPIO_PIN_9}, {GPIOD, GPIO_PIN_10}, {GPIOD, GPIO_PIN_11},
    {GPIOD, GPIO_PIN_12}, {GPIOD, GPIO_PIN_13}, {GPIOD, GPIO_PIN_14}, {GPIOD, GPIO_PIN_15},
    {GPIOD, GPIO_PIN_0}, {GPIOD, GPIO_PIN_3}, {GPIOD, GPIO_PIN_7},
    /* Port C inputs */
    {GPIOC, GPIO_PIN_6}, {GPIOC, GPIO_PIN_7}, {GPIOC, GPIO_PIN_8}, {GPIOC, GPIO_PIN_9},
    {GPIOC, GPIO_PIN_10}, {GPIOC, GPIO_PIN_11}, {GPIOC, GPIO_PIN_12},
    /* Port A inputs */
    {GPIOA, GPIO_PIN_8}, {GPIOA, GPIO_PIN_9}, {GPIOA, GPIO_PIN_15}
};

#define NUM_INPUT_PINS (sizeof(inputPinMap) / sizeof(inputPinMap[0]))

/**
 * @brief  Initialize digital input handler
 * @retval None
 */
void DigitalInput_Init(void)
{
    memset(digitalInputs, 0, sizeof(digitalInputs));
    memset(inputStates, 0, sizeof(inputStates));
    
    /* Configure input structures */
    for (uint8_t i = 0; i < NUM_INPUT_PINS && i < NUM_DIGITAL_INPUTS; i++) {
        digitalInputs[i].port = inputPinMap[i].port;
        digitalInputs[i].pin = inputPinMap[i].pin;
        digitalInputs[i].currentState = 0;
        digitalInputs[i].previousState = 0;
        digitalInputs[i].lastChangeTime = 0;
    }
    
    DEBUG_INFO("Digital Input Handler initialized, %d inputs", NUM_INPUT_PINS);
}

/**
 * @brief  Update digital inputs (call periodically)
 * @retval None
 */
void DigitalInput_Update(void)
{
    uint32_t currentTime = HAL_GetTick();
    
    for (uint8_t i = 0; i < NUM_INPUT_PINS && i < NUM_DIGITAL_INPUTS; i++) {
        if (digitalInputs[i].port != NULL) {
            /* Read GPIO pin */
            GPIO_PinState pinState = HAL_GPIO_ReadPin(digitalInputs[i].port, 
                                                      digitalInputs[i].pin);
            
            uint8_t newState = (pinState == GPIO_PIN_SET) ? 1 : 0;
            
            /* Debounce logic */
            if (newState != digitalInputs[i].currentState) {
                if ((currentTime - digitalInputs[i].lastChangeTime) >= DEBOUNCE_TIME_MS) {
                    digitalInputs[i].previousState = digitalInputs[i].currentState;
                    digitalInputs[i].currentState = newState;
                    digitalInputs[i].lastChangeTime = currentTime;
                    
                    inputStates[i] = newState;
                }
            }
        }
    }
}

/**
 * @brief  Read single digital input
 * @param  inputNum: Input number (0-63)
 * @retval Input state (0 or 1)
 */
uint8_t DigitalInput_Read(uint8_t inputNum)
{
    if (inputNum < NUM_DIGITAL_INPUTS) {
        return inputStates[inputNum];
    }
    return 0;
}

/**
 * @brief  Get all digital inputs as byte array
 * @param  buffer: Buffer to store input states
 * @param  bufferSize: Buffer size
 * @retval None
 */
void DigitalInput_GetAll(uint8_t* buffer, uint16_t bufferSize)
{
    uint16_t numBytes = 7;  // 56 inputs = 7 bytes
    
    if (numBytes > bufferSize) {
        numBytes = bufferSize;
    }
    
    memset(buffer, 0, numBytes);
    
    /* Pack bits into bytes (56 inputs) */
    for (uint16_t i = 0; i < NUM_DIGITAL_INPUTS && i < (numBytes * 8); i++) {
        if (inputStates[i]) {
            buffer[i / 8] |= (1 << (i % 8));
        }
    }
}

/**
 * @brief  Check if input has changed
 * @param  inputNum: Input number
 * @retval 1 if changed, 0 otherwise
 */
uint8_t DigitalInput_HasChanged(uint8_t inputNum)
{
    if (inputNum < NUM_DIGITAL_INPUTS) {
        return (digitalInputs[inputNum].currentState != digitalInputs[inputNum].previousState);
    }
    return 0;
}

