/*
Created 01 February 2022
 */

#ifndef NATIVES_H_
#define NATIVES_H_

#include "./lopsinvm.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

LopsinErr lopsin_native_hello(LopsinVM *vm);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NATIVES_H_ */

#ifdef NATIVES_IMPLEMENTATION
#undef NATIVES_IMPLEMENTATION

#include <stdio.h>

LopsinErr lopsin_native_hello(LopsinVM *vm)
{
    (void) vm;
    printf("Hello, World!\n");
    return ERR_OK;
}

#endif // NATIVES_IMPLEMENTATION
