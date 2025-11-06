/**
 ******************************************************************************
 * @file           : digital_output_handler.h
 * @brief          : Digital Output Handler Layer
 ******************************************************************************
 * @attention
 *
 * Application Layer for controlling digital outputs
 * Provides safe output control with error checking
 *
 ******************************************************************************
 */

#ifndef DIGITAL_OUTPUT_HANDLER_H
#define DIGITAL_OUTPUT_HANDLER_H

#include "main.h"

/* Number of digital outputs */
#define NUM_DIGITAL_OUTPUTS     56

/* Digital Output Structure */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t currentState;
} DigitalOutput_t;

/* Function Prototypes */
void DigitalOutput_Init(void);
void DigitalOutput_Set(uint8_t outputNum, uint8_t state);
void DigitalOutput_SetAll(const uint8_t* buffer, uint16_t bufferSize);
uint8_t DigitalOutput_Get(uint8_t outputNum);
void DigitalOutput_GetAll(uint8_t* buffer, uint16_t bufferSize);
void DigitalOutput_Toggle(uint8_t outputNum);

#endif /* DIGITAL_OUTPUT_HANDLER_H */

