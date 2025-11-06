/**
 ******************************************************************************
 * @file           : debug_uart.h
 * @brief          : UART Debug messaging interface
 ******************************************************************************
 * @attention
 *
 * Hardware Abstraction Layer for UART debug output
 * USART1 @ 115200 baud for debug messages
 *
 ******************************************************************************
 */

#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include "main.h"
#include <stdarg.h>

/* Debug Levels */
typedef enum {
    DEBUG_LEVEL_ERROR = 0,
    DEBUG_LEVEL_WARNING,
    DEBUG_LEVEL_INFO,
    DEBUG_LEVEL_DEBUG,
    DEBUG_LEVEL_VERBOSE
} DebugLevel_t;

/* Configuration */
#define DEBUG_ENABLED           1
#define DEBUG_DEFAULT_LEVEL     DEBUG_LEVEL_INFO
#define DEBUG_BUFFER_SIZE       256
#define DEBUG_TIMESTAMP_ENABLED 1

/* Function Prototypes */
void Debug_Init(void);
void Debug_SetLevel(DebugLevel_t level);
void Debug_Print(DebugLevel_t level, const char* format, ...);
void Debug_PrintRaw(const char* str);
void Debug_PrintHex(const uint8_t* data, uint16_t length);

/* Convenience Macros */
#if DEBUG_ENABLED
    #define DEBUG_ERROR(...)    Debug_Print(DEBUG_LEVEL_ERROR, __VA_ARGS__)
    #define DEBUG_WARNING(...)  Debug_Print(DEBUG_LEVEL_WARNING, __VA_ARGS__)
    #define DEBUG_INFO(...)     Debug_Print(DEBUG_LEVEL_INFO, __VA_ARGS__)
    #define DEBUG_DEBUG(...)    Debug_Print(DEBUG_LEVEL_DEBUG, __VA_ARGS__)
    #define DEBUG_VERBOSE(...)  Debug_Print(DEBUG_LEVEL_VERBOSE, __VA_ARGS__)
#else
    #define DEBUG_ERROR(...)
    #define DEBUG_WARNING(...)
    #define DEBUG_INFO(...)
    #define DEBUG_DEBUG(...)
    #define DEBUG_VERBOSE(...)
#endif

#endif /* DEBUG_UART_H */

