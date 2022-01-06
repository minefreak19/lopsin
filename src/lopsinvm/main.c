#include "lopsinvm.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"


LopsinInst program[] = {
    MAKE_INST(PUSH, 17),
    MAKE_INST(PUSH, 34),
    MAKE_INST(PUSH, 51),
    
    MAKE_INST(PUSH, 15),
    MAKE_INST(PUSH, 20),
    MAKE_INST(PUSH, 30),
    MAKE_INST(SWAP,  3),
    MAKE_INST(HLT, 0)

    // MAKE_INST(PUSH, '\n'),
    // MAKE_INST(PUSH, '!'),
    // MAKE_INST(PUSH, 'd'),
    // MAKE_INST(PUSH, 'l'),
    // MAKE_INST(PUSH, 'r'),
    // MAKE_INST(PUSH, 'o'),
    // MAKE_INST(PUSH, 'W'),
    // MAKE_INST(PUSH, ' '),
    // MAKE_INST(PUSH, ','),
    // MAKE_INST(PUSH, 'o'),
    // MAKE_INST(PUSH, 'l'),
    // MAKE_INST(PUSH, 'l'),
    // MAKE_INST(PUSH, 'e'),
    // MAKE_INST(PUSH, 'H'),

    // MAKE_INST(PUTC, 0),
    // MAKE_INST(CRJMP, -1),
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
        .stack      = NOTNULL(calloc(LOPSINVM_DEFAULT_STACK_CAP, sizeof(LopsinValue))),
        .stack_cap  = LOPSINVM_DEFAULT_STACK_CAP,
        .sp         = 0,
        .debug_mode = true,
    };

    lopsinvm_start(&vm);

    return 0;
}
