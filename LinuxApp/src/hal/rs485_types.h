/**
 * @file rs485_types.h
 * @brief RS485 Type Definitions - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef RS485_TYPES_H
#define RS485_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* MISRA C:2012 Rule 21.6 - Use fixed-width integer types */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Constants - MISRA C:2012 Rule 7.2: Use 'u' suffix for unsigned constants
 * ============================================================================ */

#define RS485_MAX_BUFFER_SIZE       (256U)
#define RS485_DEFAULT_BAUDRATE      (115200U)
#define RS485_DEFAULT_TIMEOUT_MS    (1000U)

/* ============================================================================
 * Error Codes - MISRA C:2012 Rule 10.1: Avoid implicit conversions
 * ============================================================================ */

typedef enum rs485_error_tag {
    RS485_OK                    = 0,
    RS485_ERR_NULL_POINTER      = -1,
    RS485_ERR_INVALID_PARAM     = -2,
    RS485_ERR_OPEN_FAILED       = -3,
    RS485_ERR_CONFIG_FAILED     = -4,
    RS485_ERR_WRITE_FAILED      = -5,
    RS485_ERR_READ_FAILED       = -6,
    RS485_ERR_TIMEOUT           = -7,
    RS485_ERR_NOT_OPEN          = -8,
    RS485_ERR_BUFFER_OVERFLOW   = -9
} rs485_error_t;

/* ============================================================================
 * Parity - MISRA C:2012 Rule 8.9: Enum declarations
 * ============================================================================ */

typedef enum rs485_parity_tag {
    RS485_PARITY_NONE = 0,
    RS485_PARITY_EVEN = 1,
    RS485_PARITY_ODD  = 2
} rs485_parity_t;

/* ============================================================================
 * Configuration Structure
 * ============================================================================ */

typedef struct rs485_config_tag {
    const char     *device;             /**< Device path (e.g., "/dev/ttySTM0") */
    uint32_t        baudrate;           /**< Baud rate */
    uint8_t         data_bits;          /**< Data bits (7 or 8) */
    rs485_parity_t  parity;             /**< Parity setting */
    uint8_t         stop_bits;          /**< Stop bits (1 or 2) */
    bool            rs485_mode;         /**< Enable RS485 mode */
    uint32_t        timeout_ms;         /**< Read timeout in milliseconds */
} rs485_config_t;

/* ============================================================================
 * Statistics Structure
 * ============================================================================ */

typedef struct rs485_stats_tag {
    uint64_t bytes_sent;                /**< Total bytes transmitted */
    uint64_t bytes_received;            /**< Total bytes received */
    uint32_t tx_count;                  /**< Transmission count */
    uint32_t rx_count;                  /**< Reception count */
    uint32_t error_count;               /**< Error count */
    uint32_t timeout_count;             /**< Timeout count */
} rs485_stats_t;

/* ============================================================================
 * Handle Type (Opaque pointer for encapsulation)
 * ============================================================================ */

typedef struct rs485_handle_tag *rs485_handle_t;

#ifdef __cplusplus
}
#endif

#endif /* RS485_TYPES_H */

