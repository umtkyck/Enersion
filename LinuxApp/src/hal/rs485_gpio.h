/**
 * @file rs485_gpio.h
 * @brief RS485 GPIO Direction Control - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * MYIR STM32MP257 RS485 uses GPIO PI10 for direction control:
 * - GPIO 138 = PI10 (nRTS)
 * - LOW (0) = Receive mode
 * - HIGH (1) = Transmit mode
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef RS485_GPIO_H
#define RS485_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * MYIR STM32MP257 RS485 Configuration
 * ============================================================================ */

/** RS485 UART device node */
#define RS485_DEVICE_NODE       "/dev/ttySTM9"

/** RS485 direction GPIO (PI10 = 138) */
#define RS485_GPIO_NUMBER       (138U)
#define RS485_GPIO_NAME         "PI10"
#define RS485_GPIO_PATH         "/sys/class/gpio/gpio138"
#define RS485_GPIO_PATH_ALT     "/sys/class/gpio/PI10"

/** Direction control values */
#define RS485_DIR_RECEIVE       (0U)    /**< GPIO LOW = RX mode */
#define RS485_DIR_TRANSMIT      (1U)    /**< GPIO HIGH = TX mode */

/* ============================================================================
 * Error Codes
 * ============================================================================ */

typedef enum rs485_gpio_error_tag {
    RS485_GPIO_OK               = 0,
    RS485_GPIO_ERR_EXPORT       = -1,
    RS485_GPIO_ERR_DIRECTION    = -2,
    RS485_GPIO_ERR_VALUE        = -3,
    RS485_GPIO_ERR_ACCESS       = -4
} rs485_gpio_error_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize RS485 GPIO direction control
 * 
 * Exports GPIO 138 (PI10) and sets it as output.
 * Must be called before using RS485.
 * 
 * @return rs485_gpio_error_t RS485_GPIO_OK on success
 */
rs485_gpio_error_t rs485_gpio_init(void);

/**
 * @brief Deinitialize RS485 GPIO
 * 
 * Unexports the GPIO pin.
 * 
 * @return rs485_gpio_error_t RS485_GPIO_OK on success
 */
rs485_gpio_error_t rs485_gpio_deinit(void);

/**
 * @brief Set RS485 to receive mode
 * 
 * Sets GPIO PI10 to LOW (0).
 * 
 * @return rs485_gpio_error_t RS485_GPIO_OK on success
 */
rs485_gpio_error_t rs485_gpio_rx_enable(void);

/**
 * @brief Set RS485 to transmit mode
 * 
 * Sets GPIO PI10 to HIGH (1).
 * 
 * @return rs485_gpio_error_t RS485_GPIO_OK on success
 */
rs485_gpio_error_t rs485_gpio_tx_enable(void);

/**
 * @brief Check if GPIO is initialized
 * 
 * @return bool true if initialized
 */
bool rs485_gpio_is_initialized(void);

/**
 * @brief Get current direction mode
 * 
 * @param[out] is_tx Output: true if in TX mode, false if RX mode
 * @return rs485_gpio_error_t RS485_GPIO_OK on success
 */
rs485_gpio_error_t rs485_gpio_get_direction(bool *is_tx);

#ifdef __cplusplus
}
#endif

#endif /* RS485_GPIO_H */



