#include "lopsinvm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define cstreq(a, b) (strcmp(a, b) == 0)

static void usage(FILE *stream, const char *program)
{
    fprintf(stream, "USAGE: %s <input.lopsinvm> [OPTIONS]\n", program);
    fprintf(stream, 
        "OPTIONS:\n"
        "   --debug, -d             Enable debug mode\n"
        "   --help,  -h             Display this help and exit\n");
}

int main(int argc, const char **argv)
{
    (void) argc;

    assert(*argv != NULL);

    const char *program_name = *argv++;

    struct {
        const char *input_file;
        bool debug_mode;
    } args = {0};

    while (*argv != NULL) {
        const char *arg = *argv++;
        
        if (cstreq(arg, "--help") || cstreq(arg, "-h")) {
            usage(stdout, program_name);
            exit(0);
        } else if (cstreq(arg, "--debug") || cstreq(arg, "-d")) {
            args.debug_mode = true;
        } else {
            // throw error if we already have an input file
            if (args.input_file != NULL) {
                usage(stderr, program_name);
                fprintf(stderr, "ERROR: Unknown option `%s`\n", arg);
                exit(1);
            }

            args.input_file = arg;
        }
    }

    if (args.input_file == NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: No input file provided\n");
        exit(1);
    }
    LopsinVM vm = lopsinvm_new();
    vm.debug_mode = args.debug_mode;

    lopsinvm_load_program_from_file(&vm, args.input_file);

    LopsinErr errlvl = lopsinvm_start(&vm);

    return errlvl;
}
