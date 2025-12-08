/**
 * @file hal_serial.h
 * @brief HAL Layer - Serial Port Interface (RS485)
 * @version 1.0.0
 * @date 2024
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 * 
 * @note MISRA C:2012 Compliant
 * @note Target: STM32MP257 MYIR Board
 * 
 * This module provides a hardware abstraction layer for serial communication
 * with RS485 support. It abstracts Linux-specific serial port operations.
 */

#ifndef HAL_SERIAL_H
#define HAL_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal/hal_types.h"
#include <sys/types.h>  /* For ssize_t */

/* ============================================================================
 * Constants
 * ============================================================================ */

/** @brief Default serial configuration */
#define HAL_SERIAL_DEFAULT_BAUDRATE     115200U
#define HAL_SERIAL_DEFAULT_TIMEOUT_MS   1000U

/** @brief STM32MP257 UART device paths */
#define HAL_SERIAL_DEV_UART4            "/dev/ttySTM0"
#define HAL_SERIAL_DEV_UART5            "/dev/ttySTM1"
#define HAL_SERIAL_DEV_USART2           "/dev/ttySTM2"

/* ============================================================================
 * Opaque Handle
 * ============================================================================ */

/**
 * @brief Serial port handle (opaque)
 * @note MISRA Rule 11.6 - Use opaque pointers for encapsulation
 */
typedef struct hal_serial_handle hal_serial_handle_t;

/* ============================================================================
 * Lifecycle Functions
 * ============================================================================ */

/**
 * @brief Create serial port handle with default configuration
 * 
 * @param[in] device Device path (e.g., "/dev/ttySTM0")
 * @param[out] handle Pointer to receive handle
 * @return HAL_OK on success, error code otherwise
 * 
 * @note MISRA Rule 17.7 - Return value must be checked
 * @note Default: 115200 8N1, RS485 mode enabled
 */
hal_error_t hal_serial_create(const char *device, hal_serial_handle_t **handle);

/**
 * @brief Create serial port handle with custom configuration
 * 
 * @param[in] config Configuration structure
 * @param[out] handle Pointer to receive handle
 * @return HAL_OK on success, error code otherwise
 */
hal_error_t hal_serial_create_with_config(const hal_serial_config_t *config,
                                          hal_serial_handle_t **handle);

/**
 * @brief Destroy serial port handle and free resources
 * 
 * @param[in,out] handle Pointer to handle (will be set to NULL)
 * @return HAL_OK on success
 * 
 * @note MISRA Rule 18.6 - Prevent dangling pointers
 */
hal_error_t hal_serial_destroy(hal_serial_handle_t **handle);

/* ============================================================================
 * Connection Functions
 * ============================================================================ */

/**
 * @brief Open serial port connection
 * 
 * @param[in] handle Serial handle
 * @return HAL_OK on success
 */
hal_error_t hal_serial_open(hal_serial_handle_t *handle);

/**
 * @brief Close serial port connection
 * 
 * @param[in] handle Serial handle
 * @return HAL_OK on success
 */
hal_error_t hal_serial_close(hal_serial_handle_t *handle);

/**
 * @brief Check if serial port is open
 * 
 * @param[in] handle Serial handle
 * @param[out] is_open Pointer to receive status
 * @return HAL_OK on success
 */
hal_error_t hal_serial_is_open(const hal_serial_handle_t *handle, bool *is_open);

/* ============================================================================
 * I/O Functions
 * ============================================================================ */

/**
 * @brief Write data to serial port
 * 
 * @param[in] handle Serial handle
 * @param[in] data Data buffer to write
 * @param[in] length Number of bytes to write
 * @param[out] written Pointer to receive bytes written (can be NULL)
 * @return HAL_OK on success
 * 
 * @note Blocks until all data written or error
 */
hal_error_t hal_serial_write(hal_serial_handle_t *handle,
                             const uint8_t *data,
                             size_t length,
                             size_t *written);

