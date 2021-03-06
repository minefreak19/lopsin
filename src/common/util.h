/*
Created 05 January 2022
 */

#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(__GNUC__) || defined(__clang__)
# define PRINTF_FORMAT(fmt_idx, chk_idx) __attribute__ ((format (printf, fmt_idx, chk_idx)))
# define FALLTHROUGH __attribute__ ((fallthrough))
#else
# define PRINTF_FORMAT(...)
# define FALLTHROUGH
#endif // defined(__GNUC__) || defined(__clang__)

#ifdef _DEBUG
# define NOTNULL(ptr) notnull_impl(__FILE__, __LINE__, __func__, #ptr, ptr)
#else
# define NOTNULL(ptr) notnull_impl(NULL, 0, NULL, NULL, ptr)
#endif
void *notnull_impl(const char *file, int line, const char *func, const char *expr, void *ptr);

#ifndef static_assert
#define static_assert _Static_assert
#endif

#define CRASH(message) assert(0 && message)
#define UNIMPLEMENTED CRASH("UNIMPLEMENTED")

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UTIL_H_ */
