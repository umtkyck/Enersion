/**
 * @file hal_types.h
 * @brief HAL Layer Common Type Definitions
 * @version 1.0.0
 * @date 2024
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 * 
 * @note MISRA C:2012 Compliant
 * @note Target: STM32MP257 MYIR Board
 */

#ifndef HAL_TYPES_H
#define HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * MISRA C:2012 Rule 21.6 - Use fixed-width integer types
 * ============================================================================ */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Version Information
 * ============================================================================ */
#define HAL_VERSION_MAJOR   1U
#define HAL_VERSION_MINOR   0U
#define HAL_VERSION_PATCH   0U

/* ============================================================================
 * Return Codes (MISRA Rule 17.4 - All functions should return status)
 * ============================================================================ */

/**
 * @brief HAL Error Codes
 * @note MISRA Rule 10.1 - Use explicit enum values
 */
typedef enum {
    HAL_OK              = 0,    /**< Operation successful */
    HAL_ERR_PARAM       = -1,   /**< Invalid parameter */
    HAL_ERR_TIMEOUT     = -2,   /**< Operation timed out */
    HAL_ERR_IO          = -3,   /**< I/O error */
    HAL_ERR_BUSY        = -4,   /**< Resource busy */
    HAL_ERR_MEMORY      = -5,   /**< Memory allocation failed */
    HAL_ERR_NOT_INIT    = -6,   /**< Not initialized */
    HAL_ERR_OVERFLOW    = -7,   /**< Buffer overflow */
    HAL_ERR_CRC         = -8,   /**< CRC mismatch */
    HAL_ERR_PROTOCOL    = -9,   /**< Protocol error */
    HAL_ERR_NOT_FOUND   = -10,  /**< Resource not found */
    HAL_ERR_PERMISSION  = -11,  /**< Permission denied */
} hal_error_t;

/* ============================================================================
 * Serial Configuration Types
 * ============================================================================ */

/**
 * @brief Serial port parity options
 */
typedef enum {
    HAL_PARITY_NONE = 0,    /**< No parity */
    HAL_PARITY_EVEN = 1,    /**< Even parity */
    HAL_PARITY_ODD  = 2,    /**< Odd parity */
} hal_parity_t;

/**
 * @brief Serial port stop bits
 */
typedef enum {
    HAL_STOPBITS_1 = 1,     /**< 1 stop bit */
    HAL_STOPBITS_2 = 2,     /**< 2 stop bits */
} hal_stopbits_t;

/**
 * @brief Serial port data bits
 */
typedef enum {
    HAL_DATABITS_7 = 7,     /**< 7 data bits */
    HAL_DATABITS_8 = 8,     /**< 8 data bits */
} hal_databits_t;

/**
 * @brief Serial port configuration structure
 * @note All fields must be explicitly initialized
 */
typedef struct {
    const char      *device;        /**< Device path (e.g., "/dev/ttySTM0") */
    uint32_t        baudrate;       /**< Baud rate (e.g., 115200) */
    hal_databits_t  data_bits;      /**< Data bits (7 or 8) */
    hal_parity_t    parity;         /**< Parity setting */
    hal_stopbits_t  stop_bits;      /**< Stop bits (1 or 2) */
    bool            rs485_mode;     /**< Enable RS485 half-duplex mode */
    uint32_t        timeout_ms;     /**< Read timeout in milliseconds */
} hal_serial_config_t;

/**
 * @brief Serial port statistics
 */
typedef struct {
    uint64_t bytes_tx;      /**< Total bytes transmitted */
    uint64_t bytes_rx;      /**< Total bytes received */
    uint32_t errors_frame;  /**< Frame errors */
    uint32_t errors_parity; /**< Parity errors */
    uint32_t errors_overrun;/**< Overrun errors */
} hal_serial_stats_t;

/* ============================================================================
 * Buffer Types
 * ============================================================================ */

/**
 * @brief Maximum buffer sizes
 * @note MISRA Rule 11.4 - Define explicit buffer limits
 */
#define HAL_MAX_DEVICE_PATH     256U
#define HAL_MAX_BUFFER_SIZE     512U
#define HAL_MAX_FRAME_SIZE      260U

/**
 * @brief Byte buffer with length
 */
typedef struct {
    uint8_t     data[HAL_MAX_BUFFER_SIZE];  /**< Buffer data */
    size_t      length;                      /**< Current data length */
    size_t      capacity;                    /**< Buffer capacity */
} hal_buffer_t;

/* ============================================================================
 * Callback Types
 * ============================================================================ */

/**
 * @brief Data received callback type
 * @param data Pointer to received data
 * @param length Number of bytes received
 * @param user_data User context pointer
 */
typedef void (*hal_rx_callback_t)(const uint8_t *data, size_t length, void *user_data);

/**
 * @brief Error callback type
 * @param error Error code
 * @param user_data User context pointer
 */
typedef void (*hal_error_callback_t)(hal_error_t error, void *user_data);

/* ============================================================================
 * Utility Macros (MISRA Compliant)
 * ============================================================================ */

/**
 * @brief Null pointer check macro
 * @note MISRA Rule 11.5 - Explicit null checks
 */
#define HAL_IS_NULL(ptr)        ((ptr) == NULL)

/**
 * @brief Array element count macro
 * @note MISRA Rule 20.7 - Macro parameter in parentheses
 */
#define HAL_ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

/**
 * @brief Minimum of two values
 * @note MISRA Rule 20.7 - Macro parameters in parentheses
 */
#define HAL_MIN(a, b)           (((a) < (b)) ? (a) : (b))

/**
 * @brief Maximum of two values
 */
#define HAL_MAX(a, b)           (((a) > (b)) ? (a) : (b))

/**
 * @brief Unused parameter macro (suppress warnings)
 * @note MISRA Rule 2.7 - Handle unused parameters explicitly
 */
#define HAL_UNUSED(x)           ((void)(x))

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Get error string for HAL error code
 * @param error Error code
 * @return Pointer to error string (static, do not free)
 */
const char *hal_error_str(hal_error_t error);

/**
 * @brief Initialize HAL buffer
 * @param buffer Pointer to buffer structure
 * @return HAL_OK on success
 */
hal_error_t hal_buffer_init(hal_buffer_t *buffer);

/**
 * @brief Clear HAL buffer
 * @param buffer Pointer to buffer structure
 * @return HAL_OK on success
 */
hal_error_t hal_buffer_clear(hal_buffer_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TYPES_H */

