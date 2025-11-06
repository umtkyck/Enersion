/**
 ******************************************************************************
 * @file           : debug_uart.c
 * @brief          : UART Debug messaging implementation
 ******************************************************************************
 */

#include "debug_uart.h"
#include <stdio.h>
#include <string.h>

/* External UART Handle */
extern UART_HandleTypeDef huart1;

/* Private Variables */
static DebugLevel_t currentDebugLevel = DEBUG_DEFAULT_LEVEL;
static char debugBuffer[DEBUG_BUFFER_SIZE];
static uint32_t systemTicks = 0;

/* Level Names */
static const char* levelNames[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "VERB "
};

/**
 * @brief  Initialize debug UART interface
 * @retval None
 */
void Debug_Init(void)
{
    currentDebugLevel = DEBUG_DEFAULT_LEVEL;
    DEBUG_INFO("Debug UART initialized");
}

/**
 * @brief  Set debug level
 * @param  level: New debug level
 * @retval None
 */
void Debug_SetLevel(DebugLevel_t level)
{
    currentDebugLevel = level;
}

/**
 * @brief  Print formatted debug message
 * @param  level: Debug level
 * @param  format: Printf-style format string
 * @retval None
 */
void Debug_Print(DebugLevel_t level, const char* format, ...)
{
    if (level > currentDebugLevel) {
        return;
    }

    va_list args;
    va_start(args, format);

    /* Get system tick for timestamp */
    systemTicks = HAL_GetTick();

    /* Format message */
    int offset = 0;
    
#if DEBUG_TIMESTAMP_ENABLED
    offset = snprintf(debugBuffer, DEBUG_BUFFER_SIZE, "[%8lu] ", systemTicks);
#endif

    /* Add level */
    offset += snprintf(debugBuffer + offset, DEBUG_BUFFER_SIZE - offset, 
                       "[%s] ", levelNames[level]);

    /* Add user message */
    offset += vsnprintf(debugBuffer + offset, DEBUG_BUFFER_SIZE - offset, 
                        format, args);

    /* Add newline */
    if (offset < DEBUG_BUFFER_SIZE - 2) {
        debugBuffer[offset++] = '\r';
        debugBuffer[offset++] = '\n';
        debugBuffer[offset] = '\0';
    }

    va_end(args);

    /* Transmit via UART */
    HAL_UART_Transmit(&huart1, (uint8_t*)debugBuffer, offset, 100);
}

/**
 * @brief  Print raw string without formatting
 * @param  str: String to print
 * @retval None
 */
void Debug_PrintRaw(const char* str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
}

/**
 * @brief  Print data as hexadecimal
 * @param  data: Data buffer
 * @param  length: Data length
 * @retval None
 */
void Debug_PrintHex(const uint8_t* data, uint16_t length)
{
    char hexStr[8];
    Debug_PrintRaw("HEX: ");
    
    for (uint16_t i = 0; i < length; i++) {
        snprintf(hexStr, sizeof(hexStr), "%02X ", data[i]);
        Debug_PrintRaw(hexStr);
        
        if ((i + 1) % 16 == 0) {
            Debug_PrintRaw("\r\n     ");
        }
    }
    
    Debug_PrintRaw("\r\n");
}