/**
 * @brief Read data from serial port
 * 
 * @param[in] handle Serial handle
 * @param[out] buffer Buffer to receive data
 * @param[in] buffer_size Buffer size
 * @param[out] received Pointer to receive bytes read
 * @return HAL_OK on success, HAL_ERR_TIMEOUT if no data
 * 
 * @note Non-blocking, returns immediately with available data
 */
hal_error_t hal_serial_read(hal_serial_handle_t *handle,
                            uint8_t *buffer,
                            size_t buffer_size,
                            size_t *received);

/**
 * @brief Read data with timeout
 * 
 * @param[in] handle Serial handle
 * @param[out] buffer Buffer to receive data
 * @param[in] buffer_size Buffer size
 * @param[in] timeout_ms Timeout in milliseconds
 * @param[out] received Pointer to receive bytes read
 * @return HAL_OK on success, HAL_ERR_TIMEOUT on timeout
 */
hal_error_t hal_serial_read_timeout(hal_serial_handle_t *handle,
                                    uint8_t *buffer,
                                    size_t buffer_size,
                                    uint32_t timeout_ms,
                                    size_t *received);

/**
 * @brief Flush input buffer (discard unread data)
 * 
 * @param[in] handle Serial handle
 * @return HAL_OK on success
 */
hal_error_t hal_serial_flush_input(hal_serial_handle_t *handle);

/**
 * @brief Flush output buffer (wait for transmission)
 * 
 * @param[in] handle Serial handle
 * @return HAL_OK on success
 */
hal_error_t hal_serial_flush_output(hal_serial_handle_t *handle);

/**
 * @brief Wait for all output to be transmitted
 * 
 * @param[in] handle Serial handle
 * @return HAL_OK on success
 */
hal_error_t hal_serial_drain(hal_serial_handle_t *handle);

/* ============================================================================
 * Configuration Functions
 * ============================================================================ */

/**
 * @brief Set baud rate
 * 
 * @param[in] handle Serial handle
 * @param[in] baudrate New baud rate
 * @return HAL_OK on success
 */
hal_error_t hal_serial_set_baudrate(hal_serial_handle_t *handle, uint32_t baudrate);

/**
 * @brief Set read timeout
 * 
 * @param[in] handle Serial handle
 * @param[in] timeout_ms Timeout in milliseconds (0 = no timeout)
 * @return HAL_OK on success
 */
hal_error_t hal_serial_set_timeout(hal_serial_handle_t *handle, uint32_t timeout_ms);

/**
 * @brief Enable/disable RS485 mode
 * 
 * @param[in] handle Serial handle
 * @param[in] enable true to enable RS485 mode
 * @return HAL_OK on success
 */
hal_error_t hal_serial_set_rs485_mode(hal_serial_handle_t *handle, bool enable);

/* ============================================================================
 * Status Functions
 * ============================================================================ */

/**
 * @brief Get number of bytes available to read
 * 
 * @param[in] handle Serial handle
 * @param[out] available Pointer to receive byte count
 * @return HAL_OK on success
 */
hal_error_t hal_serial_bytes_available(const hal_serial_handle_t *handle,
                                       size_t *available);

/**
 * @brief Get serial port statistics
 * 
 * @param[in] handle Serial handle
 * @param[out] stats Pointer to receive statistics
 * @return HAL_OK on success
 */
hal_error_t hal_serial_get_stats(const hal_serial_handle_t *handle,
                                 hal_serial_stats_t *stats);

/**
 * @brief Reset statistics counters
 * 
 * @param[in] handle Serial handle
 * @return HAL_OK on success
 */
hal_error_t hal_serial_reset_stats(hal_serial_handle_t *handle);

/**
 * @brief Get device path
 * 
 * @param[in] handle Serial handle
 * @param[out] device Pointer to receive device path (do not free)
 * @return HAL_OK on success
 */
hal_error_t hal_serial_get_device(const hal_serial_handle_t *handle,
                                  const char **device);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Calculate Modbus RTU inter-frame delay
 * 
 * @param[in] baudrate Baud rate
 * @return Delay in microseconds (3.5 character times)
 */
uint32_t hal_serial_modbus_delay_us(uint32_t baudrate);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SERIAL_H */

