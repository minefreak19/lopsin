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

    [LOPSIN_INST_DUMP]          = "dump",
    [LOPSIN_INST_PUTC]          = "putc",
};

static_assert(COUNT_LOPSIN_ERRS == 10, "Exhaustive definition of LOPSIN_ERR_NAMES with respct to LopsinErr's");
const char * const LOPSIN_ERR_NAMES[COUNT_LOPSIN_ERRS] = {
    [ERR_OK]              = "OK",

    [ERR_DSTACK_UNDERFLOW] = "Data stack underflow",
    [ERR_DSTACK_OVERFLOW]  = "Data stack overflow",
    [ERR_RSTACK_UNDERFLOW] = "Return stack underflow",
    [ERR_RSTACK_OVERFLOW]  = "Return stack overflow",

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
        
        vm->dsp, 
        vm->ip);

    for (size_t i = 0; i < vm->dsp; i++) {
        fprintf(stream, "\t%"PRId64"\n", vm->dstack[i]);
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
        if (vm->dsp >= vm->dstack_cap) {
            return ERR_DSTACK_OVERFLOW;
        }
        vm->dstack[vm->dsp++] = inst.operand;
        vm->ip++;
    } break;

    case LOPSIN_INST_DROP: {
        if (vm->dsp < (unsigned) inst.operand) {
            return ERR_DSTACK_UNDERFLOW;
        }

        vm->dsp -= inst.operand;
        vm->ip++;
    } break;

    case LOPSIN_INST_DUP: {
        if (inst.operand <= 0) {
            return ERR_INVALID_OPERAND;
        }

        if (vm->dsp + inst.operand >= vm->dstack_cap) {
            return ERR_DSTACK_OVERFLOW;
        }

        if ((LopsinValue) vm->dsp < inst.operand) {
            return ERR_DSTACK_UNDERFLOW;
        }

        size_t saved_sp = vm->dsp;
        for (LopsinValue i = 0; i < inst.operand; i++) {
            vm->dstack[vm->dsp++] = vm->dstack[saved_sp - (inst.operand - i)];
        }

        vm->ip++;
    } break;

    case LOPSIN_INST_SWAP: {
        if (inst.operand <= 0) {
            return ERR_INVALID_OPERAND;
        }

        if (vm->dsp < (2 * (size_t)inst.operand)) {
            return ERR_DSTACK_UNDERFLOW;
        }

        size_t swap_sz = inst.operand * sizeof(LopsinValue);
        LopsinValue *temp = NOTNULL(malloc(swap_sz));
        memcpy(temp, &vm->dstack[vm->dsp - (2 * inst.operand)], swap_sz);
        memcpy(&vm->dstack[vm->dsp - (2 * inst.operand)], &vm->dstack[vm->dsp - inst.operand], swap_sz);
        memcpy(&vm->dstack[vm->dsp - inst.operand], temp, swap_sz);


        // LopsinValue temp = vm->dstack[vm->dsp - 1];
        // vm->dstack[vm->dsp - 1] = vm->dstack[vm->dsp - 2];
        // vm->dstack[vm->dsp - 2] = temp;

        vm->ip++;
    } break;

    case LOPSIN_INST_SUM: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }
        LopsinValue a = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp - 1] += a;
        vm->ip++;
    } break;

    case LOPSIN_INST_SUB: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }
        LopsinValue a = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp - 1] -= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_MUL: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }
        LopsinValue a = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp - 1] *= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_DIV: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }
        LopsinValue a = vm->dstack[--vm->dsp];
        if (a == 0) {
            return ERR_DIV_BY_ZERO;
        }
        vm->dstack[vm->dsp - 1] /= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_MOD: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }
        LopsinValue a = vm->dstack[--vm->dsp];
        if (a == 0) {
            return ERR_DIV_BY_ZERO;
        }
        vm->dstack[vm->dsp - 1] %= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_SHL: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = b << a;
        vm->ip++;
    } break;

    case LOPSIN_INST_SHR: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = b >> a;
        vm->ip++;
    } break;

    case LOPSIN_INST_BOR: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = b | a;
        vm->ip++;
    } break;

    case LOPSIN_INST_BAND: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = b & a;
        vm->ip++;
    } break;

    case LOPSIN_INST_XOR: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = b ^ a;
        vm->ip++;
    } break;

    case LOPSIN_INST_BNOT: {
        if (vm->dsp < 1) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = ~a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LOR: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = b || a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LAND: {
        if (vm->dsp < 2) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = b && a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LNOT: {
        if (vm->dsp < 1) {
            return ERR_DSTACK_UNDERFLOW;
        }

        LopsinValue a = vm->dstack[--vm->dsp];

        vm->dstack[vm->dsp++] = !a;
        vm->ip++;
    } break;


    case LOPSIN_INST_GT: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;
        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp++] = b > a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LT: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;
        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp++] = b < a;
        vm->ip++;
    } break;

    case LOPSIN_INST_GTE: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;
        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp++] = b >= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_LTE: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;
        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp++] = b <= a;
        vm->ip++;
    } break;

    case LOPSIN_INST_EQ: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;
        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp++] = b == a;
        vm->ip++;
    } break;

    case LOPSIN_INST_NEQ: {
        if (vm->dsp < 2) return ERR_DSTACK_UNDERFLOW;
        LopsinValue a = vm->dstack[--vm->dsp];
        LopsinValue b = vm->dstack[--vm->dsp];
        vm->dstack[vm->dsp++] = b != a;
        vm->ip++;
    } break;

    case LOPSIN_INST_JMP: {
        vm->ip = inst.operand;
    } break;

    case LOPSIN_INST_CJMP: {
        if (vm->dsp <= 0) {
            return ERR_DSTACK_UNDERFLOW;
        }
        if (vm->dstack[vm->dsp - 1]) {
            vm->ip = inst.operand;
        } else {
            vm->ip++;
        }
    } break;

    case LOPSIN_INST_RJMP: {
        vm->ip += inst.operand;
    } break;

    case LOPSIN_INST_CRJMP: {
        if (vm->dsp <= 0) {
            return ERR_DSTACK_UNDERFLOW;
        }
        if (vm->dstack[vm->dsp - 1]) {
            vm->ip += inst.operand;
        } else {
            vm->ip++;
        }
    } break;

    case LOPSIN_INST_CALL: {
        if (vm->rsp >= vm->rstack_cap) return ERR_RSTACK_OVERFLOW;

        vm->rstack[vm->rsp++] = vm->ip + 1;
        vm->ip = inst.operand;
    } break;

    case LOPSIN_INST_RET: {
        if (vm->rsp <= 0) return ERR_RSTACK_UNDERFLOW;

        vm->ip = vm->rstack[--vm->rsp];
    } break;

    case LOPSIN_INST_DUMP: {
        if (vm->dsp <= 0) {
            return ERR_DSTACK_UNDERFLOW;
        }
        printf("%"PRId64"\n", vm->dstack[--vm->dsp]);
        vm->ip++;
    } break;

    case LOPSIN_INST_PUTC: {
        if (vm->dsp <= 0) {
            return ERR_DSTACK_UNDERFLOW;
        }
        printf("%c", (char) vm->dstack[--vm->dsp]);
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

// void lopsinvm_load_program_from_file(LopsinVM *vm, const char *path)
// {
//     FILE *file = fopen(path, "rb");
//     if (file == NULL) {
//         fprintf(stderr, "ERROR: Could not open file %s: %s\n", 
//                 path, strerror(errno));
//         exit(1);
//     }

//     if (fseek(file, 0, SEEK_END) < 0) {
//         fprintf(stderr, "ERROR: Could not read file %s: %s\n",
//             path, strerror(errno));
//         fclose(file);
//         exit(1);
//     }

//     long len = ftell(file);

//     if (len < 0) {
//         fprintf(stderr, "ERROR: Could not read file %s: %s\n",
//             path, strerror(errno));

//         fclose(file);
//         exit(1);
//     }

//     if (fseek(file, 0, SEEK_SET) < 0) {
//         fprintf(stderr, "ERROR: Could not read file %s: %s\n",
//             path, strerror(errno));
//         fclose(file);
//         exit(1);
//     }

//     LopsinInst *contents = NOTNULL(malloc(len));
//     if (fread(contents, 1, len, file) < (size_t)len) {
//         if (ferror(file)) {
//             fprintf(stderr, "ERROR: Could not read file %s: %s\n",
//                 path, strerror(errno));
//         } else if (feof(file)) {
//             fprintf(stderr, "ERROR: Could not read file %s: %s\n",
//                     path, "Reached end of file");
//         } else {
//             assert(false && "Unreachable");
//         }
//         fclose(file);
//         free(contents);
//         exit(1);
//     }

//     fclose(file);

//     size_t count = len / sizeof(LopsinInst);

//     LopsinVMProgram program = {
//         .count = count,
//         .cap   = count,
//     };

//     program.insts = NOTNULL(malloc(count * sizeof(LopsinInst)));

//     memcpy(program.insts, contents, count * sizeof(LopsinInst));

//     vm->program = program;

//     free(contents);
// }
