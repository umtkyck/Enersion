/**
 * @file rs485_serial.h
 * @brief RS485 Serial Port Interface - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * Hardware Abstraction Layer for RS485 serial communication.
 * Target: STM32MP257 MYIR Board
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef RS485_SERIAL_H
#define RS485_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rs485_types.h"

/* ============================================================================
 * Lifecycle Functions
 * ============================================================================ */

/**
 * @brief Create RS485 handle with configuration
 * 
 * MISRA C:2012 Rule 17.7: Return value shall be used
 * 
 * @param[in]  config  Configuration parameters (must not be NULL)
 * @param[out] handle  Output handle (must not be NULL)
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_create(const rs485_config_t *config, rs485_handle_t *handle);

/**
 * @brief Destroy RS485 handle and release resources
 * 
 * @param[in,out] handle  Handle to destroy (set to NULL after)
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_destroy(rs485_handle_t *handle);

/**
 * @brief Open RS485 serial port
 * 
 * @param[in] handle  Valid RS485 handle
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_open(rs485_handle_t handle);

/**
 * @brief Close RS485 serial port
 * 
 * @param[in] handle  Valid RS485 handle
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_close(rs485_handle_t handle);

/**
 * @brief Check if RS485 port is open
 * 
 * @param[in]  handle   Valid RS485 handle
 * @param[out] is_open  Output: true if port is open
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_is_open(rs485_handle_t handle, bool *is_open);

/* ============================================================================
 * I/O Functions
 * ============================================================================ */

/**
 * @brief Write data to RS485 port
 * 
 * MISRA C:2012 Rule 17.2: Functions shall not call themselves
 * 
 * @param[in]  handle   Valid RS485 handle
 * @param[in]  data     Data buffer to write (must not be NULL)
 * @param[in]  length   Number of bytes to write
 * @param[out] written  Output: actual bytes written
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_write(rs485_handle_t handle,
                          const uint8_t *data,
                          size_t length,
                          size_t *written);

/**
 * @brief Read data from RS485 port with timeout
 * 
 * @param[in]  handle      Valid RS485 handle
 * @param[out] buffer      Buffer to store read data (must not be NULL)
 * @param[in]  buffer_size Buffer size
 * @param[in]  timeout_ms  Timeout in milliseconds (0 = non-blocking)
 * @param[out] bytes_read  Output: actual bytes read
 * @return rs485_error_t RS485_OK on success, RS485_ERR_TIMEOUT on timeout
 */
rs485_error_t rs485_read(rs485_handle_t handle,
                         uint8_t *buffer,
                         size_t buffer_size,
                         uint32_t timeout_ms,
                         size_t *bytes_read);

/**
 * @brief Flush input and output buffers
 * 
 * @param[in] handle  Valid RS485 handle
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_flush(rs485_handle_t handle);

/**
 * @brief Wait for output buffer to drain
 * 
 * @param[in] handle  Valid RS485 handle
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_drain(rs485_handle_t handle);

/* ============================================================================
 * Configuration Functions
 * ============================================================================ */

/**
 * @brief Set baud rate
 * 
 * @param[in] handle    Valid RS485 handle
 * @param[in] baudrate  New baud rate
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_set_baudrate(rs485_handle_t handle, uint32_t baudrate);

/**
 * @brief Set communication parameters
 * 
 * @param[in] handle     Valid RS485 handle
 * @param[in] data_bits  Data bits (7 or 8)
 * @param[in] parity     Parity setting
 * @param[in] stop_bits  Stop bits (1 or 2)
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_set_params(rs485_handle_t handle,
                               uint8_t data_bits,
                               rs485_parity_t parity,
                               uint8_t stop_bits);

/* ============================================================================
 * Status Functions
 * ============================================================================ */

/**
 * @brief Get statistics
 * 
 * @param[in]  handle  Valid RS485 handle
 * @param[out] stats   Output statistics structure
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_get_stats(rs485_handle_t handle, rs485_stats_t *stats);

/**
 * @brief Reset statistics
 * 
 * @param[in] handle  Valid RS485 handle
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_reset_stats(rs485_handle_t handle);

/**
 * @brief Get bytes available to read
 * 
 * @param[in]  handle     Valid RS485 handle
 * @param[out] available  Output: bytes available
 * @return rs485_error_t RS485_OK on success
 */
rs485_error_t rs485_bytes_available(rs485_handle_t handle, size_t *available);

/**
 * @brief Get error description string
 * 
 * @param[in] error  Error code
 * @return const char* Error description (never NULL)
 */
const char *rs485_error_string(rs485_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* RS485_SERIAL_H */

