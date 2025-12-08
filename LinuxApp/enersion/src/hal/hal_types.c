/**
 * @file hal_types.c
 * @brief HAL Layer - Common Type Implementations
 * @version 1.0.0
 * @date 2024
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 * @note MISRA C:2012 Compliant
 */

#include "hal/hal_types.h"
#include <string.h>

/* ============================================================================
 * Static Error Strings (MISRA Rule 8.9 - File scope where possible)
 * ============================================================================ */

static const char * const g_error_strings[] = {
    "OK",                       /* HAL_OK */
    "Invalid parameter",        /* HAL_ERR_PARAM */
    "Timeout",                  /* HAL_ERR_TIMEOUT */
    "I/O error",                /* HAL_ERR_IO */
    "Resource busy",            /* HAL_ERR_BUSY */
    "Memory allocation failed", /* HAL_ERR_MEMORY */
    "Not initialized",          /* HAL_ERR_NOT_INIT */
    "Buffer overflow",          /* HAL_ERR_OVERFLOW */
    "CRC mismatch",             /* HAL_ERR_CRC */
    "Protocol error",           /* HAL_ERR_PROTOCOL */
    "Resource not found",       /* HAL_ERR_NOT_FOUND */
    "Permission denied",        /* HAL_ERR_PERMISSION */
};

#define ERROR_STRING_COUNT  (sizeof(g_error_strings) / sizeof(g_error_strings[0]))

/* ============================================================================
 * Public Functions
 * ============================================================================ */

const char *hal_error_str(hal_error_t error)
{
    const char *result;
    int32_t index;
    
    /* Convert negative error to positive index */
    if (error <= HAL_OK) {
        index = -((int32_t)error);
    } else {
        index = -1;  /* Invalid */
    }
    
    /* MISRA Rule 14.3 - Bounds check */
    if ((index >= 0) && ((size_t)index < ERROR_STRING_COUNT)) {
        result = g_error_strings[index];
    } else {
        result = "Unknown error";
    }
    
    return result;
}

hal_error_t hal_buffer_init(hal_buffer_t *buffer)
{
    hal_error_t result;
    
    /* MISRA Rule 17.7 - Check parameters */
    if (HAL_IS_NULL(buffer)) {
        result = HAL_ERR_PARAM;
    } else {
        /* MISRA Rule 21.17 - Use memset for initialization */
        (void)memset(buffer->data, 0, sizeof(buffer->data));
        buffer->length = 0U;
        buffer->capacity = HAL_MAX_BUFFER_SIZE;
        result = HAL_OK;
    }
    
    return result;
}

hal_error_t hal_buffer_clear(hal_buffer_t *buffer)
{
    hal_error_t result;
    
    if (HAL_IS_NULL(buffer)) {
        result = HAL_ERR_PARAM;
    } else {
        (void)memset(buffer->data, 0, buffer->length);
        buffer->length = 0U;
        result = HAL_OK;
    }
    
    return result;
}

