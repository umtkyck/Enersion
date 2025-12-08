/**
 * @file enersion_crc.h
 * @brief Enersion CRC16 Calculation - MISRA C:2012 Compliant
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef ENERSION_CRC_H
#define ENERSION_CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ============================================================================
 * CRC Constants
 * ============================================================================ */

#define ENERSION_CRC_INIT   (0xFFFFU)
#define ENERSION_CRC_POLY   (0xA001U)   /**< CRC-16/MODBUS polynomial */

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Calculate CRC16 checksum
 * 
 * Uses CRC-16/MODBUS polynomial: x^16 + x^15 + x^2 + 1
 * 
 * @param[in] data    Data buffer (must not be NULL)
 * @param[in] length  Data length
 * @return uint16_t CRC16 value
 */
uint16_t enersion_crc16(const uint8_t *data, size_t length);

/**
 * @brief Calculate CRC16 incrementally
 * 
 * @param[in] crc     Current CRC value
 * @param[in] data    Data buffer (must not be NULL)
 * @param[in] length  Data length
 * @return uint16_t Updated CRC16 value
 */
uint16_t enersion_crc16_update(uint16_t crc, const uint8_t *data, size_t length);

/**
 * @brief Verify CRC16 checksum
 * 
 * @param[in] data    Data buffer including CRC (must not be NULL)
 * @param[in] length  Total length including 2-byte CRC
 * @return bool true if CRC is valid
 */
bool enersion_crc16_verify(const uint8_t *data, size_t length);

/**
 * @brief Append CRC16 to buffer
 * 
 * @param[in,out] data    Data buffer (must have 2 extra bytes)
 * @param[in]     length  Data length (not including CRC)
 */
void enersion_crc16_append(uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* ENERSION_CRC_H */

