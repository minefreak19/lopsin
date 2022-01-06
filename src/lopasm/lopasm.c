#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SVDEF static inline
#define SV_IMPLEMENTATION
#include <sv.h>

#define BUFFERDEF static inline
#define BUFFER_IMPLEMENTATION
#include <buffer.h>

#include "util.h"
#include "../lopsinvm/lopsinvm.h"

static void usage(FILE *stream, const char *program)
{
    fprintf(stream, "USAGE: %s <input.lopasm> <output.lopsinvm>\n", program);
}

static bool needs_operand(LopsinInstType inst_type)
{
    switch (inst_type) {

    case LOPSIN_INST_PUSH: 
    case LOPSIN_INST_DUP: 
    case LOPSIN_INST_SWAP: 
    case LOPSIN_INST_JMP: 
    case LOPSIN_INST_CJMP: 
    case LOPSIN_INST_RJMP: 
    case LOPSIN_INST_CRJMP: 
        return true;

    default: return false;
    }
}

static bool parse_inst(String_View *source, LopsinInst *out)
{
    LopsinInst result = {0};
    String_View inst = sv_trim(sv_chop_by_delim(source, ' '));

    for (LopsinInstType i = 0; i < COUNT_LOPSIN_INST_TYPES; i++) {
        String_View inst_name = sv_from_cstr(LOPSIN_INST_TYPE_NAMES[i]);

        if (sv_eq(inst, inst_name)) {
            result.type = i;
            break;
        }
    }

    if (needs_operand(result.type)) {
        String_View operand = sv_trim(sv_chop_by_delim(source, ' '));
        char *cstr = NOTNULL(malloc((operand.count + 1) * sizeof(char)));
        memcpy(cstr, operand.data, operand.count * sizeof(char));
        cstr[operand.count] = '\0';
        result.operand = strtoll(cstr, NULL, 0);
        free(cstr);
    }

    if (out) *out = result;
    return true;
}

#define PROGRAM_CAP 1024 * 1024

int main(int argc, const char **argv)
{
    (void) argc;

    size_t prog_sz = 0;
    LopsinInst *prog = NOTNULL(calloc(PROGRAM_CAP, sizeof(LopsinInst)));

    assert(*argv != NULL);
    const char *program = *argv++;

    const char *input_path = *argv++;
    if (input_path == NULL) {
        usage(stderr, program);
        fprintf(stderr, "ERROR: No input file provided\n");
        exit(1);
    }

    const char *output_path = *argv++;
    if (output_path == NULL) {
        usage(stderr, program);
        fprintf(stderr, "ERROR: No output file provided\n");
        exit(1);
    }

    String_View input;
    Buffer *input_buf = new_buffer(0);

    buffer_append_file(input_buf, input_path);
    input = sv_from_parts(input_buf->data, input_buf->size);

    String_View line = {0};
    do {
        line = sv_trim_right(sv_chop_by_delim(&input, '\n'));
        assert(prog_sz < PROGRAM_CAP);
        parse_inst(&line, &prog[prog_sz++]);
    } while (input.count > 0);

    buffer_clear(input_buf);
    buffer_free(input_buf);

    FILE *outfile = fopen(output_path, "wb");
    if (outfile == NULL) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", 
                output_path, strerror(errno));
        exit(1);
    }

    if (fwrite(prog, sizeof(LopsinInst), prog_sz, outfile) < prog_sz) {
        fprintf(stderr, "ERROR: Could not write to file %s: %s\n",
                output_path, strerror(errno));
        exit(1);
    }
    
    fclose(outfile);

    return 0;
}
