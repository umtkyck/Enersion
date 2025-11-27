/***************************************************************************//**
 *   @file   no_os_util.h
 *   @brief  Utility macros
 *******************************************************************************/

#ifndef NO_OS_UTIL_H_
#define NO_OS_UTIL_H_

#include <stdint.h>

/* Generate a mask with bits from 'h' to 'l' set */
#define NO_OS_GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32 - 1 - (h))))

/* Get the field value from a register */
#define NO_OS_FIELD_GET(mask, val) \
    (((val) & (mask)) >> __builtin_ctz(mask))

/* Prepare a field value for a register */
#define NO_OS_FIELD_PREP(mask, val) \
    (((val) << __builtin_ctz(mask)) & (mask))

/* Array size */
#define NO_OS_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Min/Max */
#define NO_OS_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define NO_OS_MAX(a, b) (((a) > (b)) ? (a) : (b))

/* Bit operations */
#define NO_OS_BIT(x) (1UL << (x))

#endif /* NO_OS_UTIL_H_ */

