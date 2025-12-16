/**
 * @file enersion_types.h
 * @brief Enersion Protocol Type Definitions - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef ENERSION_TYPES_H
#define ENERSION_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Protocol Constants - MISRA C:2012 Rule 7.2
 * ============================================================================ */

#define ENERSION_START_BYTE         (0xAAU)
#define ENERSION_END_BYTE           (0x55U)
#define ENERSION_MAX_DATA_LEN       (250U)
#define ENERSION_MAX_PACKET_SIZE    (258U)  /* Start + Header + Data + CRC + End */
#define ENERSION_HEADER_SIZE        (5U)    /* Start + Dest + Src + Cmd + Len */
#define ENERSION_CRC_SIZE           (2U)
#define ENERSION_FOOTER_SIZE        (1U)    /* End byte */

#define ENERSION_DIO_CHANNEL_COUNT  (64U)
#define ENERSION_DIO_BYTE_COUNT     (8U)    /* 64 bits = 8 bytes */

/* ============================================================================
 * Device Addresses - MISRA C:2012 Rule 8.9
 * ============================================================================ */

typedef enum enersion_addr_tag {
    ENERSION_ADDR_BROADCAST     = 0x00U,
    ENERSION_ADDR_CTRL_420      = 0x01U,    /**< Controller 4-20mA */
    ENERSION_ADDR_CTRL_DIO      = 0x02U,    /**< Controller Digital Input */
    ENERSION_ADDR_CTRL_OUT      = 0x03U,    /**< Controller Digital Output */
    ENERSION_ADDR_MASTER        = 0x10U     /**< Master (Linux/GUI) */
} enersion_addr_t;

/* ============================================================================
 * Command Codes
 * ============================================================================ */

typedef enum enersion_cmd_tag {
    /* System Commands */
    ENERSION_CMD_PING               = 0x01U,
    ENERSION_CMD_PING_RESPONSE      = 0x02U,
    ENERSION_CMD_GET_VERSION        = 0x03U,
    ENERSION_CMD_VERSION_RESPONSE   = 0x04U,
    ENERSION_CMD_HEARTBEAT          = 0x05U,
    ENERSION_CMD_HEARTBEAT_RESPONSE = 0x06U,
    
    /* Status Commands */
    ENERSION_CMD_GET_STATUS         = 0x10U,
    ENERSION_CMD_STATUS_RESPONSE    = 0x11U,
    
    /* Digital Input Commands */
    ENERSION_CMD_READ_DI            = 0x20U,
    ENERSION_CMD_DI_RESPONSE        = 0x21U,
    
    /* Digital Output Commands */
    ENERSION_CMD_WRITE_DO           = 0x30U,
    ENERSION_CMD_DO_RESPONSE        = 0x31U,
    ENERSION_CMD_READ_DO            = 0x32U,
    
    /* Error Response */
    ENERSION_CMD_ERROR_RESPONSE     = 0xFFU
} enersion_cmd_t;

/* ============================================================================
 * Error Codes
 * ============================================================================ */

typedef enum enersion_error_tag {
    ENERSION_OK                     = 0,
    ENERSION_ERR_NULL_POINTER       = -1,
    ENERSION_ERR_INVALID_PARAM      = -2,
    ENERSION_ERR_INVALID_CRC        = -3,
    ENERSION_ERR_INVALID_PACKET     = -4,
    ENERSION_ERR_TIMEOUT            = -5,
    ENERSION_ERR_IO                 = -6,
    ENERSION_ERR_BUFFER_OVERFLOW    = -7,
    ENERSION_ERR_NOT_CONNECTED      = -8,
    ENERSION_ERR_DEVICE_ERROR       = -9
} enersion_error_t;

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Protocol Packet Structure
 */
typedef struct enersion_packet_tag {
    uint8_t dest_addr;                      /**< Destination address */
    uint8_t src_addr;                       /**< Source address */
    uint8_t command;                        /**< Command code */
    uint8_t data_len;                       /**< Data length */
    uint8_t data[ENERSION_MAX_DATA_LEN];    /**< Data payload */
} enersion_packet_t;

/**
 * @brief Version Information
 */
typedef struct enersion_version_tag {
    uint8_t major;                          /**< Major version */
    uint8_t minor;                          /**< Minor version */
    uint8_t patch;                          /**< Patch version */
    uint8_t build;                          /**< Build number */
    uint8_t mcu_id;                         /**< MCU identifier */
} enersion_version_t;

/**
 * @brief Device Status
 */
typedef struct enersion_status_tag {
    uint8_t  mcu_id;                        /**< MCU identifier */
    uint8_t  health;                        /**< Health 0-100% */
    uint32_t uptime;                        /**< Uptime in seconds */
    uint32_t error_count;                   /**< Error count */
    uint32_t rx_packet_count;               /**< RX packet count */
    uint16_t tx_packet_count;               /**< TX packet count */
} enersion_status_t;

/**
 * @brief Digital I/O State (64 channels)
 */
typedef struct enersion_dio_state_tag {
    uint8_t state[ENERSION_DIO_BYTE_COUNT]; /**< 64 bits = 8 bytes */
} enersion_dio_state_t;

/**
 * @brief Protocol Handle (Opaque)
 */
typedef struct enersion_ctx_tag *enersion_handle_t;

/* ============================================================================
 * Inline Helper Functions - MISRA C:2012 Rule 8.5
 * ============================================================================ */

/**
 * @brief Get bit value from DIO state
 */
static inline bool enersion_dio_get_bit(const enersion_dio_state_t *state, uint8_t bit)
{
    bool value = false;
    
    if ((state != NULL) && (bit < ENERSION_DIO_CHANNEL_COUNT)) {
        uint8_t byte_idx = bit / 8U;
        uint8_t bit_idx = bit % 8U;
        value = ((state->state[byte_idx] & (1U << bit_idx)) != 0U);
    }
    
    return value;
}

/**
 * @brief Set bit value in DIO state
 */
static inline void enersion_dio_set_bit(enersion_dio_state_t *state, uint8_t bit, bool value)
{
    if ((state != NULL) && (bit < ENERSION_DIO_CHANNEL_COUNT)) {
        uint8_t byte_idx = bit / 8U;
        uint8_t bit_idx = bit % 8U;
        
        if (value) {
            state->state[byte_idx] |= (uint8_t)(1U << bit_idx);
        } else {
            state->state[byte_idx] &= (uint8_t)(~(1U << bit_idx));
        }
    }
}

/**
 * @brief Clear all bits in DIO state
 */
static inline void enersion_dio_clear_all(enersion_dio_state_t *state)
{
    if (state != NULL) {
        for (uint8_t i = 0U; i < ENERSION_DIO_BYTE_COUNT; i++) {
            state->state[i] = 0x00U;
        }
    }
}

/**
 * @brief Set all bits in DIO state
 */
static inline void enersion_dio_set_all(enersion_dio_state_t *state)
{
    if (state != NULL) {
        for (uint8_t i = 0U; i < ENERSION_DIO_BYTE_COUNT; i++) {
            state->state[i] = 0xFFU;
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif /* ENERSION_TYPES_H */



