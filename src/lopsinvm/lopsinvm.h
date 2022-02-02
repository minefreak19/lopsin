/*
Created 05 January 2022
 */

#ifndef LOPSINVM_H_
#define LOPSINVM_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define LOPSINVM_BYTECODE_MAGIC "\105\114\117\120\122\151\102\141"

typedef union {
    int64_t as_i64;
    bool as_boolean;
    void *as_ptr;
} LopsinValue;

typedef enum {
    ERR_OK = 0,

    ERR_DSTACK_UNDERFLOW,
    ERR_DSTACK_OVERFLOW,

    ERR_RSTACK_UNDERFLOW,
    ERR_RSTACK_OVERFLOW,
    
    ERR_ILLEGAL_INST,
    ERR_BAD_INST_PTR,
    ERR_BAD_MEM_PTR,
    ERR_OUT_OF_MEMORY,
    ERR_HALTED,
    ERR_INVALID_OPERAND,
    ERR_INVALID_TYPE,

    ERR_DIV_BY_ZERO,

    COUNT_LOPSIN_ERRS
} LopsinErr;

typedef enum {
    LOPSIN_INST_NOP = 0,
    LOPSIN_INST_HLT,

    LOPSIN_INST_PUSH,
    LOPSIN_INST_DROP,
    LOPSIN_INST_DUP,
    LOPSIN_INST_SWAP,

    LOPSIN_INST_SUM,
    LOPSIN_INST_SUB,
    LOPSIN_INST_MUL,
    LOPSIN_INST_DIV,
    LOPSIN_INST_MOD,

    LOPSIN_INST_SHL,
    LOPSIN_INST_SHR,
    LOPSIN_INST_BOR,
    LOPSIN_INST_BAND,
    LOPSIN_INST_XOR,
    LOPSIN_INST_BNOT,
    LOPSIN_INST_LOR,
    LOPSIN_INST_LAND,
    LOPSIN_INST_LNOT,

    LOPSIN_INST_GT,
    LOPSIN_INST_LT,
    LOPSIN_INST_GTE,
    LOPSIN_INST_LTE,
    LOPSIN_INST_EQ,
    LOPSIN_INST_NEQ,

    LOPSIN_INST_JMP,
    LOPSIN_INST_CJMP,
    LOPSIN_INST_RJMP,
    LOPSIN_INST_CRJMP,
    LOPSIN_INST_CALL,
    LOPSIN_INST_RET,
    LOPSIN_INST_NCALL,

    LOPSIN_INST_MEMRD,
    LOPSIN_INST_MEMWR,

    COUNT_LOPSIN_INST_TYPES
} LopsinInstType;

typedef enum {
    LOPSIN_NATIVE_DUMP = 0,
    LOPSIN_NATIVE_PUTC,
    LOPSIN_NATIVE_READ,
    LOPSIN_NATIVE_MALLOC,
    LOPSIN_NATIVE_FREE,
    COUNT_LOPSIN_NATIVES
} LopsinNativeType;

typedef struct {
    LopsinInstType type;
    LopsinValue operand;
} LopsinInst;

#define LOPSINVM_DEFAULT_PROGRAM_COUNT 1024
#define LOPSINVM_DEFAULT_DSTACK_CAP 1024
#define LOPSINVM_DEFAULT_RSTACK_CAP 1024

typedef struct {
    LopsinInst *insts;
    size_t count;
    size_t cap;
} LopsinVMProgram;

typedef struct {
    /// Data stack.
    LopsinValue *dstack;
    size_t dstack_cap;
    size_t dsp;

    /// Return stack.
    size_t *rstack;
    size_t rstack_cap;
    size_t rsp;

    /// Program.
    LopsinVMProgram program;

    /// Instruction pointer.
    size_t ip;

    /// flags
    bool debug_mode;
    bool running;
} LopsinVM;

typedef LopsinErr (*LopsinNativeProc)(LopsinVM *);
typedef struct {
    LopsinNativeProc proc;
    const char * const name;
} LopsinNative;

#define ERR_AS_CSTR(err) (LOPSIN_ERR_NAMES[err])

#define MAKE_INST(insttype, value) { .type = LOPSIN_INST_##insttype, .operand = value }

extern const char * const LOPSIN_INST_TYPE_NAMES[COUNT_LOPSIN_INST_TYPES];
extern const char * const LOPSIN_ERR_NAMES[COUNT_LOPSIN_ERRS];
extern const LopsinNative LOPSIN_NATIVES[COUNT_LOPSIN_NATIVES];

bool requires_operand(LopsinInstType insttype);
void lopsinvalue_print(FILE *stream, LopsinValue);

LopsinVM lopsinvm_new(void);
void lopsinvm_free(LopsinVM *);

void lopsinvm_load_program_from_file(LopsinVM *, const char *path);

LopsinErr lopsinvm_run_inst(LopsinVM *);
LopsinErr lopsinvm_start(LopsinVM *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOPSINVM_H_ */
