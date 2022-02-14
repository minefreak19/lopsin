/*
Created 05 January 2022
 */

#ifndef LOPSINVM_H_
#define LOPSINVM_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define LOPSINVM_BYTECODE_MAGIC "\105\114\117\120\122\151\102\141"

typedef union {
    int64_t as_i64;
    bool as_boolean;
    void *as_ptr;
    double as_f64;
} LopsinValue;

typedef enum {
    ERR_OK = 0,

    ERR_DSTACK_UNDERFLOW,
    ERR_DSTACK_OVERFLOW,

    ERR_RSTACK_UNDERFLOW,
    ERR_RSTACK_OVERFLOW,

    ERR_ILLEGAL_INST,
    ERR_BAD_INST_PTR,
    // TODO(#6): more specialised memory errors
    ERR_BAD_MEM_PTR,
    ERR_OUT_OF_MEMORY,
    ERR_HALTED,
    ERR_INVALID_OPERAND,
    ERR_NATIVE_ERROR,
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

    LOPSIN_INST_R8,
    LOPSIN_INST_R16,
    LOPSIN_INST_R32,
    LOPSIN_INST_R64,
    LOPSIN_INST_W8,
    LOPSIN_INST_W16,
    LOPSIN_INST_W32,
    LOPSIN_INST_W64,

    LOPSIN_INST_ISUM,
    LOPSIN_INST_ISUB,
    LOPSIN_INST_IMUL,
    LOPSIN_INST_IDIV,
    LOPSIN_INST_IMOD,

    LOPSIN_INST_FSUM,
    LOPSIN_INST_FSUB,
    LOPSIN_INST_FMUL,
    LOPSIN_INST_FDIV,
    LOPSIN_INST_FMOD,

    LOPSIN_INST_I2F,
    LOPSIN_INST_F2I,

    LOPSIN_INST_IGT,
    LOPSIN_INST_ILT,
    LOPSIN_INST_IGTE,
    LOPSIN_INST_ILTE,
    LOPSIN_INST_IEQ,
    LOPSIN_INST_INEQ,

    LOPSIN_INST_FGT,
    LOPSIN_INST_FLT,
    LOPSIN_INST_FGTE,
    LOPSIN_INST_FLTE,
    LOPSIN_INST_FEQ,
    LOPSIN_INST_FNEQ,

    LOPSIN_INST_SHL,
    LOPSIN_INST_SHR,
    LOPSIN_INST_BOR,
    LOPSIN_INST_BAND,
    LOPSIN_INST_XOR,
    LOPSIN_INST_BNOT,
    LOPSIN_INST_LOR,
    LOPSIN_INST_LAND,
    LOPSIN_INST_LNOT,

    LOPSIN_INST_JMP,
    LOPSIN_INST_CJMP,
    LOPSIN_INST_RJMP,
    LOPSIN_INST_CRJMP,
    LOPSIN_INST_CALL,
    LOPSIN_INST_RET,
    LOPSIN_INST_NCALL,

    COUNT_LOPSIN_INST_TYPES
} LopsinInstType;

typedef enum {
    LOPSIN_NATIVE_PUTI,
    LOPSIN_NATIVE_PUTF,
    LOPSIN_NATIVE_PUTC,
    LOPSIN_NATIVE_READ,
    LOPSIN_NATIVE_MALLOC,
    LOPSIN_NATIVE_FREE,
    LOPSIN_NATIVE_TIME,
    COUNT_LOPSIN_NATIVES
} LopsinNativeType;

typedef struct {
    LopsinInstType type;
    uint32_t _explicit_padding;
    LopsinValue operand;
} LopsinInst;

static_assert(sizeof(LopsinInst) == 16, "");

#define LOPSINVM_DEFAULT_PROGRAM_COUNT 1024
#define LOPSINVM_DEFAULT_DSTACK_CAP 1024
#define LOPSINVM_DEFAULT_RSTACK_CAP 1024
#define LOPSINVM_ALLOCED_CHUNKS_CAP 1024

typedef struct {
    LopsinInst *insts;
    size_t count;
    size_t cap;
} LopsinVMProgram;

typedef struct {
    void *ptr;
    size_t bytes;
} Mem_Chunk;

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

    /// Allocated memory chunks
    Mem_Chunk alloced[LOPSINVM_ALLOCED_CHUNKS_CAP];
    size_t alloced_count;

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

void lopsinvm_new(LopsinVM *);
void lopsinvm_free(LopsinVM *);

void lopsinvm_load_program_from_file(LopsinVM *, const char *path);

LopsinErr lopsinvm_run_inst(LopsinVM *);
LopsinErr lopsinvm_start(LopsinVM *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOPSINVM_H_ */
