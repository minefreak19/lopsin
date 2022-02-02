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

static_assert(COUNT_LOPSIN_NATIVES == 5, "Exhaustive definition of native functions");
LopsinErr lopsin_native_dump   (LopsinVM *vm);
LopsinErr lopsin_native_putc   (LopsinVM *vm);
LopsinErr lopsin_native_read   (LopsinVM *vm);
LopsinErr lopsin_native_malloc (LopsinVM *vm);
LopsinErr lopsin_native_free   (LopsinVM *vm);

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
    putc(vm->dstack[--vm->dsp].as_i64, stdout);
    return ERR_OK;
}

LopsinErr lopsin_native_read(LopsinVM *vm) 
{
    if (vm->dsp >= vm->dstack_cap) return ERR_DSTACK_OVERFLOW;
    int64_t x;
    scanf("%"PRId64, &x);
    vm->dstack[vm->dsp++] = (LopsinValue) { .as_i64 = x };
    return ERR_OK;
}

LopsinErr lopsin_native_malloc(LopsinVM *vm)
{
    if (vm->dsp < 1) return ERR_DSTACK_UNDERFLOW;
    size_t bytes = vm->dstack[--vm->dsp].as_i64;

    assert(vm->alloced_count < LOPSINVM_ALLOCED_CHUNKS_CAP);

    void *ptr = malloc(bytes);
    if (ptr == NULL) return ERR_OUT_OF_MEMORY;

    if (vm->dsp >= vm->dstack_cap) {
        free(ptr);
        return ERR_DSTACK_OVERFLOW;
    }

    vm->alloced[vm->alloced_count++] = (Mem_Chunk) { .ptr = ptr, .bytes = bytes, };

    vm->dstack[vm->dsp++].as_ptr = ptr;
    return ERR_OK;
}

LopsinErr lopsin_native_free(LopsinVM *vm)
{
    if (vm->dsp < 1) return ERR_DSTACK_UNDERFLOW;

    void *ptr = vm->dstack[--vm->dsp].as_ptr;

    assert(vm->alloced_count > 0);
    bool found = false;
    for (size_t i = 0; i < vm->alloced_count; i++) {
        if (found) {
            vm->alloced[i - 1] = vm->alloced[i];
        }
        if (vm->alloced[i].ptr == ptr) {
            found = true;
            // this will get overwritten on the next iteration
        }
    }

    if (!found) return ERR_BAD_MEM_PTR;

    vm->alloced_count--;
    free(ptr);
    return ERR_OK;
}

#endif // NATIVES_IMPLEMENTATION
