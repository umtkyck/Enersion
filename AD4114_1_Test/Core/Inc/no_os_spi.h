/***************************************************************************//**
 *   @file   no_os_spi.h
 *   @brief  Platform-independent SPI interface - STM32 HAL implementation
 *******************************************************************************/

#ifndef NO_OS_SPI_H_
#define NO_OS_SPI_H_

#include <stdint.h>
#include "stm32h7xx_hal.h"

/**
 * @struct no_os_spi_desc
 * @brief SPI descriptor
 */
struct no_os_spi_desc {
    SPI_HandleTypeDef *hspi;
    uint32_t chip_select_port;
    uint16_t chip_select_pin;
};

/**
 * @struct no_os_spi_init_param
 * @brief SPI initialization parameters
 */
struct no_os_spi_init_param {
    SPI_HandleTypeDef *hspi;
    uint32_t chip_select_port;
    uint16_t chip_select_pin;
};

/**
 * @brief Initialize SPI
 * @param desc - SPI descriptor
 * @param param - Initialization parameters
 * @return 0 for success, negative error code otherwise
 */
int32_t no_os_spi_init(struct no_os_spi_desc **desc,
                       const struct no_os_spi_init_param *param);

/**
 * @brief Free SPI resources
 * @param desc - SPI descriptor
 * @return 0 for success, negative error code otherwise
 */
int32_t no_os_spi_remove(struct no_os_spi_desc *desc);

/**
 * @brief Write and read data via SPI
 * @param desc - SPI descriptor
 * @param data - Data buffer (TX and RX)
 * @param bytes_number - Number of bytes to transfer
 * @return 0 for success, negative error code otherwise
 */
int32_t no_os_spi_write_and_read(struct no_os_spi_desc *desc,
                                 uint8_t *data,
                                 uint16_t bytes_number);

#endif /* NO_OS_SPI_H_ */

