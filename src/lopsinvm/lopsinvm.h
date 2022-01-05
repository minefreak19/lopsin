/*
Created 05 January 2022
 */

#ifndef LOPSINVM_H_
#define LOPSINVM_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef int64_t LopsinValue;

typedef enum {
    ERR_OK = 0,

    ERR_STACK_UNDERFLOW,
    ERR_STACK_OVERFLOW,

    ERR_ILLEGAL_INST,
    ERR_BAD_INST_PTR,

    ERR_DIV_BY_ZERO,

    COUNT_LOPSIN_ERRS
} LopsinErr;

typedef enum {
    LOPSIN_INST_NOP = 0,

    LOPSIN_INST_PUSH,
    LOPSIN_INST_DROP,
    LOPSIN_INST_DUP,
    LOPSIN_INST_SWAP,

    LOPSIN_INST_SUM,
    LOPSIN_INST_SUB,
    LOPSIN_INST_MUL,
    LOPSIN_INST_DIV,
    LOPSIN_INST_MOD,

    LOPSIN_INST_JMP,
    LOPSIN_INST_CJMP,
    LOPSIN_INST_RJMP,
    LOPSIN_INST_CRJMP,
    
    LOPSIN_INST_DUMP,
    LOPSIN_INST_PUTC,

    COUNT_LOPSIN_INST_TYPES
} LopsinInstType;

typedef struct {
    LopsinInstType type;
    LopsinValue operand;
} LopsinInst;

typedef struct {
    LopsinValue *stack;
    size_t stack_cap;
    size_t sp;

    LopsinInst *program;
    size_t program_sz;

    size_t ip;

    bool debug_mode;
} LopsinVM;

#define ERR_AS_CSTR(err) (LOPSIN_ERR_NAMES[err])

#define MAKE_INST(insttype, value) { .type = LOPSIN_INST_##insttype, .operand = value }

extern const char * const LOPSIN_INST_TYPE_NAMES[COUNT_LOPSIN_INST_TYPES];
extern const char * const LOPSIN_ERR_NAMES[COUNT_LOPSIN_ERRS];

LopsinErr lopsinvm_run_inst(LopsinVM *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOPSINVM_H_ */
