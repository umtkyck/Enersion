/**
 ******************************************************************************
 * @file           : version.c
 * @brief          : Version information implementation
 ******************************************************************************
 */

#include "version.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief  Get version string
 * @param  buffer: Buffer to store version string
 * @param  size: Buffer size
 * @retval None
 */
void Version_GetString(char* buffer, uint32_t size)
{
    snprintf(buffer, size, "%s v%d.%d.%d.%d HW:%s Built: %s %s",
             MCU_NAME,
             FW_VERSION_MAJOR,
             FW_VERSION_MINOR,
             FW_VERSION_PATCH,
             FW_BUILD_NUMBER,
             HW_VERSION,
             BUILD_DATE,
             BUILD_TIME);
}

/**
 * @brief  Get firmware version as 32-bit value
 * @retval Version encoded as: [Major:8][Minor:8][Patch:8][Build:8]
 */
uint32_t Version_GetFirmwareVersion(void)
{
    return (FW_VERSION_MAJOR << 24) | 
           (FW_VERSION_MINOR << 16) | 
           (FW_VERSION_PATCH << 8) | 
           FW_BUILD_NUMBER;
}


