#include "./lopsinvm.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFERDEF static inline
#define BUFFER_IMPLEMENTATION
#include <buffer.h>

#define SVDEF static inline
#define SV_IMPLEMENTATION
#include <sv.h>

#include "util.h"

#define NATIVES_IMPLEMENTATION
#include "./natives.h"

static_assert(COUNT_LOPSIN_INST_TYPES == 34, "Exhaustive definition of LOPSIN_INST_TYPE_NAMES with respect to LopsinInstType's");
const char * const LOPSIN_INST_TYPE_NAMES[COUNT_LOPSIN_INST_TYPES] = {
    [LOPSIN_INST_NOP]           = "nop",
    [LOPSIN_INST_HLT]           = "hlt",

    [LOPSIN_INST_PUSH]          = "push",
    [LOPSIN_INST_DROP]          = "drop",
    [LOPSIN_INST_DUP]           = "dup",
    [LOPSIN_INST_SWAP]          = "swap",
    
    [LOPSIN_INST_SUM]           = "sum",
    [LOPSIN_INST_SUB]           = "sub",
    [LOPSIN_INST_MUL]           = "mul",
    [LOPSIN_INST_DIV]           = "div",
    [LOPSIN_INST_MOD]           = "mod",

    [LOPSIN_INST_SHL]           = "shl",
    [LOPSIN_INST_SHR]           = "shr",
    [LOPSIN_INST_BOR]           = "bor",
    [LOPSIN_INST_BAND]          = "band",
    [LOPSIN_INST_XOR]           = "xor",
    [LOPSIN_INST_BNOT]          = "bnot",
    [LOPSIN_INST_LOR]           = "lor",
    [LOPSIN_INST_LAND]          = "land",
    [LOPSIN_INST_LNOT]          = "lnot",
    
    [LOPSIN_INST_GT]            = "gt",
    [LOPSIN_INST_LT]            = "lt",
    [LOPSIN_INST_GTE]           = "gte",
    [LOPSIN_INST_LTE]           = "lte",
    [LOPSIN_INST_EQ]            = "eq",
    [LOPSIN_INST_NEQ]           = "neq",
    
    [LOPSIN_INST_JMP]           = "jmp",
    [LOPSIN_INST_CJMP]          = "cjmp",
    [LOPSIN_INST_RJMP]          = "rjmp",
    [LOPSIN_INST_CRJMP]         = "crjmp",
    [LOPSIN_INST_CALL]          = "call",
    [LOPSIN_INST_RET]           = "ret",
    [LOPSIN_INST_NCALL]         = "ncall",

    [LOPSIN_INST_CAST]          = "cast",
};

static_assert(COUNT_LOPSIN_ERRS == 11, "Exhaustive definition of LOPSIN_ERR_NAMES with respct to LopsinErr's");
const char * const LOPSIN_ERR_NAMES[COUNT_LOPSIN_ERRS] = {
    [ERR_OK]                = "OK",

    [ERR_DSTACK_UNDERFLOW]  = "Data stack underflow",
    [ERR_DSTACK_OVERFLOW]   = "Data stack overflow",
    [ERR_RSTACK_UNDERFLOW]  = "Return stack underflow",
    [ERR_RSTACK_OVERFLOW]   = "Return stack overflow",

    [ERR_ILLEGAL_INST]      = "Illegal instruction",
    [ERR_BAD_INST_PTR]      = "Bad instruction pointer",
    [ERR_HALTED]            = "Already halted",
    [ERR_INVALID_OPERAND]   = "Invalid operand for operation",
    [ERR_INVALID_TYPE]      = "Invalid type for operation",

    [ERR_DIV_BY_ZERO]       = "Division by zero",
};

static_assert(COUNT_LOPSIN_TYPES == 4, "Exhaustive definition of LOPSIN_TYPE_NAMES[] with respect to LopsinType's");
const char * const LOPSIN_TYPE_NAMES[COUNT_LOPSIN_TYPES] = {
    [LOPSIN_TYPE_VOID]      = "void",
    [LOPSIN_TYPE_I64]       = "i64",
    [LOPSIN_TYPE_BOOL]      = "bool",
    [LOPSIN_TYPE_PTR]       = "ptr",
};


