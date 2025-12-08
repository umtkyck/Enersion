/**
 * @file service_dio.h
 * @brief Service Layer - Digital I/O Management
 * @version 1.0.0
 * @date 2024
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 * 
 * @note MISRA C:2012 Compliant
 * 
 * High-level interface for Digital Input/Output operations.
 * Provides automatic polling, state caching, and change notifications.
 */

#ifndef SERVICE_DIO_H
#define SERVICE_DIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "service/service_types.h"
#include "hal/hal_types.h"

/* ============================================================================
 * Opaque Handle
 * ============================================================================ */

/**
 * @brief DIO Service handle (opaque)
 */
typedef struct service_dio_handle service_dio_handle_t;

/* ============================================================================
 * Lifecycle Functions
 * ============================================================================ */

/**
 * @brief Create DIO service instance
 * 
 * @param[in] config Service configuration
 * @param[out] handle Pointer to receive handle
 * @return HAL_OK on success
 */
hal_error_t service_dio_create(const service_config_t *config,
                               service_dio_handle_t **handle);

/**
 * @brief Destroy DIO service instance
 * 
 * @param[in,out] handle Pointer to handle (set to NULL)
 * @return HAL_OK on success
 */
hal_error_t service_dio_destroy(service_dio_handle_t **handle);

/**
 * @brief Start DIO service (connect and begin polling)
 * 
 * @param[in] handle Service handle
 * @return HAL_OK on success
 */
hal_error_t service_dio_start(service_dio_handle_t *handle);

/**
 * @brief Stop DIO service (stop polling and disconnect)
 * 
 * @param[in] handle Service handle
 * @return HAL_OK on success
 */
hal_error_t service_dio_stop(service_dio_handle_t *handle);

/**
 * @brief Check if service is running
 * 
 * @param[in] handle Service handle
 * @param[out] running Pointer to receive status
 * @return HAL_OK on success
 */
hal_error_t service_dio_is_running(const service_dio_handle_t *handle,
                                   bool *running);

/* ============================================================================
 * Callback Registration
 * ============================================================================ */

/**
 * @brief Register state change callback
 * 
 * @param[in] handle Service handle
 * @param[in] callback Callback function
 * @param[in] user_data User context passed to callback
 * @return HAL_OK on success
 */
hal_error_t service_dio_set_state_callback(service_dio_handle_t *handle,
                                           service_state_callback_t callback,
                                           void *user_data);

/**
 * @brief Register DI change callback
 * 
 * @param[in] handle Service handle
 * @param[in] callback Callback function
 * @param[in] user_data User context passed to callback
 * @return HAL_OK on success
 */
hal_error_t service_dio_set_di_callback(service_dio_handle_t *handle,
                                        service_di_callback_t callback,
                                        void *user_data);

/**
 * @brief Register DO confirm callback
 * 
 * @param[in] handle Service handle
 * @param[in] callback Callback function
 * @param[in] user_data User context passed to callback
 * @return HAL_OK on success
 */
hal_error_t service_dio_set_do_callback(service_dio_handle_t *handle,
                                        service_do_callback_t callback,
                                        void *user_data);

/**
 * @brief Register error callback
 * 
 * @param[in] handle Service handle
 * @param[in] callback Callback function
 * @param[in] user_data User context passed to callback
 * @return HAL_OK on success
 */
hal_error_t service_dio_set_error_callback(service_dio_handle_t *handle,
                                           service_error_callback_t callback,
                                           void *user_data);

/* ============================================================================
 * Device Discovery
 * ============================================================================ */

/**
 * @brief Scan for DI/DO devices on bus
 * 
 * @param[in] handle Service handle
 * @param[out] di_online DI controller found
 * @param[out] do_online DO controller found
 * @return HAL_OK on success
 */
hal_error_t service_dio_scan_devices(service_dio_handle_t *handle,
                                     bool *di_online,
                                     bool *do_online);

/**
 * @brief Get device information
 * 
 * @param[in] handle Service handle
 * @param[in] device Device type (DEVICE_TYPE_DI or DEVICE_TYPE_DO)
 * @param[out] info Device information
 * @return HAL_OK on success
 */
hal_error_t service_dio_get_device_info(const service_dio_handle_t *handle,
                                        device_type_t device,
                                        service_device_info_t *info);

