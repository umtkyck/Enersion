/**
 * @file rs485_serial.c
 * @brief RS485 Serial Port Implementation - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

/* MISRA C:2012 Rule 21.6: Standard library input/output functions */
#define _GNU_SOURCE

#include "rs485_serial.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/serial.h>

/* ============================================================================
 * Internal Structure - MISRA C:2012 Rule 8.4: Compatible declarations
 * ============================================================================ */

struct rs485_handle_tag {
    int32_t         fd;                         /**< File descriptor */
    char            device[128];                /**< Device path */
    rs485_config_t  config;                     /**< Configuration */
    rs485_stats_t   stats;                      /**< Statistics */
    struct termios  original_termios;           /**< Original settings */
    bool            termios_saved;              /**< Settings saved flag */
    bool            is_open;                    /**< Port open flag */
};

/* ============================================================================
 * Static Functions - MISRA C:2012 Rule 8.8: Static linkage
 * ============================================================================ */

/**
 * @brief Convert baudrate to speed_t
 * MISRA C:2012 Rule 16.4: Switch statement shall have default
 */
static speed_t baudrate_to_speed(uint32_t baudrate)
{
    speed_t speed;
    
    switch (baudrate) {
        case 9600U:    speed = B9600;    break;
        case 19200U:   speed = B19200;   break;
        case 38400U:   speed = B38400;   break;
        case 57600U:   speed = B57600;   break;
        case 115200U:  speed = B115200;  break;
        case 230400U:  speed = B230400;  break;
        case 460800U:  speed = B460800;  break;
        case 921600U:  speed = B921600;  break;
        default:       speed = B115200;  break;
    }
    
    return speed;
}

/**
 * @brief Configure serial port
 * MISRA C:2012 Rule 17.7: Return value shall be used
 */