static_assert(COUNT_LOPSIN_NATIVES == 3, "Exhaustive definition of LOPSIN_NATIVES[] with respect to LopsinNativeType's");
const LopsinNative LOPSIN_NATIVES[COUNT_LOPSIN_NATIVES] = {
    [LOPSIN_NATIVE_DUMP] = { .name = "dump", .proc = &lopsin_native_dump },
    [LOPSIN_NATIVE_PUTC] = { .name = "putc", .proc = &lopsin_native_putc },
    [LOPSIN_NATIVE_READ] = { .name = "read", .proc = &lopsin_native_read },
};

static void lopvm_dump_stack(FILE *stream, const LopsinVM *vm)
{
    fprintf(stream, 
        "Stack pointer: %zu\n"
        "Inst pointer:  %zu\n"
        "Stack: \n",
        
        vm->dsp, 
        vm->ip);

    for (size_t i = 0; i < vm->dsp; i++) {
        LopsinValue a = vm->dstack[i];

        fprintf(stream, "\t");
        lopsinvalue_print(stream, a);
        fprintf(stream, "\n");
    }
}

bool requires_operand(LopsinInstType insttype)
{
    static_assert(COUNT_LOPSIN_INST_TYPES == 34, "Exhaustive handling of LopsinInstType's in requires_operand");

    switch (insttype) {
    case LOPSIN_INST_NOP:
    case LOPSIN_INST_HLT:
    case LOPSIN_INST_SUM:
    case LOPSIN_INST_SUB:
    case LOPSIN_INST_MUL:
    case LOPSIN_INST_DIV:
    case LOPSIN_INST_MOD:
    case LOPSIN_INST_SHL:
    case LOPSIN_INST_SHR:
    case LOPSIN_INST_BOR:
    case LOPSIN_INST_BAND:
    case LOPSIN_INST_XOR:
    case LOPSIN_INST_BNOT:
    case LOPSIN_INST_LOR:
    case LOPSIN_INST_LAND:
    case LOPSIN_INST_LNOT:
    case LOPSIN_INST_GT:
    case LOPSIN_INST_LT:
    case LOPSIN_INST_GTE:
    case LOPSIN_INST_LTE:
    case LOPSIN_INST_EQ:
    case LOPSIN_INST_NEQ:
    case LOPSIN_INST_RET:
        return false;

    case LOPSIN_INST_PUSH:
    case LOPSIN_INST_DROP:
    case LOPSIN_INST_DUP:
    case LOPSIN_INST_SWAP:
    case LOPSIN_INST_JMP:
    case LOPSIN_INST_CJMP:
    case LOPSIN_INST_RJMP:
    case LOPSIN_INST_CRJMP:
    case LOPSIN_INST_CALL:
    case LOPSIN_INST_NCALL:
    case LOPSIN_INST_CAST:
        return true;

    default: {
        CRASH("unreachable");
    }
    }
}