/* ============================================================================
 * Digital Input Operations
 * ============================================================================ */

/**
 * @brief Read all digital inputs (cached)
 * 
 * @param[in] handle Service handle
 * @param[out] state Current input state
 * @return HAL_OK on success
 * 
 * @note Returns cached value, updated by background polling
 */
hal_error_t service_dio_read_inputs(const service_dio_handle_t *handle,
                                    protocol_dio_state_t *state);

/**
 * @brief Read single digital input (cached)
 * 
 * @param[in] handle Service handle
 * @param[in] channel Channel number (0-63)
 * @param[out] value Input state
 * @return HAL_OK on success
 */
hal_error_t service_dio_read_input(const service_dio_handle_t *handle,
                                   uint8_t channel,
                                   bool *value);

/**
 * @brief Force immediate DI read (blocking)
 * 
 * @param[in] handle Service handle
 * @param[out] state Current input state
 * @return HAL_OK on success
 */
hal_error_t service_dio_read_inputs_now(service_dio_handle_t *handle,
                                        protocol_dio_state_t *state);

/**
 * @brief Get DI data with metadata
 * 
 * @param[in] handle Service handle
 * @param[out] data DI data including timestamps
 * @return HAL_OK on success
 */
hal_error_t service_dio_get_di_data(const service_dio_handle_t *handle,
                                    service_di_data_t *data);

/* ============================================================================
 * Digital Output Operations
 * ============================================================================ */

/**
 * @brief Write all digital outputs
 * 
 * @param[in] handle Service handle
 * @param[in] state Output state to write
 * @return HAL_OK on success
 * 
 * @note Asynchronous - result via callback
 */
hal_error_t service_dio_write_outputs(service_dio_handle_t *handle,
                                      const protocol_dio_state_t *state);

/**
 * @brief Write single digital output
 * 
 * @param[in] handle Service handle
 * @param[in] channel Channel number (0-63)
 * @param[in] value Output state
 * @return HAL_OK on success
 */
hal_error_t service_dio_write_output(service_dio_handle_t *handle,
                                     uint8_t channel,
                                     bool value);

/**
 * @brief Toggle single digital output
 * 
 * @param[in] handle Service handle
 * @param[in] channel Channel number (0-63)
 * @return HAL_OK on success
 */
hal_error_t service_dio_toggle_output(service_dio_handle_t *handle,
                                      uint8_t channel);

/**
 * @brief Read current output state
 * 
 * @param[in] handle Service handle
 * @param[out] state Current output state
 * @return HAL_OK on success
 */
hal_error_t service_dio_read_outputs(const service_dio_handle_t *handle,
                                     protocol_dio_state_t *state);

/**
 * @brief Force immediate DO read (blocking)
 * 
 * @param[in] handle Service handle
 * @param[out] state Current output state
 * @return HAL_OK on success
 */
hal_error_t service_dio_read_outputs_now(service_dio_handle_t *handle,
                                         protocol_dio_state_t *state);

/**
 * @brief Get DO data with metadata
 * 
 * @param[in] handle Service handle
 * @param[out] data DO data including timestamps
 * @return HAL_OK on success
 */
hal_error_t service_dio_get_do_data(const service_dio_handle_t *handle,
                                    service_do_data_t *data);

/**
 * @brief Set all outputs OFF
 * 
 * @param[in] handle Service handle
 * @return HAL_OK on success
 */
hal_error_t service_dio_all_outputs_off(service_dio_handle_t *handle);

/**
 * @brief Set all outputs ON
 * 
 * @param[in] handle Service handle
 * @return HAL_OK on success
 */
hal_error_t service_dio_all_outputs_on(service_dio_handle_t *handle);

/* ============================================================================
 * Statistics
 * ============================================================================ */

/**
 * @brief Get service statistics
 * 
 * @param[in] handle Service handle
 * @param[out] stats Statistics structure
 * @return HAL_OK on success
 */
hal_error_t service_dio_get_stats(const service_dio_handle_t *handle,
                                  service_stats_t *stats);

/**
 * @brief Reset statistics
 * 
 * @param[in] handle Service handle
 * @return HAL_OK on success
 */
hal_error_t service_dio_reset_stats(service_dio_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_DIO_H */

