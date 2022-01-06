/*
Created 05 January 2022
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(__GNUC__) || defined(__clang__)
# define PRINTF_FORMAT(fmt_idx, chk_idx) __attribute__ ((format (printf, fmt_idx, chk_idx)))
#else
# define PRINTF_FORMAT(...)
#endif // defined(__GNUC__) || defined(__clang__)

#ifdef _DEBUG
# define NOTNULL(ptr) notnull_impl(__FILE__, __LINE__, __func__, #ptr, ptr)
#else
# define NOTNULL(ptr) notnull_impl(NULL, 0, NULL, NULL, ptr)
#endif

static inline void *notnull_impl(const char *file, int line, const char *func, const char *expr, void *ptr)
{
    if (ptr == NULL) {
#ifdef _DEBUG
        fprintf(stderr, "%s:%d: %s: ERROR: `%s` == NULL\n", file, line, func, expr);
#else
        (void) file;
        (void) line;
        (void) func;
        (void) expr;

        fprintf(stderr, "ERROR: Null pointer allocated. Possibly out of memory.\n");
#endif
        exit(1);
    }
    return ptr;
}

#ifndef static_assert
#define static_assert _Static_assert
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UTIL_H_ */
