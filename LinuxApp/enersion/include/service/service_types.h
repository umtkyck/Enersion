/**
 * @file service_types.h
 * @brief Service Layer - Type Definitions
 * @version 1.0.0
 * @date 2024
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 * 
 * @note MISRA C:2012 Compliant
 */

#ifndef SERVICE_TYPES_H
#define SERVICE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "protocol/protocol_types.h"

/* ============================================================================
 * Constants
 * ============================================================================ */

#define SERVICE_HEARTBEAT_INTERVAL_MS   2000U   /**< Heartbeat every 2 seconds */
#define SERVICE_POLL_INTERVAL_MS        100U    /**< Poll interval for DI */
#define SERVICE_MAX_DEVICES             4U      /**< Maximum devices on bus */

/* ============================================================================
 * Device State
 * ============================================================================ */

/**
 * @brief Device connection state
 */
typedef enum {
    DEVICE_STATE_DISCONNECTED   = 0,    /**< Not connected */
    DEVICE_STATE_CONNECTING     = 1,    /**< Connection in progress */
    DEVICE_STATE_CONNECTED      = 2,    /**< Connected and healthy */
    DEVICE_STATE_ERROR          = 3,    /**< Communication error */
    DEVICE_STATE_TIMEOUT        = 4,    /**< Response timeout */
} device_state_t;

/**
 * @brief Device type
 */
typedef enum {
    DEVICE_TYPE_UNKNOWN = 0,
    DEVICE_TYPE_DI      = 1,    /**< Digital Input controller */
    DEVICE_TYPE_DO      = 2,    /**< Digital Output controller */
} device_type_t;

/* ============================================================================
 * Device Information
 * ============================================================================ */

/**
 * @brief Device information structure
 */
typedef struct {
    protocol_address_t  address;        /**< Device address */
    device_type_t       type;           /**< Device type */
    device_state_t      state;          /**< Connection state */
    protocol_version_t  version;        /**< Firmware version */
    protocol_status_t   status;         /**< Device status */
    uint32_t            last_seen_ms;   /**< Last response timestamp */
    uint32_t            error_count;    /**< Communication errors */
    bool                online;         /**< Device is responding */
} service_device_info_t;

/* ============================================================================
 * DIO Service Types
 * ============================================================================ */

/**
 * @brief Digital Input service data
 */
typedef struct {
    protocol_dio_state_t    current_state;      /**< Current input state */
    protocol_dio_state_t    previous_state;     /**< Previous state (for change detection) */
    uint32_t                last_update_ms;     /**< Last update timestamp */
    bool                    valid;              /**< Data is valid */
} service_di_data_t;

/**
 * @brief Digital Output service data
 */
typedef struct {
    protocol_dio_state_t    current_state;      /**< Current output state */
    protocol_dio_state_t    pending_state;      /**< Pending state to write */
    uint32_t                last_update_ms;     /**< Last update timestamp */
    bool                    valid;              /**< Data is valid */
    bool                    pending_write;      /**< Write pending flag */
} service_do_data_t;

/* ============================================================================
 * Callback Types
 * ============================================================================ */

/**
 * @brief Connection state change callback
 * @param device Device address
 * @param state New connection state
 * @param user_data User context
 */
typedef void (*service_state_callback_t)(protocol_address_t device,
                                         device_state_t state,
                                         void *user_data);

/**
 * @brief Digital Input change callback
 * @param old_state Previous state
 * @param new_state Current state
 * @param user_data User context
 */
typedef void (*service_di_callback_t)(const protocol_dio_state_t *old_state,
                                      const protocol_dio_state_t *new_state,
                                      void *user_data);

/**
 * @brief Digital Output confirm callback
 * @param state Confirmed state
 * @param success true if write successful
 * @param user_data User context
 */
typedef void (*service_do_callback_t)(const protocol_dio_state_t *state,
                                      bool success,
                                      void *user_data);

/**
 * @brief Error callback
 * @param device Device address
 * @param error Error description
 * @param user_data User context
 */
typedef void (*service_error_callback_t)(protocol_address_t device,
                                         const char *error,
                                         void *user_data);

/* ============================================================================
 * Service Configuration
 * ============================================================================ */

/**
 * @brief Service layer configuration
 */
typedef struct {
    const char      *serial_port;       /**< Serial port path */
    uint32_t        baudrate;           /**< Baud rate */
    uint32_t        timeout_ms;         /**< Response timeout */
    uint32_t        heartbeat_ms;       /**< Heartbeat interval */
    uint32_t        poll_interval_ms;   /**< DI poll interval */
    bool            auto_reconnect;     /**< Auto reconnect on error */
    bool            debug;              /**< Enable debug logging */
} service_config_t;

/**
 * @brief Service statistics
 */
typedef struct {
    uint32_t    tx_count;           /**< Frames transmitted */
    uint32_t    rx_count;           /**< Frames received */
    uint32_t    crc_errors;         /**< CRC errors */
    uint32_t    timeout_errors;     /**< Timeout errors */
    uint32_t    retry_count;        /**< Total retries */
    uint32_t    uptime_sec;         /**< Service uptime */
} service_stats_t;

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_TYPES_H */

