/**
 * @file protocol_frame.h
 * @brief Protocol Layer - Frame Encoding/Decoding
 * @version 1.0.0
 * @date 2024
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 * 
 * @note MISRA C:2012 Compliant
 */

#ifndef PROTOCOL_FRAME_H
#define PROTOCOL_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "protocol/protocol_types.h"
#include "hal/hal_types.h"

/* ============================================================================
 * CRC Functions
 * ============================================================================ */

/**
 * @brief Calculate CRC16 checksum (Modbus polynomial)
 * @param data Pointer to data buffer
 * @param length Data length
 * @return CRC16 value
 */
uint16_t protocol_crc16(const uint8_t *data, size_t length);

/**
 * @brief Verify CRC16 of buffer (CRC at end)
 * @param data Buffer including CRC at end
 * @param length Total length including CRC
 * @return true if CRC valid, false otherwise
 */
bool protocol_crc16_verify(const uint8_t *data, size_t length);

/* ============================================================================
 * Frame Encoding
 * ============================================================================ */

/**
 * @brief Encode frame to raw buffer
 * 
 * @param[in] frame Frame structure to encode
 * @param[out] raw_frame Output raw frame buffer
 * @return HAL_OK on success, error code otherwise
 * 
 * Frame format:
 * [START][DEST][SRC][CMD][LEN][DATA...][CRC_LO][CRC_HI][END]
 */
hal_error_t protocol_frame_encode(const protocol_frame_t *frame,
                                  protocol_raw_frame_t *raw_frame);

/**
 * @brief Decode raw buffer to frame structure
 * 
 * @param[in] raw_frame Raw frame buffer
 * @param[out] frame Decoded frame structure
 * @return HAL_OK on success, error code otherwise
 */
hal_error_t protocol_frame_decode(const protocol_raw_frame_t *raw_frame,
                                  protocol_frame_t *frame);

/**
 * @brief Validate raw frame (check delimiters and CRC)
 * 
 * @param[in] raw_frame Raw frame buffer
 * @return HAL_OK if valid, error code otherwise
 */
hal_error_t protocol_frame_validate(const protocol_raw_frame_t *raw_frame);

/* ============================================================================
 * Frame Building Helpers
 * ============================================================================ */

/**
 * @brief Initialize frame with common fields
 * 
 * @param[out] frame Frame to initialize
 * @param[in] dest_addr Destination address
 * @param[in] command Command code
 */
void protocol_frame_init(protocol_frame_t *frame,
                         protocol_address_t dest_addr,
                         protocol_command_t command);

/**
 * @brief Build PING request frame
 * @param[in] dest_addr Target device address
 * @param[out] raw_frame Output raw frame
 * @return HAL_OK on success
 */
hal_error_t protocol_build_ping(protocol_address_t dest_addr,
                                protocol_raw_frame_t *raw_frame);

/**
 * @brief Build GET_VERSION request frame
 * @param[in] dest_addr Target device address
 * @param[out] raw_frame Output raw frame
 * @return HAL_OK on success
 */
hal_error_t protocol_build_get_version(protocol_address_t dest_addr,
                                       protocol_raw_frame_t *raw_frame);

/**
 * @brief Build HEARTBEAT request frame
 * @param[in] dest_addr Target device address
 * @param[out] raw_frame Output raw frame
 * @return HAL_OK on success
 */
hal_error_t protocol_build_heartbeat(protocol_address_t dest_addr,
                                     protocol_raw_frame_t *raw_frame);

/**
 * @brief Build GET_STATUS request frame
 * @param[in] dest_addr Target device address
 * @param[out] raw_frame Output raw frame
 * @return HAL_OK on success
 */
hal_error_t protocol_build_get_status(protocol_address_t dest_addr,
                                      protocol_raw_frame_t *raw_frame);

/**
 * @brief Build READ_DI (Digital Input) request frame
 * @param[out] raw_frame Output raw frame
 * @return HAL_OK on success
 */
hal_error_t protocol_build_read_di(protocol_raw_frame_t *raw_frame);

/**
 * @brief Build READ_DO (Digital Output state) request frame
 * @param[out] raw_frame Output raw frame
 * @return HAL_OK on success
 */
hal_error_t protocol_build_read_do(protocol_raw_frame_t *raw_frame);

/**
 * @brief Build WRITE_DO (Digital Output) request frame
 * @param[in] state Digital output state to write
 * @param[out] raw_frame Output raw frame
 * @return HAL_OK on success
 */
hal_error_t protocol_build_write_do(const protocol_dio_state_t *state,
                                    protocol_raw_frame_t *raw_frame);

/* ============================================================================
 * Response Parsing Helpers
 * ============================================================================ */

/**
 * @brief Parse PING response
 * @param[in] frame Received frame
 * @return HAL_OK if valid ping response
 */
hal_error_t protocol_parse_ping_response(const protocol_frame_t *frame);

/**
 * @brief Parse VERSION response
 * @param[in] frame Received frame
 * @param[out] version Parsed version info
 * @return HAL_OK on success
 */
hal_error_t protocol_parse_version_response(const protocol_frame_t *frame,
                                            protocol_version_t *version);

/**
 * @brief Parse HEARTBEAT response
 * @param[in] frame Received frame
 * @param[out] device_id Device identifier
 * @param[out] health Health percentage
 * @return HAL_OK on success
 */
hal_error_t protocol_parse_heartbeat_response(const protocol_frame_t *frame,
                                              uint8_t *device_id,
                                              uint8_t *health);

/**
 * @brief Parse STATUS response
 * @param[in] frame Received frame
 * @param[out] status Parsed status info
 * @return HAL_OK on success
 */
hal_error_t protocol_parse_status_response(const protocol_frame_t *frame,
                                           protocol_status_t *status);

/**
 * @brief Parse DI (Digital Input) response
 * @param[in] frame Received frame
 * @param[out] state Parsed input state
 * @return HAL_OK on success
 */
hal_error_t protocol_parse_di_response(const protocol_frame_t *frame,
                                       protocol_dio_state_t *state);

/**
 * @brief Parse DO (Digital Output) response
 * @param[in] frame Received frame
 * @param[out] state Parsed output state
 * @return HAL_OK on success
 */
hal_error_t protocol_parse_do_response(const protocol_frame_t *frame,
                                       protocol_dio_state_t *state);

/**
 * @brief Check if frame is error response
 * @param[in] frame Frame to check
 * @param[out] error_code Error code if error response
 * @return true if error response
 */
bool protocol_is_error_response(const protocol_frame_t *frame,
                                protocol_error_t *error_code);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Get command name string
 * @param command Command code
 * @return Command name string
 */
const char *protocol_command_str(protocol_command_t command);

/**
 * @brief Get device name string
 * @param address Device address
 * @return Device name string
 */
const char *protocol_device_str(protocol_address_t address);

/**
 * @brief Get protocol error string
 * @param error Error code
 * @return Error string
 */
const char *protocol_error_str(protocol_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_FRAME_H */

