
#if defined __linux__ || (defined __APPLE__ && defined __MACH__)
#   define _GNU_SOURCE

#   include <sys/wait.h>
#   include <signal.h>
#elif defined _WIN32

#endif // defined(__linux__) || (defined __APPLE__ && defined __MACH__)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define SV_IMPLEMENTATION
#include <sv.h>
// <sv.h> is included by lopasm.h
#undef SV_IMPLEMENTATION

#define BUFFER_IMPLEMENTATION
#include <buffer.h>

#include "./lopasm.h"

static void usage(FILE *stream, const char *program_name)
{
    fprintf(stream, "USAGE: %s <input.lopasm> -o <output.lopsinvm> [OPTIONS]\n",
            program_name);
    fprintf(stream,
        "OPTIONS:\n"
        "   --debug, -d             Enable debugging mode\n"
        "   --help,  -h             Print this help message and exit\n"
        "   --run,   -r             Run program after compilation (requries --vm)\n"
        "   --vm <vm.exe>           Use a shebang pointing to <vm.exe> (this does nothing smart with the working directory, exercise caution)\n"
    );
}

#define cstreq(a, b) (strcmp(a, b) == 0)

int main(int argc, const char **argv)
{
    assert(argc > 0);

    const char *program_name = *argv++;

    struct {
        const char *input_path;
        const char *output_path;

        const char *vm_path;
        bool debug_mode;
        bool run;
    } args = {0};

    while (*argv != NULL) {
        const char *arg = *argv++;

        if (cstreq(arg, "--help") || cstreq(arg, "-h")) {
            usage(stdout, program_name);
            exit(0);
        } else if (cstreq(arg, "-o")) {
            args.output_path = *argv++;
            if (args.output_path == NULL) {
                usage(stderr, program_name);
                fprintf(stderr, "ERROR: trailing `-o` without output file\n");
                exit(1);
            }
        } else if (cstreq(arg, "--debug") || cstreq(arg, "-d")) {
            args.debug_mode = true;
        } else if (cstreq(arg, "--vm")) {
            args.vm_path = *argv++;

            #ifdef _WIN32
            fprintf(stderr, "WARN: shebang is not supported on Windows\n");
            #endif

            if (args.vm_path == NULL) {
                usage(stderr, program_name);
                fprintf(stderr, "ERROR: trailing `--vm` without vm executable\n");
                exit(1);
            }
        } else if (cstreq(arg, "--run") || cstreq(arg, "-r")) {
            args.run = true;
        } else {
            // TODO(#4): lopasm does not support multiple compilation units
            if (args.input_path != NULL) {
                usage(stderr, program_name);
                fprintf(stderr, "ERROR: unrecognised argument `%s`\n", arg);
                exit(1);
            }

            args.input_path = arg;
        }
    }

    if (args.input_path == NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: no input file provided\n");
        exit(1);
    }

    if (args.output_path == NULL) {
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: no output file provided\n");
        exit(1);
    }

    Buffer *input_buf = new_buffer(0);
    buffer_append_file(input_buf, args.input_path);

    String_View input = {
        .count = input_buf->size,
        .data  = input_buf->data,
    };

    // TODO: lopasm has no proper error reporting
    LopAsm_Lexer lexer = {
        .loc = {0},
        .source = input,
    };

    LopAsm_Parser *parser = new_parser();
    LopAsm_Token tok = {0};

    bool success;
    do {
        success = lopasm_lexer_spit_token(&lexer, &tok);

        if (success) {
            if (args.debug_mode) {
                lopasm_print_token(stdout, tok);
            }

            if (!lopasm_parser_accept_token(parser, tok)) {
                fprintf(stderr, "ERROR: parser did not accept token\n");
                exit(1);
            } else {
                if (args.debug_mode) {
                    printf("Parser accepted token.\n");
                }
            }
        }
    } while (success);

    lopasm_parser_next_phase(parser);

    {
        Buffer *output_buf = new_buffer(0);

        if (args.vm_path) {
            buffer_append_cstr(output_buf, "#!");
            buffer_append_cstr(output_buf, args.vm_path);
            buffer_append_char(output_buf, '\n');
        }
        buffer_append_cstr(output_buf, LOPSINVM_BYTECODE_MAGIC);

        LopsinInst inst = {0};
        do {
            success = lopasm_parser_spit_inst(parser, &inst);

            if (success) {
                if (args.debug_mode) {
                    printf("Spit instruction: %s\t",
                           LOPSIN_INST_TYPE_NAMES[inst.type]);

                    if (requires_operand(inst.type)) {
                        lopsinvalue_print(stdout, inst.operand);
                    }

                    printf("\n");
                }

                buffer_append_bytes(output_buf, &inst, sizeof(LopsinInst));
            }
        } while (success);

        buffer_write_to_file(output_buf, args.output_path);

        buffer_clear(output_buf);
        buffer_free(output_buf);
    }

    buffer_clear(input_buf);
    buffer_free(input_buf);

    printf("Compiled %s -> %s successfully.\n", args.input_path, args.output_path);

    if (args.run) {
        if (args.vm_path == NULL) {
            usage(stderr, program_name);
            fprintf(stderr, "ERROR: option `--run` requires `--vm` specified\n");
            exit(1);
        }

        printf("Running program...\n");
        {
            Buffer *cmd_path_buf = new_buffer(0);

            // we're assuming vm_path is properly relative ie ./bin/lopsinvm, not bin/lopsinvm
            buffer_append_cstr(cmd_path_buf, args.vm_path);
            buffer_append_char(cmd_path_buf, ' ');
            buffer_append_cstr(cmd_path_buf, args.output_path);
            buffer_append_char(cmd_path_buf, '\0');

            printf("[CMD] %s\n", cmd_path_buf->data);
            int status = system(cmd_path_buf->data);

#           if defined __linux__ || (defined __APPLE__ && defined __MACH__)
            {
                if (WIFEXITED(status)) {
                    int code = WEXITSTATUS(status);
                    if (code != 0) {
                        printf("Process \033[91;1mexited abnormally\033[0m with code \033[91;1m%d\033[0m.\n",
                            code);
                    } else {
                        printf("Process exited \033[92;1mnormally\033[0m.\n");
                    }
                } else if (WIFSIGNALED(status)) {
                    int signal = WTERMSIG(status);
                    printf("Process terminated by %s\n", strsignal(signal));
                }
            }
#           elif defined _WIN32
            {
                if (status != 0) {
                    printf("Process \033[91;1mexited abnormally\033[0m with code \033[91;1m%d\033[0m.\n",
                           status);
                } else {
                    printf("Process exited \033[92;1mnormally\033[0m.\n");
                }
            }
#           else
#           error posix or windows pls
#           endif

            if (status < 0) {
                fprintf(stderr, "ERROR: Could not execute command `%s`: %s",
                        cmd_path_buf->data, strerror(errno));
                exit(1);
            }

            buffer_clear(cmd_path_buf);
            buffer_free(cmd_path_buf);
        }
    }

    return 0;
}
