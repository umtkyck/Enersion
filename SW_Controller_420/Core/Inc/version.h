/**
 ******************************************************************************
 * @file           : version.h
 * @brief          : Version information for SW_Controller_420
 ******************************************************************************
 * @attention
 *
 * All code and comments in English language
 *
 ******************************************************************************
 */

#ifndef VERSION_H
#define VERSION_H

/* Firmware Version Information */
#define FW_VERSION_MAJOR        1
#define FW_VERSION_MINOR        0
#define FW_VERSION_PATCH        0
#define FW_BUILD_NUMBER         1

/* Hardware Version */
#define HW_VERSION              "R1M1"

/* MCU Identification */
#define MCU_ID                  0x01    /* Controller 420 (4-20mA Interface) */
#define MCU_NAME                "CONTROLLER_420"

/* Build Information */
#define BUILD_DATE              __DATE__
#define BUILD_TIME              __TIME__

/* Version String Helper */
#define VERSION_STRING_SIZE     64

/* Function Prototypes */
void Version_GetString(char* buffer, uint32_t size);
uint32_t Version_GetFirmwareVersion(void);

#endif /* VERSION_H */