LopsinErr lopsinvm_run_inst(LopsinVM *vm)
{
    static_assert(COUNT_LOPSIN_INST_TYPES == 34, "Exhaustive handling of LopsinInstType's in lopsinvm_run_inst()");
    
    if (!vm->running) {
        return ERR_HALTED;
    }

    if (vm->ip >= vm->program.count) {
        return ERR_BAD_INST_PTR;
    }
    LopsinInst inst = vm->program.insts[vm->ip];
    
    if (vm->debug_mode) {
        fprintf(stdout, "Current instruction: %s ", 
            LOPSIN_INST_TYPE_NAMES[inst.type]);
        lopsinvalue_print(stdout, inst.operand);
        fprintf(stdout, "\n");

        lopvm_dump_stack(stdout, vm);
    }

    switch (inst.type) {
    
    case LOPSIN_INST_NOP: {
        vm->ip++;
    } break;

    case LOPSIN_INST_HLT: {
        vm->running = false;
    } break;

    case LOPSIN_INST_PUSH: {
        if (vm->dsp >= vm->dstack_cap) {
            return ERR_DSTACK_OVERFLOW;
        }
        vm->dstack[vm->dsp++] = inst.operand;
        vm->ip++;
    } break;

    case LOPSIN_INST_DROP: {
        if (inst.operand.type != LOPSIN_TYPE_I64 
            || inst.operand.as.i64 < 0)
        {
            return ERR_INVALID_OPERAND;
        }

        if (vm->dsp < (unsigned) inst.operand.as.i64) {
            return ERR_DSTACK_UNDERFLOW;
        }

        vm->dsp -= inst.operand.as.i64;
        vm->ip++;
    } break;

    case LOPSIN_INST_DUP: {
        if (inst.operand.type != LOPSIN_TYPE_I64 
            || inst.operand.as.i64 <= 0) {
            return ERR_INVALID_OPERAND;
        }

        if (vm->dsp + inst.operand.as.i64 >= vm->dstack_cap) {
            return ERR_DSTACK_OVERFLOW;
        }

        if (vm->dsp < (unsigned) inst.operand.as.i64) {
            return ERR_DSTACK_UNDERFLOW;
        }

        size_t saved_sp = vm->dsp;
        for (int64_t i = 0; i < inst.operand.as.i64; i++) {
            vm->dstack[vm->dsp++] = vm->dstack[saved_sp - (inst.operand.as.i64 - i)];
        }

        vm->ip++;
    } break;

    case LOPSIN_INST_SWAP: {
        if (inst.operand.type != LOPSIN_TYPE_I64 || inst.operand.as.i64 <= 0) {
            return ERR_INVALID_OPERAND;
        }

        const size_t operand = (size_t)inst.operand.as.i64;
        if (vm->dsp < (2 * operand)) {
            return ERR_DSTACK_UNDERFLOW;
        }

        size_t swap_sz = operand * sizeof(LopsinValue);
        LopsinValue *temp = NOTNULL(malloc(swap_sz));
        memcpy(temp, &vm->dstack[vm->dsp - (2 * operand)], swap_sz);
        memcpy(&vm->dstack[vm->dsp - (2 * operand)], &vm->dstack[vm->dsp - operand], swap_sz);
        memcpy(&vm->dstack[vm->dsp - operand], temp, swap_sz);


        // LopsinValue temp = vm->dstack[vm->dsp - 1];
        // vm->dstack[vm->dsp - 1] = vm->dstack[vm->dsp - 2];
        // vm->dstack[vm->dsp - 2] = temp;

        vm->ip++;
    } break;

    case LOPSIN_INST_SUM: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 + a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_SUB: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 - a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_MUL: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 * a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_DIV: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        if (a.as.i64 == 0) return ERR_DIV_BY_ZERO;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 / a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_MOD: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        if (a.as.i64 == 0) return ERR_DIV_BY_ZERO;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 % a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_SHL: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 << a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_SHR: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 >> a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_BOR: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 | a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_BAND: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 & a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_XOR: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .i64 = b.as.i64 ^ a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_BNOT: {
        if (vm->dsp < 1) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];

        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as.i64 = ~a.as.i64,
        };
    } break;

    case LOPSIN_INST_LOR: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .boolean = b.as.boolean || a.as.boolean,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_LAND: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_BOOL) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as = {
                .boolean = b.as.boolean && a.as.boolean,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_LNOT: {
        if (vm->dsp < 1) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];

        if (a.type != LOPSIN_TYPE_BOOL) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = a.type,
            .as.boolean = !a.as.boolean,
        };
    } break;

    case LOPSIN_INST_GT: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = LOPSIN_TYPE_BOOL,
            .as = {
                .boolean = b.as.i64 > a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_LT: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = LOPSIN_TYPE_BOOL,
            .as = {
                .boolean = b.as.i64 < a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_GTE: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = LOPSIN_TYPE_BOOL,
            .as = {
                .boolean = b.as.i64 >= a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_LTE: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        if (a.type != b.type) return ERR_INVALID_TYPE;
        if (a.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = LOPSIN_TYPE_BOOL,
            .as = {
                .boolean = b.as.i64 <= a.as.i64,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_EQ: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];
        
        bool result;

        if (a.type != b.type) {
            result = false;
        } else {
            switch (a.type) {
                case LOPSIN_TYPE_BOOL: result = a.as.boolean == b.as.boolean; break;
                case LOPSIN_TYPE_I64:  result = a.as.i64 == b.as.i64; break;

                default: {
                    CRASH("unreachable");
                }
            }
        }

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = LOPSIN_TYPE_BOOL,
            .as = {
                .boolean = result,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_NEQ: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        bool result;

        if (a.type != b.type) {
            result = true;
        } else {
            switch (a.type) {
                case LOPSIN_TYPE_BOOL: result = a.as.boolean != b.as.boolean; break;
                case LOPSIN_TYPE_I64:  result = a.as.i64 != b.as.i64; break;

                default: {
                    CRASH("unreachable");
                }
            }
        }

        vm->dstack[vm->dsp++] = (LopsinValue) {
            .type = LOPSIN_TYPE_BOOL,
            .as = {
                .boolean = result,
            },
        };

        vm->ip++;
    } break;

    case LOPSIN_INST_JMP: {
        if (inst.operand.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;
        vm->ip = inst.operand.as.i64;
    } break;

    case LOPSIN_INST_CJMP: {
        if (inst.operand.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;

        if (vm->dsp <= 0) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];

        if (a.type != LOPSIN_TYPE_BOOL) return ERR_INVALID_TYPE;

        if (a.as.boolean) {
            vm->ip = inst.operand.as.i64;
        } else {
            vm->ip++;
        }
    } break;

    case LOPSIN_INST_RJMP: {
        if (inst.operand.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;
        vm->ip += inst.operand.as.i64;
    } break;

    case LOPSIN_INST_CRJMP: {
        if (inst.operand.type != LOPSIN_TYPE_I64) return ERR_INVALID_OPERAND;

        if (vm->dsp <= 0) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];

        if (a.type != LOPSIN_TYPE_BOOL) return ERR_INVALID_TYPE;

        if (a.as.boolean) {
            vm->ip += inst.operand.as.i64;
        } else {
            vm->ip++;
        }
    } break;

    case LOPSIN_INST_CALL: {
        if (inst.operand.type != LOPSIN_TYPE_I64) return ERR_INVALID_OPERAND;
        if (vm->rsp >= vm->rstack_cap) return ERR_RSTACK_OVERFLOW;

        vm->rstack[vm->rsp++] = vm->ip + 1;
        vm->ip = inst.operand.as.i64;
    } break;

    case LOPSIN_INST_RET: {
        if (vm->rsp <= 0) return ERR_RSTACK_UNDERFLOW;

        vm->ip = vm->rstack[--vm->rsp];
    } break;

    case LOPSIN_INST_NCALL: {
        LopsinNativeType idx = inst.operand.as.i64;
        if (idx < 0 || idx > COUNT_LOPSIN_NATIVES) return ERR_INVALID_OPERAND;

        LopsinNative native = LOPSIN_NATIVES[idx];
        LopsinErr errlvl = (*native.proc)(vm);
        if (errlvl != ERR_OK) return errlvl;
        
        vm->ip++; 
        // if the natives want to keep the ip they can just ip-- it.
    } break;

    case LOPSIN_INST_CAST: {
        if (inst.operand.type != LOPSIN_TYPE_I64) return ERR_INVALID_TYPE;
        if (vm->dsp < 1) return ERR_DSTACK_UNDERFLOW;

        if (!(0 < inst.operand.as.i64 && inst.operand.as.i64 < COUNT_LOPSIN_TYPES)) 
            return ERR_INVALID_OPERAND;

        vm->dstack[vm->dsp - 1].type = inst.operand.as.i64;

        vm->ip++;
    } break;

    default: {
        return ERR_ILLEGAL_INST;
    }
    }
    
    return ERR_OK;
}

void lopsinvalue_print(FILE *stream, LopsinValue value)
{
    static_assert(COUNT_LOPSIN_TYPES == 4, "Exhaustive handling of LopsinType's in lopsinvalue_print");
    switch (value.type) {
    case LOPSIN_TYPE_VOID: fprintf(stream, "%s", "(void)"); break;
    case LOPSIN_TYPE_I64 : fprintf(stream, "%"PRId64, value.as.i64); break;
    case LOPSIN_TYPE_PTR:  fprintf(stream, "%p", value.as.ptr); break;
    case LOPSIN_TYPE_BOOL: fprintf(stream, "%s", value.as.boolean ? "true" : "false"); break;

    default: {
        return;
    }
    }
}

LopsinErr lopsinvm_start(LopsinVM *vm)
{
    LopsinErr err = 0;

    vm->running = true;

    while (vm->running) {
        err = lopsinvm_run_inst(vm);
        if (err) {
            fprintf(stderr, "ERROR: At inst %zu: %s\n", vm->ip, ERR_AS_CSTR(err));
            vm->running = false;
        }
    }

    return err;
}

LopsinVM lopsinvm_new(void)
{
    return (LopsinVM) {
        .debug_mode = false,
        .running = false,
        
        .ip = 0,
        .program = {
            .insts = NULL,
            .count = 0,
            .cap = 0,
        },
        
        .dsp = 0,
        .dstack = NOTNULL(calloc(LOPSINVM_DEFAULT_DSTACK_CAP, sizeof(LopsinValue))),
        .dstack_cap = LOPSINVM_DEFAULT_DSTACK_CAP,

        .rsp = 0,
        .rstack = NOTNULL(calloc(LOPSINVM_DEFAULT_RSTACK_CAP, sizeof(size_t))),
        .rstack_cap = LOPSINVM_DEFAULT_RSTACK_CAP,
    };
}

void lopsinvm_free(LopsinVM *vm)
{
    assert(!vm->running);

    free(vm->dstack);
    free(vm->rstack);
    free(vm->program.insts);
}

static inline bool sv_try_chop_by_sv_left(String_View *sv, 
                                   const String_View thicc_delim, 
                                   String_View *out)
{
    String_View window = sv_from_parts(sv->data, thicc_delim.count);
    size_t i = 0;
    while (i + thicc_delim.count < sv->count 
        && !(sv_eq(window, thicc_delim))) 
    {
        i++;
        window.data++;
    }

    String_View result = sv_from_parts(sv->data, i);

    if (i + thicc_delim.count == sv->count) {
        return false;
    }
    
    // Chop!
    sv->data  += i + thicc_delim.count;
    sv->count -= i + thicc_delim.count;

    if (out) *out = result;
    return true;
}

void lopsinvm_load_program_from_file(LopsinVM *vm, const char *path)
{
    Buffer *buf = new_buffer(0);

    buffer_append_file(buf, path);

    String_View bytecode = sv_from_parts(buf->data, buf->size);
    const String_View magic = SV_STATIC(LOPSINVM_BYTECODE_MAGIC);

    if (sv_try_chop_by_sv_left(&bytecode, magic, NULL)) {
        size_t count = bytecode.count / sizeof(LopsinInst);
        LopsinVMProgram program = {
            .cap   = count,
            .count = count,
            .insts = NOTNULL(malloc(bytecode.count)),
        };

        memcpy(program.insts, bytecode.data, bytecode.count);

        vm->program = program;
    } else {
        fprintf(stderr, "ERROR: Could not load program from file %s: %s\n",
                path, "Incorrect format");
        exit(1);
    }

    buffer_clear(buf);
    buffer_free(buf);
}
