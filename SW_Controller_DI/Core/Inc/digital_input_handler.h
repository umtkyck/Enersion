/**
 ******************************************************************************
 * @file           : digital_input_handler.h
 * @brief          : Digital Input Handler Layer
 ******************************************************************************
 * @attention
 *
 * Application Layer for reading and managing digital inputs
 * Handles debouncing and change detection
 *
 ******************************************************************************
 */

#ifndef DIGITAL_INPUT_HANDLER_H
#define DIGITAL_INPUT_HANDLER_H

#include "main.h"

/* Number of digital inputs */
#define NUM_DIGITAL_INPUTS      56

/* Debounce time in milliseconds */
#define DEBOUNCE_TIME_MS        20

/* Digital Input Structure */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t currentState;
    uint8_t previousState;
    uint32_t lastChangeTime;
} DigitalInput_t;

/* Function Prototypes */
void DigitalInput_Init(void);
void DigitalInput_Update(void);
uint8_t DigitalInput_Read(uint8_t inputNum);
void DigitalInput_GetAll(uint8_t* buffer, uint16_t bufferSize);
uint8_t DigitalInput_HasChanged(uint8_t inputNum);

#endif /* DIGITAL_INPUT_HANDLER_H */

