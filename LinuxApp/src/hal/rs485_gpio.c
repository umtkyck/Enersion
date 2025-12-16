/**
 * @file rs485_gpio.c
 * @brief RS485 GPIO Direction Control Implementation - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "rs485_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* ============================================================================
 * Private Variables
 * ============================================================================ */

static bool g_gpio_initialized = false;

/* ============================================================================
 * Private Functions
 * ============================================================================ */

/**
 * @brief Write string to sysfs file
 */
static rs485_gpio_error_t write_sysfs(const char *path, const char *value)
{
    rs485_gpio_error_t result = RS485_GPIO_OK;
    int fd;
    
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        result = RS485_GPIO_ERR_ACCESS;
    } else {
        ssize_t len = (ssize_t)strlen(value);
        ssize_t written = write(fd, value, (size_t)len);
        
        if (written != len) {
            result = RS485_GPIO_ERR_VALUE;
        }
        
        (void)close(fd);
    }
    
    return result;
}

/**
 * @brief Read string from sysfs file
 */
static rs485_gpio_error_t read_sysfs(const char *path, char *buffer, size_t size)
{
    rs485_gpio_error_t result = RS485_GPIO_OK;
    int fd;
    
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        result = RS485_GPIO_ERR_ACCESS;
    } else {
        ssize_t bytes_read = read(fd, buffer, size - 1U);
        
        if (bytes_read < 0) {
            result = RS485_GPIO_ERR_VALUE;
            buffer[0] = '\0';
        } else {
            buffer[bytes_read] = '\0';
            /* Remove trailing newline */
            if ((bytes_read > 0) && (buffer[bytes_read - 1] == '\n')) {
                buffer[bytes_read - 1] = '\0';
            }
        }
        
        (void)close(fd);
    }
    
    return result;
}

/**
 * @brief Get the correct GPIO path (some systems use gpio138, others use PI10)
 */
static const char *get_gpio_path(void)
{
    if (access(RS485_GPIO_PATH "/value", F_OK) == 0) {
        return RS485_GPIO_PATH;
    } else if (access(RS485_GPIO_PATH_ALT "/value", F_OK) == 0) {
        return RS485_GPIO_PATH_ALT;
    }
    return RS485_GPIO_PATH;  /* Default */
}

/**
 * @brief Check if GPIO is already exported
 */
static bool is_gpio_exported(void)
{
    return (access(RS485_GPIO_PATH "/value", F_OK) == 0) ||
           (access(RS485_GPIO_PATH_ALT "/value", F_OK) == 0);
}

/* ============================================================================
 * Public Functions
 * ============================================================================ */

rs485_gpio_error_t rs485_gpio_init(void)
{
    rs485_gpio_error_t result = RS485_GPIO_OK;
    char gpio_num_str[8];
    
    if (g_gpio_initialized) {
        /* Already initialized */
        return RS485_GPIO_OK;
    }
    
    /* Check if already exported */
    if (!is_gpio_exported()) {
        /* Export GPIO */
        (void)snprintf(gpio_num_str, sizeof(gpio_num_str), "%u", RS485_GPIO_NUMBER);
        result = write_sysfs("/sys/class/gpio/export", gpio_num_str);
        
        if (result != RS485_GPIO_OK) {
            /* May already be exported by another process */
            if (is_gpio_exported()) {
                result = RS485_GPIO_OK;
            } else {
                return RS485_GPIO_ERR_EXPORT;
            }
        }
        
        /* Wait for sysfs to create the files */
        usleep(100000);  /* 100ms */
    }
    
    /* Set direction to output */
    if (result == RS485_GPIO_OK) {
        char path[64];
        (void)snprintf(path, sizeof(path), "%s/direction", get_gpio_path());
        result = write_sysfs(path, "out");
        if (result != RS485_GPIO_OK) {
            result = RS485_GPIO_ERR_DIRECTION;
        }
    }
    
    /* Default to receive mode */
    if (result == RS485_GPIO_OK) {
        result = rs485_gpio_rx_enable();
    }
    
    if (result == RS485_GPIO_OK) {
        g_gpio_initialized = true;
    }
    
    return result;
}

rs485_gpio_error_t rs485_gpio_deinit(void)
{
    rs485_gpio_error_t result = RS485_GPIO_OK;
    char gpio_num_str[8];
    
    if (!g_gpio_initialized) {
        return RS485_GPIO_OK;
    }
    
    /* Set to receive mode before unexport */
    (void)rs485_gpio_rx_enable();
    
    /* Unexport GPIO */
    (void)snprintf(gpio_num_str, sizeof(gpio_num_str), "%u", RS485_GPIO_NUMBER);
    result = write_sysfs("/sys/class/gpio/unexport", gpio_num_str);
    
    g_gpio_initialized = false;
    
    return result;
}

rs485_gpio_error_t rs485_gpio_rx_enable(void)
{
    rs485_gpio_error_t result;
    char path[64];
    
    /* Set GPIO to LOW for receive mode */
    (void)snprintf(path, sizeof(path), "%s/value", get_gpio_path());
    result = write_sysfs(path, "0");
    
    if (result != RS485_GPIO_OK) {
        result = RS485_GPIO_ERR_VALUE;
    }
    
    return result;
}

rs485_gpio_error_t rs485_gpio_tx_enable(void)
{
    rs485_gpio_error_t result;
    char path[64];
    
    /* Set GPIO to HIGH for transmit mode */
    (void)snprintf(path, sizeof(path), "%s/value", get_gpio_path());
    result = write_sysfs(path, "1");
    
    if (result != RS485_GPIO_OK) {
        result = RS485_GPIO_ERR_VALUE;
    }
    
    return result;
}

bool rs485_gpio_is_initialized(void)
{
    return g_gpio_initialized;
}

rs485_gpio_error_t rs485_gpio_get_direction(bool *is_tx)
{
    rs485_gpio_error_t result;
    char value_str[4];
    char path[64];
    
    if (is_tx == NULL) {
        return RS485_GPIO_ERR_VALUE;
    }
    
    (void)snprintf(path, sizeof(path), "%s/value", get_gpio_path());
    result = read_sysfs(path, value_str, sizeof(value_str));
    
    if (result == RS485_GPIO_OK) {
        *is_tx = (value_str[0] == '1');
    }
    
    return result;
}



