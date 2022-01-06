/*
Created 06 January 2022
 */

#ifndef NOBUILD_COMMON_H_
#define NOBUILD_COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define CFLAGS "-std=c11", "-Wall", "-Wextra", "-Werror", "-Wpedantic", "-Wno-missing-braces", "-Wmissing-prototypes", "-static"
#define DEBUG_CFLAGS CFLAGS, "-ggdb", "-D_DEBUG"
#define BUILD_CFLAGS CFLAGS, "-O3"

#define NOBUILD_CFLAGS "-std=c99", "-O3", "-DCC=\""CC"\""

#define LDFLAGS "-g", "-static"

#define SRCDIR "src"
#define BINDIR "bin"

#define C_INCLUDES "-I" SRCDIR "/common/"

const char * const MODULES[] = {
    "lopsinvm",
};

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))


#ifndef CC
#define CC "cc"
#endif

typedef enum {
    MODE_DEBUG,
    MODE_BUILD,
} Mode;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NOBUILD_COMMON_H_ */
