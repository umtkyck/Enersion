/**
 * @file protocol_types.h
 * @brief Protocol Layer - Type Definitions
 * @version 1.0.0
 * @date 2024
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 * 
 * @note MISRA C:2012 Compliant
 * 
 * Defines the Enersion protocol structures for DI/DO communication
 * with STM32H7 controllers.
 */

#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Protocol Constants
 * ============================================================================ */

/** @brief Frame delimiters */
#define PROTOCOL_START_BYTE         0xAAU
#define PROTOCOL_END_BYTE           0x55U

/** @brief Frame size limits */
#define PROTOCOL_MAX_DATA_LENGTH    250U
#define PROTOCOL_MIN_FRAME_SIZE     8U      /* Start + Addr(2) + Cmd + Len + CRC(2) + End */
#define PROTOCOL_MAX_FRAME_SIZE     260U    /* Header + Data + Footer */
#define PROTOCOL_HEADER_SIZE        5U      /* Start + DestAddr + SrcAddr + Cmd + Len */
#define PROTOCOL_FOOTER_SIZE        3U      /* CRC(2) + End */

/** @brief Timeouts */
#define PROTOCOL_DEFAULT_TIMEOUT_MS 500U
#define PROTOCOL_RETRY_COUNT        3U

/** @brief Digital I/O limits */
#define PROTOCOL_DIO_CHANNEL_COUNT  64U
#define PROTOCOL_DIO_BYTE_COUNT     8U      /* 64 bits = 8 bytes */

/* ============================================================================
 * Device Addresses
 * ============================================================================ */

/**
 * @brief Device address enumeration
 * @note MISRA Rule 10.1 - Explicit enum values
 */
typedef enum {
    PROTOCOL_ADDR_BROADCAST     = 0x00U,    /**< Broadcast to all */
    PROTOCOL_ADDR_CTRL_420      = 0x01U,    /**< Controller 4-20mA (not used) */
    PROTOCOL_ADDR_CTRL_DI       = 0x02U,    /**< Controller Digital Input */
    PROTOCOL_ADDR_CTRL_DO       = 0x03U,    /**< Controller Digital Output */
    PROTOCOL_ADDR_MASTER        = 0x10U,    /**< Master (this device) */
} protocol_address_t;

/* ============================================================================
 * Command Codes
 * ============================================================================ */

/**
 * @brief Protocol command codes
 */
typedef enum {
    /* System Commands (0x01-0x0F) */
    PROTOCOL_CMD_PING               = 0x01U,
    PROTOCOL_CMD_PING_RESPONSE      = 0x02U,
    PROTOCOL_CMD_GET_VERSION        = 0x03U,
    PROTOCOL_CMD_VERSION_RESPONSE   = 0x04U,
    PROTOCOL_CMD_HEARTBEAT          = 0x05U,
    PROTOCOL_CMD_HEARTBEAT_RESPONSE = 0x06U,
    
    /* Status Commands (0x10-0x1F) */
    PROTOCOL_CMD_GET_STATUS         = 0x10U,
    PROTOCOL_CMD_STATUS_RESPONSE    = 0x11U,
    
    /* Digital Input Commands (0x20-0x2F) */
    PROTOCOL_CMD_READ_DI            = 0x20U,
    PROTOCOL_CMD_DI_RESPONSE        = 0x21U,
    
    /* Digital Output Commands (0x30-0x3F) */
    PROTOCOL_CMD_WRITE_DO           = 0x30U,
    PROTOCOL_CMD_DO_RESPONSE        = 0x31U,
    PROTOCOL_CMD_READ_DO            = 0x32U,
    
    /* Error (0xFF) */
    PROTOCOL_CMD_ERROR              = 0xFFU,
} protocol_command_t;

/* ============================================================================
 * Error Codes
 * ============================================================================ */

/**
 * @brief Protocol error codes
 */
typedef enum {
    PROTOCOL_ERR_NONE           = 0x00U,
    PROTOCOL_ERR_CRC            = 0x01U,
    PROTOCOL_ERR_INVALID_ADDR   = 0x02U,
    PROTOCOL_ERR_INVALID_CMD    = 0x03U,
    PROTOCOL_ERR_INVALID_LEN    = 0x04U,
    PROTOCOL_ERR_TIMEOUT        = 0x05U,
    PROTOCOL_ERR_BUSY           = 0x06U,
    PROTOCOL_ERR_PARAM          = 0x07U,
    PROTOCOL_ERR_FRAME          = 0x08U,
} protocol_error_t;

/* ============================================================================
 * Frame Structures
 * ============================================================================ */

/**
 * @brief Protocol frame structure
 * @note MISRA Rule 19.2 - No anonymous unions/structs
 */
typedef struct {
    uint8_t     dest_addr;                      /**< Destination address */
    uint8_t     src_addr;                       /**< Source address */
    uint8_t     command;                        /**< Command code */
    uint8_t     data[PROTOCOL_MAX_DATA_LENGTH]; /**< Payload data */
    uint8_t     data_length;                    /**< Payload length */
    uint16_t    crc;                            /**< CRC16 checksum */
} protocol_frame_t;