static rs485_error_t configure_port(struct rs485_handle_tag *ctx)
{
    rs485_error_t result = RS485_OK;
    struct termios tty;
    int32_t ret;
    
    /* MISRA C:2012 Rule 21.15: memset on struct */
    (void)memset(&tty, 0, sizeof(tty));
    
    ret = tcgetattr(ctx->fd, &tty);
    if (ret != 0) {
        result = RS485_ERR_CONFIG_FAILED;
    }
    
    if (result == RS485_OK) {
        /* Set baud rate */
        speed_t speed = baudrate_to_speed(ctx->config.baudrate);
        (void)cfsetispeed(&tty, speed);
        (void)cfsetospeed(&tty, speed);
        
        /* Control modes */
        tty.c_cflag |= (tcflag_t)(CLOCAL | CREAD);
        
        /* Data bits - MISRA C:2012 Rule 10.3: Type conversion */
        tty.c_cflag &= ~(tcflag_t)CSIZE;
        if (ctx->config.data_bits == 7U) {
            tty.c_cflag |= (tcflag_t)CS7;
        } else {
            tty.c_cflag |= (tcflag_t)CS8;
        }
        
        /* Parity */
        switch (ctx->config.parity) {
            case RS485_PARITY_EVEN:
                tty.c_cflag |= (tcflag_t)PARENB;
                tty.c_cflag &= ~(tcflag_t)PARODD;
                break;
            case RS485_PARITY_ODD:
                tty.c_cflag |= (tcflag_t)(PARENB | PARODD);
                break;
            case RS485_PARITY_NONE:
            default:
                tty.c_cflag &= ~(tcflag_t)PARENB;
                break;
        }
        
        /* Stop bits */
        if (ctx->config.stop_bits == 2U) {
            tty.c_cflag |= (tcflag_t)CSTOPB;
        } else {
            tty.c_cflag &= ~(tcflag_t)CSTOPB;
        }
        
        /* No hardware flow control */
        tty.c_cflag &= ~(tcflag_t)CRTSCTS;
        
        /* Input modes - raw input */
        tty.c_iflag &= ~(tcflag_t)(IGNBRK | BRKINT | PARMRK | ISTRIP |
                                    INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY);
        
        /* Output modes - raw output */
        tty.c_oflag &= ~(tcflag_t)OPOST;
        
        /* Local modes - raw mode */
        tty.c_lflag &= ~(tcflag_t)(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        
        /* Control characters */
        tty.c_cc[VMIN] = 0;
        tty.c_cc[VTIME] = 1;  /* 100ms timeout */
        
        /* Apply settings */
        ret = tcsetattr(ctx->fd, TCSANOW, &tty);
        if (ret != 0) {
            result = RS485_ERR_CONFIG_FAILED;
        }
    }
    
    /* Configure RS485 mode if enabled */
    if ((result == RS485_OK) && ctx->config.rs485_mode) {
#ifdef TIOCGRS485
        struct serial_rs485 rs485conf;
        
        ret = ioctl(ctx->fd, TIOCGRS485, &rs485conf);
        if (ret >= 0) {
            rs485conf.flags |= SER_RS485_ENABLED;
            rs485conf.flags |= SER_RS485_RTS_ON_SEND;
            rs485conf.flags &= ~(uint32_t)SER_RS485_RTS_AFTER_SEND;
            rs485conf.delay_rts_before_send = 0;
            rs485conf.delay_rts_after_send = 0;
            
            (void)ioctl(ctx->fd, TIOCSRS485, &rs485conf);
        }
#endif
    }
    
    return result;
}

/* ============================================================================
 * Error String Table - MISRA C:2012 Rule 8.9: Local scope
 * ============================================================================ */

static const char * const error_strings[] = {
    "Success",
    "Null pointer",
    "Invalid parameter",
    "Open failed",
    "Configuration failed",
    "Write failed",
    "Read failed",
    "Timeout",
    "Port not open",
    "Buffer overflow"
};

#define ERROR_STRING_COUNT (sizeof(error_strings) / sizeof(error_strings[0]))

/* ============================================================================
 * Public Functions
 * ============================================================================ */

rs485_error_t rs485_create(const rs485_config_t *config, rs485_handle_t *handle)
{
    rs485_error_t result = RS485_OK;
    struct rs485_handle_tag *ctx = NULL;
    
    /* MISRA C:2012 Rule 14.3: Controlling expression */
    if ((config == NULL) || (handle == NULL)) {
        result = RS485_ERR_NULL_POINTER;
    } else if (config->device == NULL) {
        result = RS485_ERR_INVALID_PARAM;
    } else {
        /* Allocate context */
        ctx = (struct rs485_handle_tag *)calloc(1U, sizeof(struct rs485_handle_tag));
        if (ctx == NULL) {
            result = RS485_ERR_INVALID_PARAM;
        }
    }
    
    if ((result == RS485_OK) && (ctx != NULL)) {
        /* Initialize context */
        ctx->fd = -1;
        ctx->is_open = false;
        ctx->termios_saved = false;
        
        /* Copy device path */
        size_t len = strlen(config->device);
        if (len >= sizeof(ctx->device)) {
            len = sizeof(ctx->device) - 1U;
        }
        (void)strncpy(ctx->device, config->device, len);
        ctx->device[len] = '\0';
        
        /* Copy configuration with defaults */
        ctx->config = *config;
        ctx->config.device = ctx->device;
        
        if (ctx->config.baudrate == 0U) {
            ctx->config.baudrate = RS485_DEFAULT_BAUDRATE;
        }
        if (ctx->config.data_bits == 0U) {
            ctx->config.data_bits = 8U;
        }
        if (ctx->config.stop_bits == 0U) {
            ctx->config.stop_bits = 1U;
        }
        if (ctx->config.timeout_ms == 0U) {
            ctx->config.timeout_ms = RS485_DEFAULT_TIMEOUT_MS;
        }
        
        /* Clear statistics */
        (void)memset(&ctx->stats, 0, sizeof(ctx->stats));
        
        *handle = ctx;
    }
    
    return result;
}

rs485_error_t rs485_destroy(rs485_handle_t *handle)
{
    rs485_error_t result = RS485_OK;
    
    if (handle == NULL) {
        result = RS485_ERR_NULL_POINTER;
    } else if (*handle != NULL) {
        struct rs485_handle_tag *ctx = *handle;
        
        /* Close port if open */
        if (ctx->is_open) {
            (void)rs485_close(ctx);
        }
        
        /* Free context */
        free(ctx);
        *handle = NULL;
    } else {
        /* Handle is already NULL - no action needed */
    }
    
    return result;
}

rs485_error_t rs485_open(rs485_handle_t handle)
{
    rs485_error_t result = RS485_OK;
    struct rs485_handle_tag *ctx = handle;
    
    if (ctx == NULL) {
        result = RS485_ERR_NULL_POINTER;
    } else if (ctx->is_open) {
        /* Already open */
        result = RS485_OK;
    } else {
        /* Open serial port */
        ctx->fd = open(ctx->device, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (ctx->fd < 0) {
            result = RS485_ERR_OPEN_FAILED;
        }
    }
    
    if (result == RS485_OK) {
        /* Save original termios */
        if (tcgetattr(ctx->fd, &ctx->original_termios) == 0) {
            ctx->termios_saved = true;
        }
        
        /* Configure port */
        result = configure_port(ctx);
        
        if (result == RS485_OK) {
            /* Flush buffers */
            (void)tcflush(ctx->fd, TCIOFLUSH);
            ctx->is_open = true;
        } else {
            (void)close(ctx->fd);
            ctx->fd = -1;
        }
    }
    
    return result;
}

rs485_error_t rs485_close(rs485_handle_t handle)
{
    rs485_error_t result = RS485_OK;
    struct rs485_handle_tag *ctx = handle;
    
    if (ctx == NULL) {
        result = RS485_ERR_NULL_POINTER;
    } else if (!ctx->is_open) {
        result = RS485_ERR_NOT_OPEN;
    } else {
        /* Restore original termios */
        if (ctx->termios_saved) {
            (void)tcsetattr(ctx->fd, TCSANOW, &ctx->original_termios);
        }
        
        (void)close(ctx->fd);
        ctx->fd = -1;
        ctx->is_open = false;
    }
    
    return result;
}

rs485_error_t rs485_is_open(rs485_handle_t handle, bool *is_open)
{
    rs485_error_t result = RS485_OK;
    
    if ((handle == NULL) || (is_open == NULL)) {
        result = RS485_ERR_NULL_POINTER;
    } else {
        *is_open = handle->is_open;
    }
    
    return result;
}

rs485_error_t rs485_write(rs485_handle_t handle,
                          const uint8_t *data,
                          size_t length,
                          size_t *written)
{
    rs485_error_t result = RS485_OK;
    struct rs485_handle_tag *ctx = handle;
    
    if ((ctx == NULL) || (data == NULL) || (written == NULL)) {
        result = RS485_ERR_NULL_POINTER;
    } else if (!ctx->is_open) {
        result = RS485_ERR_NOT_OPEN;
    } else if (length == 0U) {
        *written = 0U;
    } else {
        /* Write data */
        ssize_t total = 0;
        size_t remaining = length;
        const uint8_t *ptr = data;
        
        while (remaining > 0U) {
            ssize_t n = write(ctx->fd, ptr, remaining);
            
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                result = RS485_ERR_WRITE_FAILED;
                ctx->stats.error_count++;
                break;
            }
            
            total += n;
            ptr += (size_t)n;
            remaining -= (size_t)n;
        }
        
        if (result == RS485_OK) {
            *written = (size_t)total;
            ctx->stats.bytes_sent += (uint64_t)total;
            ctx->stats.tx_count++;
        }
    }
    
    return result;
}

rs485_error_t rs485_read(rs485_handle_t handle,
                         uint8_t *buffer,
                         size_t buffer_size,
                         uint32_t timeout_ms,
                         size_t *bytes_read)
{
    rs485_error_t result = RS485_OK;
    struct rs485_handle_tag *ctx = handle;
    
    if ((ctx == NULL) || (buffer == NULL) || (bytes_read == NULL)) {
        result = RS485_ERR_NULL_POINTER;
    } else if (!ctx->is_open) {
        result = RS485_ERR_NOT_OPEN;
    } else if (buffer_size == 0U) {
        *bytes_read = 0U;
    } else {
        /* Use select for timeout */
        fd_set readfds;
        struct timeval tv;
        
        FD_ZERO(&readfds);
        FD_SET(ctx->fd, &readfds);
        
        tv.tv_sec = (long)(timeout_ms / 1000U);
        tv.tv_usec = (long)((timeout_ms % 1000U) * 1000U);
        
        int32_t ret = select(ctx->fd + 1, &readfds, NULL, NULL, &tv);
        
        if (ret < 0) {
            result = RS485_ERR_READ_FAILED;
            ctx->stats.error_count++;
        } else if (ret == 0) {
            /* Timeout */
            *bytes_read = 0U;
            ctx->stats.timeout_count++;
            result = RS485_ERR_TIMEOUT;
        } else {
            /* Data available */
            ssize_t n = read(ctx->fd, buffer, buffer_size);
            
            if (n < 0) {
                result = RS485_ERR_READ_FAILED;
                ctx->stats.error_count++;
                *bytes_read = 0U;
            } else {
                *bytes_read = (size_t)n;
                ctx->stats.bytes_received += (uint64_t)n;
                ctx->stats.rx_count++;
            }
        }
    }
    
    return result;
}

rs485_error_t rs485_flush(rs485_handle_t handle)
{
    rs485_error_t result = RS485_OK;
    
    if (handle == NULL) {
        result = RS485_ERR_NULL_POINTER;
    } else if (!handle->is_open) {
        result = RS485_ERR_NOT_OPEN;
    } else {
        if (tcflush(handle->fd, TCIOFLUSH) != 0) {
            result = RS485_ERR_CONFIG_FAILED;
        }
    }
    
    return result;
}

rs485_error_t rs485_drain(rs485_handle_t handle)
{
    rs485_error_t result = RS485_OK;
    
    if (handle == NULL) {
        result = RS485_ERR_NULL_POINTER;
    } else if (!handle->is_open) {
        result = RS485_ERR_NOT_OPEN;
    } else {
        if (tcdrain(handle->fd) != 0) {
            result = RS485_ERR_CONFIG_FAILED;
        }
    }
    
    return result;
}

rs485_error_t rs485_set_baudrate(rs485_handle_t handle, uint32_t baudrate)
{
    rs485_error_t result = RS485_OK;
    
    if (handle == NULL) {
        result = RS485_ERR_NULL_POINTER;
    } else {
        handle->config.baudrate = baudrate;
        
        if (handle->is_open) {
            result = configure_port(handle);
        }
    }
    
    return result;
}

rs485_error_t rs485_set_params(rs485_handle_t handle,
                               uint8_t data_bits,
                               rs485_parity_t parity,
                               uint8_t stop_bits)
{
    rs485_error_t result = RS485_OK;
    
    if (handle == NULL) {
        result = RS485_ERR_NULL_POINTER;
    } else if ((data_bits != 7U) && (data_bits != 8U)) {
        result = RS485_ERR_INVALID_PARAM;
    } else if ((stop_bits != 1U) && (stop_bits != 2U)) {
        result = RS485_ERR_INVALID_PARAM;
    } else {
        handle->config.data_bits = data_bits;
        handle->config.parity = parity;
        handle->config.stop_bits = stop_bits;
        
        if (handle->is_open) {
            result = configure_port(handle);
        }
    }
    
    return result;
}

rs485_error_t rs485_get_stats(rs485_handle_t handle, rs485_stats_t *stats)
{
    rs485_error_t result = RS485_OK;
    
    if ((handle == NULL) || (stats == NULL)) {
        result = RS485_ERR_NULL_POINTER;
    } else {
        *stats = handle->stats;
    }
    
    return result;
}

rs485_error_t rs485_reset_stats(rs485_handle_t handle)
{
    rs485_error_t result = RS485_OK;
    
    if (handle == NULL) {
        result = RS485_ERR_NULL_POINTER;
    } else {
        (void)memset(&handle->stats, 0, sizeof(handle->stats));
    }
    
    return result;
}

rs485_error_t rs485_bytes_available(rs485_handle_t handle, size_t *available)
{
    rs485_error_t result = RS485_OK;
    
    if ((handle == NULL) || (available == NULL)) {
        result = RS485_ERR_NULL_POINTER;
    } else if (!handle->is_open) {
        result = RS485_ERR_NOT_OPEN;
    } else {
        int32_t bytes = 0;
        if (ioctl(handle->fd, FIONREAD, &bytes) < 0) {
            result = RS485_ERR_READ_FAILED;
            *available = 0U;
        } else {
            *available = (size_t)bytes;
        }
    }
    
    return result;
}

const char *rs485_error_string(rs485_error_t error)
{
    const char *str;
    int32_t idx = -((int32_t)error);
    
    if ((idx >= 0) && ((size_t)idx < ERROR_STRING_COUNT)) {
        str = error_strings[idx];
    } else {
        str = "Unknown error";
    }
    
    return str;
}

