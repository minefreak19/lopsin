#include "./lopsinvm.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static_assert(COUNT_LOPSIN_INST_TYPES == 22, "Exhaustive definition of LOPSIN_INST_TYPE_NAMES with respect to LopsinInstType's");
const char * const LOPSIN_INST_TYPE_NAMES[COUNT_LOPSIN_INST_TYPES] = {
    [LOPSIN_INST_NOP]           = "nop",

    [LOPSIN_INST_PUSH]          = "push",
    [LOPSIN_INST_DROP]          = "drop",
    [LOPSIN_INST_DUP]           = "dup",
    [LOPSIN_INST_SWAP]          = "swap",
    
    [LOPSIN_INST_SUM]           = "sum",
    [LOPSIN_INST_SUB]           = "sub",
    [LOPSIN_INST_MUL]           = "mul",
    [LOPSIN_INST_DIV]           = "div",
    [LOPSIN_INST_MOD]           = "mod",

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

    [LOPSIN_INST_DUMP]          = "dump",
    [LOPSIN_INST_PUTC]          = "putc",
};

static_assert(COUNT_LOPSIN_ERRS == 6, "Exhaustive definition of LOPSIN_ERR_NAMES with respct to LopsinErr's");
const char * const LOPSIN_ERR_NAMES[COUNT_LOPSIN_ERRS] = {
    [ERR_OK]              = "OK",

    [ERR_STACK_UNDERFLOW] = "Stack underflow",
    [ERR_STACK_OVERFLOW]  = "Stack overflow",

    [ERR_ILLEGAL_INST]    = "Illegal instruction",
    [ERR_BAD_INST_PTR]    = "Bad instruction pointer",

    [ERR_DIV_BY_ZERO]     = "Division by zero",
};

static void lopvm_dump_stack(FILE *stream, const LopsinVM *vm)
{
    fprintf(stream, 
        "Stack pointer: %zu\n"
        "Inst pointer:  %zu\n"
        "Stack: \n",
        
        vm->sp, 
        vm->ip);

    for (size_t i = 0; i < vm->sp; i++) {
        fprintf(stream, "\t%"PRId64"\n", vm->stack[i]);
    }
}

LopsinErr lopsinvm_run_inst(LopsinVM *vm)
{
    static_assert(COUNT_LOPSIN_INST_TYPES == 22, "Exhaustive handling of LopsinInstType's in lopsinvm_run_inst()");
    if (vm->ip >= vm->program_sz) {
        return ERR_BAD_INST_PTR;
    }
    LopsinInst inst = vm->program[vm->ip];
    
    if (vm->debug_mode) {
        fprintf(stdout, "Current instruction: %s %"PRId64"\n", 
            LOPSIN_INST_TYPE_NAMES[inst.type], 
            inst.operand);

        lopvm_dump_stack(stdout, vm);
    }

    switch (inst.type) {
    
    case LOPSIN_INST_NOP: {
        vm->ip++;
    } break;

    case LOPSIN_INST_PUSH: {
        if (vm->sp >= vm->stack_cap) {
            return ERR_STACK_OVERFLOW;
        }
        vm->stack[vm->sp++] = inst.operand;
        vm->ip++;
    } break;

    case LOPSIN_INST_DROP: {
        vm->sp--;
        vm->ip++;
    } break;

    case LOPSIN_INST_DUP: {
        if (vm->sp >= vm->stack_cap) {
            return ERR_STACK_OVERFLOW;
        }
        vm->stack[vm->sp] = vm->stack[vm->sp - 1];
        vm->sp++;
        vm->ip++;
    } break;

    case LOPSIN_INST_SWAP: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }
    
        LopsinValue temp = vm->stack[vm->sp - 1];
        vm->stack[vm->sp - 1] = vm->stack[vm->sp - 2];
        vm->stack[vm->sp - 2] = temp;

        vm->ip++;
    } break;

    case LOPSIN_INST_SUM: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        LopsinValue a = vm->stack[--vm->sp];
        vm->stack[vm->sp - 1] += a;
        vm->ip++;
    } break;

    case LOPSIN_INST_SUB: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        LopsinValue a = vm->stack[--vm->sp];
        vm->stack[vm->sp - 1] -= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_MUL: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        LopsinValue a = vm->stack[--vm->sp];
        vm->stack[vm->sp - 1] *= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_DIV: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        LopsinValue a = vm->stack[--vm->sp];
        if (a == 0) {
            return ERR_DIV_BY_ZERO;
        }
        vm->stack[vm->sp - 1] /= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_MOD: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }
        LopsinValue a = vm->stack[--vm->sp];
        if (a == 0) {
            return ERR_DIV_BY_ZERO;
        }
        vm->stack[vm->sp - 1] %= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_GT: {
        if (vm->sp < 2) return ERR_STACK_UNDERFLOW;
        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];
        vm->stack[vm->sp++] = b > a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LT: {
        if (vm->sp < 2) return ERR_STACK_UNDERFLOW;
        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];
        vm->stack[vm->sp++] = b < a;
        vm->ip++;
    } break;

    case LOPSIN_INST_GTE: {
        if (vm->sp < 2) return ERR_STACK_UNDERFLOW;
        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];
        vm->stack[vm->sp++] = b >= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LTE: {
        if (vm->sp < 2) return ERR_STACK_UNDERFLOW;
        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];
        vm->stack[vm->sp++] = b <= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_EQ: {
        if (vm->sp < 2) return ERR_STACK_UNDERFLOW;
        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];
        vm->stack[vm->sp++] = b == a;
        vm->ip++;
    } break;

    case LOPSIN_INST_NEQ: {
        if (vm->sp < 2) return ERR_STACK_UNDERFLOW;
        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];
        vm->stack[vm->sp++] = b != a;
        vm->ip++;
    } break;

    case LOPSIN_INST_JMP: {
        vm->ip = inst.operand;
    } break;

    case LOPSIN_INST_CJMP: {
        if (vm->sp <= 0) {
            return ERR_STACK_UNDERFLOW;
        }
        if (vm->stack[vm->sp - 1]) {
            vm->ip = inst.operand;
        } else {
            vm->ip++;
        }
    } break;

    case LOPSIN_INST_RJMP: {
        vm->ip += inst.operand;
    } break;

    case LOPSIN_INST_CRJMP: {
        if (vm->sp <= 0) {
            return ERR_STACK_UNDERFLOW;
        }
        if (vm->stack[vm->sp - 1]) {
            vm->ip += inst.operand;
        } else {
            vm->ip++;
        }
    } break;

    case LOPSIN_INST_DUMP: {
        if (vm->sp <= 0) {
            return ERR_STACK_UNDERFLOW;
        }
        printf("%"PRId64"\n", vm->stack[--vm->sp]);
        vm->ip++;
    } break;

    case LOPSIN_INST_PUTC: {
        if (vm->sp <= 0) {
            return ERR_STACK_UNDERFLOW;
        }
        printf("%c", (char) vm->stack[--vm->sp]);
        vm->ip++;
    } break;

    default: {
        return ERR_ILLEGAL_INST;
    }
    }
    
    return ERR_OK;
}
