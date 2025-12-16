/**
 * @file enersion_protocol.c
 * @brief Enersion Protocol Implementation - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "enersion_protocol.h"
#include "enersion_crc.h"
#include "rs485_serial.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Internal Structure
 * ============================================================================ */

struct enersion_ctx_tag {
    rs485_handle_t rs485;           /**< RS485 HAL handle */
    uint32_t       timeout_ms;      /**< Response timeout */
    uint8_t        retries;         /**< Retry count */
    uint8_t        my_address;      /**< Own address */
    uint8_t        tx_buffer[ENERSION_MAX_PACKET_SIZE];
    uint8_t        rx_buffer[ENERSION_MAX_PACKET_SIZE];
};

/* ============================================================================
 * Error String Table
 * ============================================================================ */

static const char * const error_strings[] = {
    "Success",
    "Null pointer",
    "Invalid parameter",
    "CRC error",
    "Invalid packet",
    "Timeout",
    "I/O error",
    "Buffer overflow",
    "Not connected",
    "Device error"
};

static const char * const cmd_strings[] = {
    "UNKNOWN",
    "PING",
    "PING_RESPONSE",
    "GET_VERSION",
    "VERSION_RESPONSE",
    "HEARTBEAT",
    "HEARTBEAT_RESPONSE"
};

static const char * const device_names[] = {
    "Broadcast",
    "Controller 4-20mA",
    "Controller DIO",
    "Controller OUT"
};

/* ============================================================================
 * Public Functions
 * ============================================================================ */

