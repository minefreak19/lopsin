#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_IMPLEMENTATION
#include <buffer.h>

#include "./lopasm.h"

static void usage(FILE *stream, const char *program_name)
{
    fprintf(stream, "USAGE: %s <input.lopasm> <output.lopsinvm>\n", 
            program_name);
}

int main(int argc, const char **argv)
{
    assert(argc > 0);

    const char *program_name = *argv++;

    const char *input_path = *argv++;
    if (input_path == NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: no input file provided\n");
        exit(1);
    }

    const char *output_path = *argv++;
    if (output_path == NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: no output file provided\n");
        exit(1);
    }

    Buffer *input_buf = new_buffer(0);
    buffer_append_file(input_buf, input_path);

    String_View input = {
        .count = input_buf->size,
        .data  = input_buf->data,
    };

    LopAsm_Lexer lexer = {
        .loc = {0},
        .source = input,
    };
    LopAsm_Token tok = {0};

    bool success;
    do {
        success = lopasm_lexer_spit_token(&lexer, &tok);

        if (success) {
            lopasm_print_token(stdout, tok);
        }
    } while (success);

    buffer_clear(input_buf);
    buffer_free(input_buf);

    return 0;
}
