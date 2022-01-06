#include "lopsinvm.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

static void usage(FILE *stream, const char *program)
{
    fprintf(stream, "USAGE: %s <input.lopsinvm>\n", program);
}

int main(int argc, const char **argv)
{
    (void) argc;

    assert(*argv != NULL);

    const char *program_name = *argv++;
    
    const char *input_file = *argv++;
    if (input_file == NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: No input file provided\n");
        exit(1);
    }

    LopsinVM vm = lopsinvm_new();

    lopsinvm_load_program_from_file(&vm, input_file);

    lopsinvm_start(&vm);

    return 0;
}