enersion_error_t enersion_create(rs485_handle_t rs485_handle,
                                  enersion_handle_t *handle)
{
    enersion_error_t result = ENERSION_OK;
    struct enersion_ctx_tag *ctx = NULL;
    
    if ((rs485_handle == NULL) || (handle == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    } else {
        ctx = (struct enersion_ctx_tag *)calloc(1U, sizeof(struct enersion_ctx_tag));
        if (ctx == NULL) {
            result = ENERSION_ERR_INVALID_PARAM;
        }
    }
    
    if ((result == ENERSION_OK) && (ctx != NULL)) {
        ctx->rs485 = rs485_handle;
        ctx->timeout_ms = ENERSION_DEFAULT_TIMEOUT_MS;
        ctx->retries = ENERSION_DEFAULT_RETRIES;
        ctx->my_address = (uint8_t)ENERSION_ADDR_MASTER;
        
        *handle = ctx;
    }
    
    return result;
}

enersion_error_t enersion_destroy(enersion_handle_t *handle)
{
    enersion_error_t result = ENERSION_OK;
    
    if (handle == NULL) {
        result = ENERSION_ERR_NULL_POINTER;
    } else if (*handle != NULL) {
        free(*handle);
        *handle = NULL;
    } else {
        /* Already NULL */
    }
    
    return result;
}

enersion_error_t enersion_set_timeout(enersion_handle_t handle,
                                       uint32_t timeout_ms)
{
    enersion_error_t result = ENERSION_OK;
    
    if (handle == NULL) {
        result = ENERSION_ERR_NULL_POINTER;
    } else {
        handle->timeout_ms = timeout_ms;
    }
    
    return result;
}

enersion_error_t enersion_set_retries(enersion_handle_t handle,
                                       uint8_t retries)
{
    enersion_error_t result = ENERSION_OK;
    
    if (handle == NULL) {
        result = ENERSION_ERR_NULL_POINTER;
    } else {
        handle->retries = retries;
    }
    
    return result;
}

enersion_error_t enersion_encode_packet(const enersion_packet_t *packet,
                                         uint8_t *buffer,
                                         size_t buffer_size,
                                         size_t *encoded_len)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((packet == NULL) || (buffer == NULL) || (encoded_len == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    } else {
        size_t required = ENERSION_HEADER_SIZE + (size_t)packet->data_len +
                          ENERSION_CRC_SIZE + ENERSION_FOOTER_SIZE;
        
        if (buffer_size < required) {
            result = ENERSION_ERR_BUFFER_OVERFLOW;
        }
    }
    
    if (result == ENERSION_OK) {
        size_t idx = 0U;
        
        /* Start byte */
        buffer[idx] = ENERSION_START_BYTE;
        idx++;
        
        /* Header */
        buffer[idx] = packet->dest_addr;
        idx++;
        buffer[idx] = packet->src_addr;
        idx++;
        buffer[idx] = packet->command;
        idx++;
        buffer[idx] = packet->data_len;
        idx++;
        
        /* Data */
        if (packet->data_len > 0U) {
            (void)memcpy(&buffer[idx], packet->data, packet->data_len);
            idx += packet->data_len;
        }
        
        /* CRC - calculated over header and data (excluding start byte) */
        uint16_t crc = enersion_crc16(&buffer[1], idx - 1U);
        buffer[idx] = (uint8_t)(crc & 0xFFU);
        idx++;
        buffer[idx] = (uint8_t)((crc >> 8U) & 0xFFU);
        idx++;
        
        /* End byte */
        buffer[idx] = ENERSION_END_BYTE;
        idx++;
        
        *encoded_len = idx;
    }
    
    return result;
}

enersion_error_t enersion_decode_packet(const uint8_t *buffer,
                                         size_t buffer_len,
                                         enersion_packet_t *packet)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((buffer == NULL) || (packet == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    } else if (buffer_len < (ENERSION_HEADER_SIZE + ENERSION_CRC_SIZE + ENERSION_FOOTER_SIZE)) {
        result = ENERSION_ERR_INVALID_PACKET;
    } else if (buffer[0] != ENERSION_START_BYTE) {
        result = ENERSION_ERR_INVALID_PACKET;
    } else if (buffer[buffer_len - 1U] != ENERSION_END_BYTE) {
        result = ENERSION_ERR_INVALID_PACKET;
    } else {
        /* Verify CRC */
        size_t crc_start = buffer_len - ENERSION_CRC_SIZE - ENERSION_FOOTER_SIZE;
        uint16_t received_crc = (uint16_t)buffer[crc_start] |
                                ((uint16_t)buffer[crc_start + 1U] << 8U);
        uint16_t calculated_crc = enersion_crc16(&buffer[1], crc_start - 1U);
        
        if (received_crc != calculated_crc) {
            result = ENERSION_ERR_INVALID_CRC;
        }
    }
    
    if (result == ENERSION_OK) {
        /* Parse header */
        packet->dest_addr = buffer[1];
        packet->src_addr = buffer[2];
        packet->command = buffer[3];
        packet->data_len = buffer[4];
        
        /* Validate data length */
        size_t expected_len = ENERSION_HEADER_SIZE + (size_t)packet->data_len +
                              ENERSION_CRC_SIZE + ENERSION_FOOTER_SIZE;
        
        if (buffer_len != expected_len) {
            result = ENERSION_ERR_INVALID_PACKET;
        } else if (packet->data_len > 0U) {
            (void)memcpy(packet->data, &buffer[5], packet->data_len);
        } else {
            /* No data to copy */
        }
    }
    
    return result;
}

enersion_error_t enersion_transaction(enersion_handle_t handle,
                                       const enersion_packet_t *request,
                                       enersion_packet_t *response)
{
    enersion_error_t result = ENERSION_OK;
    struct enersion_ctx_tag *ctx = handle;
    
    if ((ctx == NULL) || (request == NULL) || (response == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        uint8_t retries = ctx->retries;
        
        do {
            /* Encode request */
            size_t tx_len = 0U;
            result = enersion_encode_packet(request, ctx->tx_buffer,
                                            sizeof(ctx->tx_buffer), &tx_len);
            
            if (result != ENERSION_OK) {
                break;
            }
            
            /* Flush input buffer */
            (void)rs485_flush(ctx->rs485);
            
            /* Send request */
            size_t written = 0U;
            rs485_error_t rs_err = rs485_write(ctx->rs485, ctx->tx_buffer,
                                               tx_len, &written);
            
            if ((rs_err != RS485_OK) || (written != tx_len)) {
                result = ENERSION_ERR_IO;
                continue;
            }
            
            /* Wait for transmission to complete */
            (void)rs485_drain(ctx->rs485);
            
            /* Read response */
            size_t rx_len = 0U;
            size_t total_read = 0U;
            uint32_t timeout_remaining = ctx->timeout_ms;
            
            while (total_read < sizeof(ctx->rx_buffer)) {
                size_t bytes_read = 0U;
                rs_err = rs485_read(ctx->rs485, &ctx->rx_buffer[total_read],
                                    sizeof(ctx->rx_buffer) - total_read,
                                    timeout_remaining, &bytes_read);
                
                if (rs_err == RS485_ERR_TIMEOUT) {
                    if (total_read >= (ENERSION_HEADER_SIZE + ENERSION_CRC_SIZE + ENERSION_FOOTER_SIZE)) {
                        /* Try to decode what we have */
                        break;
                    }
                    result = ENERSION_ERR_TIMEOUT;
                    break;
                }
                
                if (rs_err != RS485_OK) {
                    result = ENERSION_ERR_IO;
                    break;
                }
                
                total_read += bytes_read;
                
                /* Check if we have a complete packet */
                if ((total_read >= 5U) && (ctx->rx_buffer[0] == ENERSION_START_BYTE)) {
                    size_t expected = ENERSION_HEADER_SIZE +
                                     (size_t)ctx->rx_buffer[4] +
                                     ENERSION_CRC_SIZE + ENERSION_FOOTER_SIZE;
                    
                    if (total_read >= expected) {
                        rx_len = expected;
                        break;
                    }
                }
                
                /* Update timeout */
                if (timeout_remaining > 10U) {
                    timeout_remaining -= 10U;
                } else {
                    timeout_remaining = 0U;
                }
            }
            
            /* Decode response */
            if ((result == ENERSION_OK) && (rx_len > 0U)) {
                result = enersion_decode_packet(ctx->rx_buffer, rx_len, response);
            }
            
            if (result == ENERSION_OK) {
                break;  /* Success */
            }
            
            retries--;
            
        } while (retries > 0U);
    }
    
    return result;
}

enersion_error_t enersion_send(enersion_handle_t handle,
                                const enersion_packet_t *packet)
{
    enersion_error_t result = ENERSION_OK;
    struct enersion_ctx_tag *ctx = handle;
    
    if ((ctx == NULL) || (packet == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        size_t tx_len = 0U;
        result = enersion_encode_packet(packet, ctx->tx_buffer,
                                        sizeof(ctx->tx_buffer), &tx_len);
        
        if (result == ENERSION_OK) {
            size_t written = 0U;
            rs485_error_t rs_err = rs485_write(ctx->rs485, ctx->tx_buffer,
                                               tx_len, &written);
            
            if ((rs_err != RS485_OK) || (written != tx_len)) {
                result = ENERSION_ERR_IO;
            } else {
                (void)rs485_drain(ctx->rs485);
            }
        }
    }
    
    return result;
}

enersion_error_t enersion_ping(enersion_handle_t handle,
                                enersion_addr_t address,
                                bool *online)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((handle == NULL) || (online == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        enersion_packet_t request;
        enersion_packet_t response;
        
        request.dest_addr = (uint8_t)address;
        request.src_addr = handle->my_address;
        request.command = (uint8_t)ENERSION_CMD_PING;
        request.data_len = 0U;
        
        result = enersion_transaction(handle, &request, &response);
        
        *online = (result == ENERSION_OK) &&
                  (response.command == (uint8_t)ENERSION_CMD_PING_RESPONSE);
        
        if (*online) {
            result = ENERSION_OK;
        }
    }
    
    return result;
}

enersion_error_t enersion_get_version(enersion_handle_t handle,
                                       enersion_addr_t address,
                                       enersion_version_t *version)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((handle == NULL) || (version == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        enersion_packet_t request;
        enersion_packet_t response;
        
        request.dest_addr = (uint8_t)address;
        request.src_addr = handle->my_address;
        request.command = (uint8_t)ENERSION_CMD_GET_VERSION;
        request.data_len = 0U;
        
        result = enersion_transaction(handle, &request, &response);
        
        if ((result == ENERSION_OK) &&
            (response.command == (uint8_t)ENERSION_CMD_VERSION_RESPONSE) &&
            (response.data_len >= 5U)) {
            
            version->major = response.data[0];
            version->minor = response.data[1];
            version->patch = response.data[2];
            version->build = response.data[3];
            version->mcu_id = response.data[4];
        } else if (result == ENERSION_OK) {
            result = ENERSION_ERR_INVALID_PACKET;
        }
    }
    
    return result;
}

enersion_error_t enersion_heartbeat(enersion_handle_t handle,
                                     enersion_addr_t address,
                                     uint8_t *health)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((handle == NULL) || (health == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        enersion_packet_t request;
        enersion_packet_t response;
        
        request.dest_addr = (uint8_t)address;
        request.src_addr = handle->my_address;
        request.command = (uint8_t)ENERSION_CMD_HEARTBEAT;
        request.data_len = 0U;
        
        result = enersion_transaction(handle, &request, &response);
        
        if ((result == ENERSION_OK) &&
            (response.command == (uint8_t)ENERSION_CMD_HEARTBEAT_RESPONSE) &&
            (response.data_len >= 2U)) {
            
            *health = response.data[1];  /* Health is second byte */
        } else if (result == ENERSION_OK) {
            result = ENERSION_ERR_INVALID_PACKET;
        }
    }
    
    return result;
}

enersion_error_t enersion_get_status(enersion_handle_t handle,
                                      enersion_addr_t address,
                                      enersion_status_t *status)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((handle == NULL) || (status == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        enersion_packet_t request;
        enersion_packet_t response;
        
        request.dest_addr = (uint8_t)address;
        request.src_addr = handle->my_address;
        request.command = (uint8_t)ENERSION_CMD_GET_STATUS;
        request.data_len = 0U;
        
        result = enersion_transaction(handle, &request, &response);
        
        if ((result == ENERSION_OK) &&
            (response.command == (uint8_t)ENERSION_CMD_STATUS_RESPONSE) &&
            (response.data_len >= 16U)) {
            
            status->mcu_id = response.data[0];
            status->health = response.data[1];
            
            /* Parse uptime (little-endian) */
            status->uptime = (uint32_t)response.data[2] |
                            ((uint32_t)response.data[3] << 8U) |
                            ((uint32_t)response.data[4] << 16U) |
                            ((uint32_t)response.data[5] << 24U);
            
            /* Parse error_count */
            status->error_count = (uint32_t)response.data[6] |
                                 ((uint32_t)response.data[7] << 8U) |
                                 ((uint32_t)response.data[8] << 16U) |
                                 ((uint32_t)response.data[9] << 24U);
            
            /* Parse rx_packet_count */
            status->rx_packet_count = (uint32_t)response.data[10] |
                                     ((uint32_t)response.data[11] << 8U) |
                                     ((uint32_t)response.data[12] << 16U) |
                                     ((uint32_t)response.data[13] << 24U);
            
            /* Parse tx_packet_count */
            status->tx_packet_count = (uint16_t)response.data[14] |
                                     ((uint16_t)response.data[15] << 8U);
        } else if (result == ENERSION_OK) {
            result = ENERSION_ERR_INVALID_PACKET;
        }
    }
    
    return result;
}

enersion_error_t enersion_read_digital_inputs(enersion_handle_t handle,
                                               enersion_dio_state_t *state)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((handle == NULL) || (state == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        enersion_packet_t request;
        enersion_packet_t response;
        
        request.dest_addr = (uint8_t)ENERSION_ADDR_CTRL_DIO;
        request.src_addr = handle->my_address;
        request.command = (uint8_t)ENERSION_CMD_READ_DI;
        request.data_len = 0U;
        
        result = enersion_transaction(handle, &request, &response);
        
        if ((result == ENERSION_OK) &&
            (response.command == (uint8_t)ENERSION_CMD_DI_RESPONSE) &&
            (response.data_len >= ENERSION_DIO_BYTE_COUNT)) {
            
            (void)memcpy(state->state, response.data, ENERSION_DIO_BYTE_COUNT);
        } else if (result == ENERSION_OK) {
            result = ENERSION_ERR_INVALID_PACKET;
        }
    }
    
    return result;
}

enersion_error_t enersion_write_digital_outputs(enersion_handle_t handle,
                                                 const enersion_dio_state_t *state)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((handle == NULL) || (state == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        enersion_packet_t request;
        enersion_packet_t response;
        
        request.dest_addr = (uint8_t)ENERSION_ADDR_CTRL_OUT;
        request.src_addr = handle->my_address;
        request.command = (uint8_t)ENERSION_CMD_WRITE_DO;
        request.data_len = ENERSION_DIO_BYTE_COUNT;
        (void)memcpy(request.data, state->state, ENERSION_DIO_BYTE_COUNT);
        
        result = enersion_transaction(handle, &request, &response);
        
        if ((result == ENERSION_OK) &&
            (response.command != (uint8_t)ENERSION_CMD_DO_RESPONSE)) {
            result = ENERSION_ERR_DEVICE_ERROR;
        }
    }
    
    return result;
}

enersion_error_t enersion_read_digital_outputs(enersion_handle_t handle,
                                                enersion_dio_state_t *state)
{
    enersion_error_t result = ENERSION_OK;
    
    if ((handle == NULL) || (state == NULL)) {
        result = ENERSION_ERR_NULL_POINTER;
    }
    
    if (result == ENERSION_OK) {
        enersion_packet_t request;
        enersion_packet_t response;
        
        request.dest_addr = (uint8_t)ENERSION_ADDR_CTRL_OUT;
        request.src_addr = handle->my_address;
        request.command = (uint8_t)ENERSION_CMD_READ_DO;
        request.data_len = 0U;
        
        result = enersion_transaction(handle, &request, &response);
        
        if ((result == ENERSION_OK) &&
            (response.command == (uint8_t)ENERSION_CMD_DO_RESPONSE) &&
            (response.data_len >= ENERSION_DIO_BYTE_COUNT)) {
            
            (void)memcpy(state->state, response.data, ENERSION_DIO_BYTE_COUNT);
        } else if (result == ENERSION_OK) {
            result = ENERSION_ERR_INVALID_PACKET;
        }
    }
    
    return result;
}

const char *enersion_error_string(enersion_error_t error)
{
    const char *str;
    int32_t idx = -((int32_t)error);
    
    if ((idx >= 0) && ((size_t)idx < (sizeof(error_strings) / sizeof(error_strings[0])))) {
        str = error_strings[idx];
    } else {
        str = "Unknown error";
    }
    
    return str;
}

const char *enersion_command_string(enersion_cmd_t cmd)
{
    const char *str;
    
    if ((uint8_t)cmd < (sizeof(cmd_strings) / sizeof(cmd_strings[0]))) {
        str = cmd_strings[(uint8_t)cmd];
    } else {
        str = "UNKNOWN";
    }
    
    return str;
}

const char *enersion_device_name(enersion_addr_t addr)
{
    const char *str;
    
    if ((uint8_t)addr < (sizeof(device_names) / sizeof(device_names[0]))) {
        str = device_names[(uint8_t)addr];
    } else {
        str = "Unknown Device";
    }
    
    return str;
}



