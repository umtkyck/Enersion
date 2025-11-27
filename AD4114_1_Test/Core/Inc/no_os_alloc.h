/***************************************************************************//**
 *   @file   no_os_alloc.h
 *   @brief  Memory allocation functions
 *******************************************************************************/

#ifndef NO_OS_ALLOC_H_
#define NO_OS_ALLOC_H_

#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocate memory
 * @param size - Size in bytes
 * @return Pointer to allocated memory or NULL
 */
static inline void *no_os_malloc(size_t size)
{
    return malloc(size);
}

/**
 * @brief Allocate and zero memory
 * @param nitems - Number of items
 * @param size - Size of each item
 * @return Pointer to allocated memory or NULL
 */
static inline void *no_os_calloc(size_t nitems, size_t size)
{
    return calloc(nitems, size);
}

/**
 * @brief Free memory
 * @param ptr - Pointer to memory
 */
static inline void no_os_free(void *ptr)
{
    free(ptr);
}

#endif /* NO_OS_ALLOC_H_ */