/**
 * @brief Raw frame buffer for encoding/decoding
 */
typedef struct {
    uint8_t     buffer[PROTOCOL_MAX_FRAME_SIZE];    /**< Raw frame data */
    size_t      length;                              /**< Frame length */
} protocol_raw_frame_t;

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Version information structure
 */
typedef struct {
    uint8_t     major;      /**< Major version */
    uint8_t     minor;      /**< Minor version */
    uint8_t     patch;      /**< Patch version */
    uint8_t     build;      /**< Build number */
    uint8_t     device_id;  /**< Device identifier */
} protocol_version_t;

/**
 * @brief Device status structure
 */
typedef struct {
    uint8_t     device_id;          /**< Device identifier */
    uint8_t     health;             /**< Health percentage (0-100) */
    uint32_t    uptime_sec;         /**< Uptime in seconds */
    uint32_t    error_count;        /**< Total error count */
    uint32_t    rx_packet_count;    /**< Received packet count */
    uint16_t    tx_packet_count;    /**< Transmitted packet count */
} protocol_status_t;

/**
 * @brief Digital I/O state (64 channels)
 * @note Bit 0 of byte 0 = Channel 0, Bit 7 of byte 7 = Channel 63
 */
typedef struct {
    uint8_t     channels[PROTOCOL_DIO_BYTE_COUNT];  /**< 64 bits packed */
} protocol_dio_state_t;

/* ============================================================================
 * Inline Helper Functions (MISRA Compliant)
 * ============================================================================ */

/**
 * @brief Get bit value from DIO state
 * @param state Pointer to DIO state
 * @param channel Channel number (0-63)
 * @return Bit value (true = HIGH, false = LOW)
 */
static inline bool protocol_dio_get_bit(const protocol_dio_state_t *state, 
                                        uint8_t channel)
{
    bool result = false;
    
    if ((state != NULL) && (channel < PROTOCOL_DIO_CHANNEL_COUNT)) {
        uint8_t byte_idx = channel / 8U;
        uint8_t bit_idx = channel % 8U;
        uint8_t mask = (uint8_t)(1U << bit_idx);
        
        result = ((state->channels[byte_idx] & mask) != 0U);
    }
    
    return result;
}

/**
 * @brief Set bit value in DIO state
 * @param state Pointer to DIO state
 * @param channel Channel number (0-63)
 * @param value Bit value (true = HIGH, false = LOW)
 */
static inline void protocol_dio_set_bit(protocol_dio_state_t *state,
                                        uint8_t channel,
                                        bool value)
{
    if ((state != NULL) && (channel < PROTOCOL_DIO_CHANNEL_COUNT)) {
        uint8_t byte_idx = channel / 8U;
        uint8_t bit_idx = channel % 8U;
        uint8_t mask = (uint8_t)(1U << bit_idx);
        
        if (value) {
            state->channels[byte_idx] |= mask;
        } else {
            state->channels[byte_idx] &= (uint8_t)(~mask);
        }
    }
}

/**
 * @brief Clear all bits in DIO state
 * @param state Pointer to DIO state
 */
static inline void protocol_dio_clear_all(protocol_dio_state_t *state)
{
    if (state != NULL) {
        for (uint8_t i = 0U; i < PROTOCOL_DIO_BYTE_COUNT; i++) {
            state->channels[i] = 0U;
        }
    }
}

/**
 * @brief Set all bits in DIO state
 * @param state Pointer to DIO state
 */
static inline void protocol_dio_set_all(protocol_dio_state_t *state)
{
    if (state != NULL) {
        for (uint8_t i = 0U; i < PROTOCOL_DIO_BYTE_COUNT; i++) {
            state->channels[i] = 0xFFU;
        }
    }
}

/**
 * @brief Copy DIO state
 * @param dest Destination state
 * @param src Source state
 */
static inline void protocol_dio_copy(protocol_dio_state_t *dest,
                                     const protocol_dio_state_t *src)
{
    if ((dest != NULL) && (src != NULL)) {
        for (uint8_t i = 0U; i < PROTOCOL_DIO_BYTE_COUNT; i++) {
            dest->channels[i] = src->channels[i];
        }
    }
}

/**
 * @brief Compare two DIO states
 * @param a First state
 * @param b Second state
 * @return true if equal, false otherwise
 */
static inline bool protocol_dio_equal(const protocol_dio_state_t *a,
                                      const protocol_dio_state_t *b)
{
    bool equal = true;
    
    if ((a == NULL) || (b == NULL)) {
        equal = false;
    } else {
        for (uint8_t i = 0U; i < PROTOCOL_DIO_BYTE_COUNT; i++) {
            if (a->channels[i] != b->channels[i]) {
                equal = false;
                break;
            }
        }
    }
    
    return equal;
}

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_TYPES_H */

