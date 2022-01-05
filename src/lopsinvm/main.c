#include "lopsinvm.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

#define LOPSINVM_STACK_CAP 1024 

LopsinInst program[] = {
    MAKE_INST(PUSH, 0),
    MAKE_INST(PUSH, '\n'),
    MAKE_INST(PUSH, '!'),
    MAKE_INST(PUSH, 'd'),
    MAKE_INST(PUSH, 'l'),
    MAKE_INST(PUSH, 'r'),
    MAKE_INST(PUSH, 'o'),
    MAKE_INST(PUSH, 'W'),
    MAKE_INST(PUSH, ' '),
    MAKE_INST(PUSH, ','),
    MAKE_INST(PUSH, 'o'),
    MAKE_INST(PUSH, 'l'),
    MAKE_INST(PUSH, 'l'),
    MAKE_INST(PUSH, 'e'),
    MAKE_INST(PUSH, 'H'),

    MAKE_INST(PUTC, 0),
    MAKE_INST(CRJMP, -1),
};

int main(int argc, const char **argv)
{
    (void) argc;

    assert(*argv != NULL);

    const char *program_name = *argv++;
    (void) program_name;
    
    LopsinVM vm = {
        .ip         = 0,
        .program    = program,
        .program_sz = ARRAY_LEN(program),
        .stack      = NOTNULL(calloc(LOPSINVM_STACK_CAP, sizeof(LopsinValue))),
        .stack_cap  = LOPSINVM_STACK_CAP,
        .sp         = 0,
        .debug_mode = false,
    };

    LopsinErr err = 0;

    while (vm.ip < vm.program_sz) {
        err = lopsinvm_run_inst(&vm);
        if (err) {
            fprintf(stderr, "ERROR: At inst %zu: %s\n", vm.ip, ERR_AS_CSTR(err));
            return 1;
        }
    }
    return 0;
}
