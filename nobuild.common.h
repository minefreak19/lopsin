/*
Created 06 January 2022
 */

#ifndef NOBUILD_COMMON_H_
#define NOBUILD_COMMON_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define CXXFLAGS "-std=c++2a", "-Wall", "-Wextra", "-Werror", "-Wpedantic", "-Wno-missing-braces", "-static"
#define DEBUG_CXXFLAGS CXXFLAGS, "-ggdb", "-D_DEBUG"
#define BUILD_CXXFLAGS CXXFLAGS, "-O3"

#define CFLAGS "-std=c11", "-Wall", "-Wextra", "-Werror", "-Wpedantic", "-Wno-missing-braces", "-Wmissing-prototypes", "-static", "-lm"
#define DEBUG_CFLAGS CFLAGS, "-ggdb", "-D_DEBUG"
#define BUILD_CFLAGS CFLAGS, "-O3"

#define NOBUILD_CFLAGS "-std=c11", "-O3"
#define SRCDIR "src"
#define BINDIR "bin"

#define C_INCLUDES JOIN("", "-I", PATH(SRCDIR, "common"))

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))


#ifndef CC
#define CC (getenv("CC"))
#endif

#ifndef CXX
#define CXX (getenv("CXX"))
#endif

typedef enum {
    MODE_DEBUG,
    MODE_BUILD,
} Mode;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NOBUILD_COMMON_H_ */
