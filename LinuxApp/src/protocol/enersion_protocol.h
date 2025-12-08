/**
 * @file enersion_protocol.h
 * @brief Enersion Protocol Interface - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef ENERSION_PROTOCOL_H
#define ENERSION_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "enersion_types.h"
#include "rs485_types.h"

/* ============================================================================
 * Configuration
 * ============================================================================ */

#define ENERSION_DEFAULT_TIMEOUT_MS     (500U)
#define ENERSION_DEFAULT_RETRIES        (3U)

/* ============================================================================
 * Lifecycle Functions
 * ============================================================================ */

/**
 * @brief Create protocol context
 * 
 * @param[in]  rs485_handle  RS485 HAL handle
 * @param[out] handle        Output protocol handle
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_create(rs485_handle_t rs485_handle,
                                  enersion_handle_t *handle);

/**
 * @brief Destroy protocol context
 * 
 * @param[in,out] handle  Handle to destroy
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_destroy(enersion_handle_t *handle);

/**
 * @brief Set response timeout
 * 
 * @param[in] handle      Protocol handle
 * @param[in] timeout_ms  Timeout in milliseconds
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_set_timeout(enersion_handle_t handle,
                                       uint32_t timeout_ms);

/**
 * @brief Set retry count
 * 
 * @param[in] handle   Protocol handle
 * @param[in] retries  Retry count
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_set_retries(enersion_handle_t handle,
                                       uint8_t retries);

/* ============================================================================
 * Packet Encoding/Decoding
 * ============================================================================ */

/**
 * @brief Encode packet to wire format
 * 
 * @param[in]  packet       Packet to encode
 * @param[out] buffer       Output buffer
 * @param[in]  buffer_size  Buffer size
 * @param[out] encoded_len  Output: encoded length
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_encode_packet(const enersion_packet_t *packet,
                                         uint8_t *buffer,
                                         size_t buffer_size,
                                         size_t *encoded_len);

/**
 * @brief Decode packet from wire format
 * 
 * @param[in]  buffer      Input buffer
 * @param[in]  buffer_len  Buffer length
 * @param[out] packet      Output packet
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_decode_packet(const uint8_t *buffer,
                                         size_t buffer_len,
                                         enersion_packet_t *packet);

/* ============================================================================
 * Transaction Functions
 * ============================================================================ */

/**
 * @brief Send packet and wait for response
 * 
 * @param[in]  handle    Protocol handle
 * @param[in]  request   Request packet
 * @param[out] response  Response packet
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_transaction(enersion_handle_t handle,
                                       const enersion_packet_t *request,
                                       enersion_packet_t *response);

/**
 * @brief Send packet without waiting for response
 * 
 * @param[in] handle  Protocol handle
 * @param[in] packet  Packet to send
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_send(enersion_handle_t handle,
                                const enersion_packet_t *packet);

/* ============================================================================
 * System Commands
 * ============================================================================ */

/**
 * @brief Ping device
 * 
 * @param[in]  handle   Protocol handle
 * @param[in]  address  Device address
 * @param[out] online   Output: true if device responds
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_ping(enersion_handle_t handle,
                                enersion_addr_t address,
                                bool *online);

/**
 * @brief Get device version
 * 
 * @param[in]  handle   Protocol handle
 * @param[in]  address  Device address
 * @param[out] version  Output: version information
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_get_version(enersion_handle_t handle,
                                       enersion_addr_t address,
                                       enersion_version_t *version);

/**
 * @brief Send heartbeat and get health
 * 
 * @param[in]  handle   Protocol handle
 * @param[in]  address  Device address
 * @param[out] health   Output: health percentage (0-100)
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_heartbeat(enersion_handle_t handle,
                                     enersion_addr_t address,
                                     uint8_t *health);

/**
 * @brief Get device status
 * 
 * @param[in]  handle   Protocol handle
 * @param[in]  address  Device address
 * @param[out] status   Output: status information
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_get_status(enersion_handle_t handle,
                                      enersion_addr_t address,
                                      enersion_status_t *status);

/* ============================================================================
 * Digital Input Commands (Controller DIO - 0x02)
 * ============================================================================ */

/**
 * @brief Read all 64 digital inputs
 * 
 * @param[in]  handle  Protocol handle
 * @param[out] state   Output: input state (64 bits)
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_read_digital_inputs(enersion_handle_t handle,
                                               enersion_dio_state_t *state);

/* ============================================================================
 * Digital Output Commands (Controller OUT - 0x03)
 * ============================================================================ */

/**
 * @brief Write all 64 digital outputs
 * 
 * @param[in] handle  Protocol handle
 * @param[in] state   Output state to write (64 bits)
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_write_digital_outputs(enersion_handle_t handle,
                                                 const enersion_dio_state_t *state);

/**
 * @brief Read current digital output state
 * 
 * @param[in]  handle  Protocol handle
 * @param[out] state   Output: current output state (64 bits)
 * @return enersion_error_t ENERSION_OK on success
 */
enersion_error_t enersion_read_digital_outputs(enersion_handle_t handle,
                                                enersion_dio_state_t *state);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Get error string
 * 
 * @param[in] error  Error code
 * @return const char* Error description
 */
const char *enersion_error_string(enersion_error_t error);

/**
 * @brief Get command name
 * 
 * @param[in] cmd  Command code
 * @return const char* Command name
 */
const char *enersion_command_string(enersion_cmd_t cmd);

/**
 * @brief Get device name by address
 * 
 * @param[in] addr  Device address
 * @return const char* Device name
 */
const char *enersion_device_name(enersion_addr_t addr);

#ifdef __cplusplus
}
#endif

#endif /* ENERSION_PROTOCOL_H */

