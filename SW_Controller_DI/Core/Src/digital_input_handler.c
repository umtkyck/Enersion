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

/* Input pin mapping - MUST match main.h MCU_DI0-DI55 definitions exactly */
static const struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} inputPinMap[] = {
    /* DI0-DI3: Port F (PF12-PF15) */
    {GPIOF, GPIO_PIN_12}, // DI0  = PF12
    {GPIOF, GPIO_PIN_13}, // DI1  = PF13
    {GPIOF, GPIO_PIN_14}, // DI2  = PF14
    {GPIOF, GPIO_PIN_15}, // DI3  = PF15
    
    /* DI4-DI5: Port G (PG0-PG1) */
    {GPIOG, GPIO_PIN_0},  // DI4  = PG0
    {GPIOG, GPIO_PIN_1},  // DI5  = PG1
    
    /* DI6-DI14: Port E (PE7-PE15) */
    {GPIOE, GPIO_PIN_7},  // DI6  = PE7
    {GPIOE, GPIO_PIN_8},  // DI7  = PE8
    {GPIOE, GPIO_PIN_9},  // DI8  = PE9
    {GPIOE, GPIO_PIN_10}, // DI9  = PE10
    {GPIOE, GPIO_PIN_11}, // DI10 = PE11
    {GPIOE, GPIO_PIN_12}, // DI11 = PE12
    {GPIOE, GPIO_PIN_13}, // DI12 = PE13
    {GPIOE, GPIO_PIN_14}, // DI13 = PE14
    {GPIOE, GPIO_PIN_15}, // DI14 = PE15
    
    /* DI15-DI18: Port B (PB10-PB13) */
    {GPIOB, GPIO_PIN_10}, // DI15 = PB10
    {GPIOB, GPIO_PIN_11}, // DI16 = PB11
    {GPIOB, GPIO_PIN_12}, // DI17 = PB12
    {GPIOB, GPIO_PIN_13}, // DI18 = PB13
    
    /* DI19-DI26: Port D (PD8-PD15) */
    {GPIOD, GPIO_PIN_8},  // DI19 = PD8
    {GPIOD, GPIO_PIN_9},  // DI20 = PD9
    {GPIOD, GPIO_PIN_10}, // DI21 = PD10
    {GPIOD, GPIO_PIN_11}, // DI22 = PD11
    {GPIOD, GPIO_PIN_12}, // DI23 = PD12
    {GPIOD, GPIO_PIN_13}, // DI24 = PD13
    {GPIOD, GPIO_PIN_14}, // DI25 = PD14
    {GPIOD, GPIO_PIN_15}, // DI26 = PD15
    
    /* DI27-DI33: Port G (PG2-PG8) */
    {GPIOG, GPIO_PIN_2},  // DI27 = PG2
    {GPIOG, GPIO_PIN_3},  // DI28 = PG3
    {GPIOG, GPIO_PIN_4},  // DI29 = PG4
    {GPIOG, GPIO_PIN_5},  // DI30 = PG5
    {GPIOG, GPIO_PIN_6},  // DI31 = PG6
    {GPIOG, GPIO_PIN_7},  // DI32 = PG7
    {GPIOG, GPIO_PIN_8},  // DI33 = PG8
    
    /* DI34-DI37: Port C (PC6-PC9) */
    {GPIOC, GPIO_PIN_6},  // DI34 = PC6
    {GPIOC, GPIO_PIN_7},  // DI35 = PC7
    {GPIOC, GPIO_PIN_8},  // DI36 = PC8
    {GPIOC, GPIO_PIN_9},  // DI37 = PC9
    
    /* DI38-DI39: Port A (PA8-PA9) */
    {GPIOA, GPIO_PIN_8},  // DI38 = PA8
    {GPIOA, GPIO_PIN_9},  // DI39 = PA9
    
    /* DI40: Port A (PA15) */
    {GPIOA, GPIO_PIN_15}, // DI40 = PA15
    
    /* DI41-DI43: Port C (PC10-PC12) */
    {GPIOC, GPIO_PIN_10}, // DI41 = PC10
    {GPIOC, GPIO_PIN_11}, // DI42 = PC11
    {GPIOC, GPIO_PIN_12}, // DI43 = PC12
    
    /* DI44: Port D (PD0) */
    {GPIOD, GPIO_PIN_0},  // DI44 = PD0
    
    /* DI45: Port D (PD3) */
    {GPIOD, GPIO_PIN_3},  // DI45 = PD3
    
    /* DI46: Port D (PD7) */
    {GPIOD, GPIO_PIN_7},  // DI46 = PD7
    
    /* DI47-DI53: Port G (PG9-PG15) */
    {GPIOG, GPIO_PIN_9},  // DI47 = PG9
    {GPIOG, GPIO_PIN_10}, // DI48 = PG10
    {GPIOG, GPIO_PIN_11}, // DI49 = PG11
    {GPIOG, GPIO_PIN_12}, // DI50 = PG12
    {GPIOG, GPIO_PIN_13}, // DI51 = PG13
    {GPIOG, GPIO_PIN_14}, // DI52 = PG14
    {GPIOG, GPIO_PIN_15}, // DI53 = PG15
    
    /* DI54-DI55: Port B (PB3-PB4) */
    {GPIOB, GPIO_PIN_3},  // DI54 = PB3
    {GPIOB, GPIO_PIN_4}   // DI55 = PB4
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

