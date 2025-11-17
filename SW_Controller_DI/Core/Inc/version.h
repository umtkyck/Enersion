/**
 ******************************************************************************
 * @file           : version.h
 * @brief          : Version information for SW_Controller_DI
 ******************************************************************************
 * @attention
 *
 * All code and comments in English language
 *
 ******************************************************************************
 */

#ifndef VERSION_H
#define VERSION_H

#include <stdint.h>

/* Firmware Version Information */
#define FW_VERSION_MAJOR        1
#define FW_VERSION_MINOR        1
#define FW_VERSION_PATCH        0
#define FW_BUILD_NUMBER         2

/* Hardware Version */
#define HW_VERSION              "R1M1"

/* MCU Identification */
#define MCU_ID                  0x02    /* Controller DI (Digital Inputs) */
#define MCU_NAME                "CONTROLLER_DI"

/* Build Information */
#define BUILD_DATE              __DATE__
#define BUILD_TIME              __TIME__

/* Version String Helper */
#define VERSION_STRING_SIZE     64

/* Function Prototypes */
void Version_GetString(char* buffer, uint32_t size);
uint32_t Version_GetFirmwareVersion(void);

#endif /* VERSION_H */


