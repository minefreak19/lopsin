#include "./lopsinvm.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "util.h"

static_assert(COUNT_LOPSIN_INST_TYPES == 32, "Exhaustive definition of LOPSIN_INST_TYPE_NAMES with respect to LopsinInstType's");
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

    [LOPSIN_INST_DUMP]          = "dump",
    [LOPSIN_INST_PUTC]          = "putc",
};

static_assert(COUNT_LOPSIN_ERRS == 8, "Exhaustive definition of LOPSIN_ERR_NAMES with respct to LopsinErr's");
const char * const LOPSIN_ERR_NAMES[COUNT_LOPSIN_ERRS] = {
    [ERR_OK]              = "OK",

    [ERR_STACK_UNDERFLOW] = "Stack underflow",
    [ERR_STACK_OVERFLOW]  = "Stack overflow",

    [ERR_ILLEGAL_INST]    = "Illegal instruction",
    [ERR_BAD_INST_PTR]    = "Bad instruction pointer",
    [ERR_HALTED]          = "Already halted",
    [ERR_INVALID_OPERAND]     = "Invalid operand for operation",

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
    static_assert(COUNT_LOPSIN_INST_TYPES == 32, "Exhaustive handling of LopsinInstType's in lopsinvm_run_inst()");
    
    if (!vm->running) {
        return ERR_HALTED;
    }

    if (vm->ip >= vm->program.count) {
        return ERR_BAD_INST_PTR;
    }
    LopsinInst inst = vm->program.insts[vm->ip];
    
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

    case LOPSIN_INST_HLT: {
        vm->running = false;
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
        if (inst.operand <= 0) {
            return ERR_INVALID_OPERAND;
        }

        if (vm->sp + inst.operand >= vm->stack_cap) {
            return ERR_STACK_OVERFLOW;
        }

        if ((LopsinValue) vm->sp < inst.operand) {
            return ERR_STACK_UNDERFLOW;
        }

        size_t saved_sp = vm->sp;
        for (LopsinValue i = 0; i < inst.operand; i++) {
            vm->stack[vm->sp++] = vm->stack[saved_sp - (inst.operand - i)];
        }

        vm->ip++;
    } break;

    case LOPSIN_INST_SWAP: {
        if (inst.operand <= 0) {
            return ERR_INVALID_OPERAND;
        }

        if (vm->sp < (2 * (size_t)inst.operand)) {
            return ERR_STACK_UNDERFLOW;
        }

        size_t swap_sz = inst.operand * sizeof(LopsinValue);
        LopsinValue *temp = NOTNULL(malloc(swap_sz));
        memcpy(temp, &vm->stack[vm->sp - (2 * inst.operand)], swap_sz);
        memcpy(&vm->stack[vm->sp - (2 * inst.operand)], &vm->stack[vm->sp - inst.operand], swap_sz);
        memcpy(&vm->stack[vm->sp - inst.operand], temp, swap_sz);


        // LopsinValue temp = vm->stack[vm->sp - 1];
        // vm->stack[vm->sp - 1] = vm->stack[vm->sp - 2];
        // vm->stack[vm->sp - 2] = temp;

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

    case LOPSIN_INST_SHL: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = b << a;
        vm->ip++;
    } break;

    case LOPSIN_INST_SHR: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = b >> a;
        vm->ip++;
    } break;

    case LOPSIN_INST_BOR: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = b | a;
        vm->ip++;
    } break;

    case LOPSIN_INST_BAND: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = b & a;
        vm->ip++;
    } break;

    case LOPSIN_INST_XOR: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = b ^ a;
        vm->ip++;
    } break;

    case LOPSIN_INST_BNOT: {
        if (vm->sp < 1) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = ~a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LOR: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = b || a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LAND: {
        if (vm->sp < 2) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];
        LopsinValue b = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = b && a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LNOT: {
        if (vm->sp < 1) {
            return ERR_STACK_UNDERFLOW;
        }

        LopsinValue a = vm->stack[--vm->sp];

        vm->stack[vm->sp++] = !a;
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
        .ip = 0,
        .program = {
            .insts = NULL,
            .count = 0,
            .cap = 0,
        },
        .running = false,
        .sp = 0,
        .stack = NOTNULL(calloc(LOPSINVM_DEFAULT_STACK_CAP, sizeof(LopsinValue))),
        .stack_cap = LOPSINVM_DEFAULT_STACK_CAP,
    };
}

void lopsinvm_load_program_from_memory(LopsinVM *vm, LopsinInst *program, size_t program_sz)
{
    vm->program.insts = NOTNULL(malloc(program_sz * sizeof(LopsinInst)));
    vm->program.count = program_sz;
    memcpy(vm->program.insts, program, program_sz * sizeof(LopsinInst));
}

void lopsinvm_load_program_from_file(LopsinVM *vm, const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", 
                path, strerror(errno));
        exit(1);
    }

    if (fseek(file, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: Could not read file %s: %s\n",
            path, strerror(errno));
        fclose(file);
        exit(1);
    }

    long len = ftell(file);

    if (len < 0) {
        fprintf(stderr, "ERROR: Could not read file %s: %s\n",
            path, strerror(errno));

        fclose(file);
        exit(1);
    }

    if (fseek(file, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file %s: %s\n",
            path, strerror(errno));
        fclose(file);
        exit(1);
    }

    LopsinInst *contents = NOTNULL(malloc(len));
    if (fread(contents, 1, len, file) < (size_t)len) {
        if (ferror(file)) {
            fprintf(stderr, "ERROR: Could not read file %s: %s\n",
                path, strerror(errno));
        } else if (feof(file)) {
            fprintf(stderr, "ERROR: Could not read file %s: %s\n",
                    path, "Reached end of file");
        } else {
            assert(false && "Unreachable");
        }
        fclose(file);
        free(contents);
        exit(1);
    }

    fclose(file);

    size_t count = len / sizeof(LopsinInst);
    lopsinvm_load_program_from_memory(vm, contents, count);

    free(contents);
}
