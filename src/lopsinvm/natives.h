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

#define NATIVE(name) LopsinErr lopsin_native_##name(LopsinVM *)

static_assert(COUNT_LOPSIN_NATIVES == 3, "Exhaustive definition of native functions");
NATIVE(dump);
NATIVE(putc);
NATIVE(read);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NATIVES_H_ */

#ifdef NATIVES_IMPLEMENTATION
#undef NATIVES_IMPLEMENTATION

#include <stdio.h>
#include "./lopsinvm.h"

LopsinErr lopsin_native_dump(LopsinVM *vm)
{
    if (vm->dsp < 1) return ERR_DSTACK_UNDERFLOW;
    lopsinvalue_print(stdout, vm->dstack[--vm->dsp]);
    return ERR_OK;
}

LopsinErr lopsin_native_putc(LopsinVM *vm)
{
    if (vm->dsp < 1) return ERR_DSTACK_UNDERFLOW;
    putc(vm->dstack[--vm->dsp].as.i64, stdout);
    return ERR_OK;
}

LopsinErr lopsin_native_read(LopsinVM *vm) 
{
    if (vm->dsp >= vm->dstack_cap) return ERR_DSTACK_OVERFLOW;
    int64_t x;
    scanf("%"PRId64, &x);
    vm->dstack[vm->dsp++] = (LopsinValue) { .type = LOPSIN_TYPE_I64, .as.i64 = x };
    return ERR_OK;
}

#endif // NATIVES_IMPLEMENTATION
