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

/* Output pin mapping - MUST match main.h MCU_DO0-DO55 definitions exactly */
static const struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} outputPinMap[] = {
    /* DO0-DO10: Port F (PF0-PF10) */
    {GPIOF, GPIO_PIN_0},  // DO0  = PF0
    {GPIOF, GPIO_PIN_1},  // DO1  = PF1
    {GPIOF, GPIO_PIN_2},  // DO2  = PF2
    {GPIOF, GPIO_PIN_3},  // DO3  = PF3
    {GPIOF, GPIO_PIN_4},  // DO4  = PF4
    {GPIOF, GPIO_PIN_5},  // DO5  = PF5
    {GPIOF, GPIO_PIN_6},  // DO6  = PF6
    {GPIOF, GPIO_PIN_7},  // DO7  = PF7
    {GPIOF, GPIO_PIN_8},  // DO8  = PF8
    {GPIOF, GPIO_PIN_9},  // DO9  = PF9
    {GPIOF, GPIO_PIN_10}, // DO10 = PF10
    
    /* DO11-DO14: Port C (PC0-PC3) */
    {GPIOC, GPIO_PIN_0},  // DO11 = PC0
    {GPIOC, GPIO_PIN_1},  // DO12 = PC1
    {GPIOC, GPIO_PIN_2},  // DO13 = PC2
    {GPIOC, GPIO_PIN_3},  // DO14 = PC3
    
    /* DO15-DO22: Port A (PA0-PA7) */
    {GPIOA, GPIO_PIN_0},  // DO15 = PA0
    {GPIOA, GPIO_PIN_1},  // DO16 = PA1
    {GPIOA, GPIO_PIN_2},  // DO17 = PA2
    {GPIOA, GPIO_PIN_3},  // DO18 = PA3
    {GPIOA, GPIO_PIN_4},  // DO19 = PA4
    {GPIOA, GPIO_PIN_5},  // DO20 = PA5
    {GPIOA, GPIO_PIN_6},  // DO21 = PA6
    {GPIOA, GPIO_PIN_7},  // DO22 = PA7
    
    /* DO23-DO24: Port C (PC4-PC5) */
    {GPIOC, GPIO_PIN_4},  // DO23 = PC4
    {GPIOC, GPIO_PIN_5},  // DO24 = PC5
    
    /* DO25-DO27: Port B (PB0-PB2) */
    {GPIOB, GPIO_PIN_0},  // DO25 = PB0
    {GPIOB, GPIO_PIN_1},  // DO26 = PB1
    {GPIOB, GPIO_PIN_2},  // DO27 = PB2
    
    /* DO28-DO29: Port F (PF11-PF12) */
    {GPIOF, GPIO_PIN_11}, // DO28 = PF11
    {GPIOF, GPIO_PIN_12}, // DO29 = PF12
    
    /* DO30: Port E (PE15) */
    {GPIOE, GPIO_PIN_15}, // DO30 = PE15
    
    /* DO31-DO34: Port B (PB10-PB13) */
    {GPIOB, GPIO_PIN_10}, // DO31 = PB10
    {GPIOB, GPIO_PIN_11}, // DO32 = PB11
    {GPIOB, GPIO_PIN_12}, // DO33 = PB12
    {GPIOB, GPIO_PIN_13}, // DO34 = PB13
    
    /* DO35-DO42: Port D (PD8-PD15) */
    {GPIOD, GPIO_PIN_8},  // DO35 = PD8
    {GPIOD, GPIO_PIN_9},  // DO36 = PD9
    {GPIOD, GPIO_PIN_10}, // DO37 = PD10
    {GPIOD, GPIO_PIN_11}, // DO38 = PD11
    {GPIOD, GPIO_PIN_12}, // DO39 = PD12
    {GPIOD, GPIO_PIN_13}, // DO40 = PD13
    {GPIOD, GPIO_PIN_14}, // DO41 = PD14
    {GPIOD, GPIO_PIN_15}, // DO42 = PD15
    
    /* DO43-DO49: Port G (PG2-PG8) */
    {GPIOG, GPIO_PIN_2},  // DO43 = PG2
    {GPIOG, GPIO_PIN_3},  // DO44 = PG3
    {GPIOG, GPIO_PIN_4},  // DO45 = PG4
    {GPIOG, GPIO_PIN_5},  // DO46 = PG5
    {GPIOG, GPIO_PIN_6},  // DO47 = PG6
    {GPIOG, GPIO_PIN_7},  // DO48 = PG7
    {GPIOG, GPIO_PIN_8},  // DO49 = PG8
    
    /* DO50-DO53: Port C (PC6-PC9) */
    {GPIOC, GPIO_PIN_6},  // DO50 = PC6
    {GPIOC, GPIO_PIN_7},  // DO51 = PC7
    {GPIOC, GPIO_PIN_8},  // DO52 = PC8
    {GPIOC, GPIO_PIN_9},  // DO53 = PC9
    
    /* DO54-DO55: Port A (PA8-PA9) */
    {GPIOA, GPIO_PIN_8},  // DO54 = PA8
    {GPIOA, GPIO_PIN_9}   // DO55 = PA9
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

