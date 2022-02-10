#define NOBUILD_IMPLEMENTATION
#include "./nobuild.h"
#include "./nobuild.common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const char * const MODULES[] = {
    "lopsinvm",
    "lopasm",
};

bool starts_with(Cstr cstr, Cstr prefix)
{
    size_t prefix_len = strlen(prefix);
    if (prefix_len > strlen(cstr)) return false;
    return strncmp(cstr, prefix, prefix_len) == 0;
}

void build_module(Mode mode, const char *module)
{
    Cstr module_nobuild_c = PATH(SRCDIR, module, "nobuild.c");
    Cstr module_nobuild   = PATH(SRCDIR, module, "nobuild");
    
    if (!PATH_EXISTS(module_nobuild)) {
        if (PATH_EXISTS(module_nobuild_c)) {
            CMD(
                CC,
                "-o",
                module_nobuild,
                module_nobuild_c,
                NOBUILD_CFLAGS
            );
        } else {
            ERRO("Could not find nobuild.c for module `%s`", module);
            return;
        }
    }
    CMD(module_nobuild, mode == MODE_BUILD ? "build" : "debug");

    // Cstr_Array compile_cmd_arr = {0};

    // compile_cmd_arr = cstr_array_append(compile_cmd_arr, CC);
    // compile_cmd_arr = cstr_array_append(compile_cmd_arr, "-o");
    // compile_cmd_arr = cstr_array_append(compile_cmd_arr, PATH(BINDIR, module));

    // Cstr srcpath = PATH(SRCDIR, module);
    // Cstr_Array srcfiles = {0};
    // FOREACH_FILE_IN_DIR(srcfile, srcpath, {
    //     if (!(IS_DIR(srcfile)) 
    //       && (ENDS_WITH(srcfile, ".c")))
    //     {
    //         srcfiles = cstr_array_append(srcfiles, PATH(srcpath, srcfile));
    //     }
    // });

    // if (srcfiles.count == 0) {
    //     INFO("No source files found in directory %s. Moving on...", srcpath);
    //     return;
    // }

    // FOREACH_ARRAY(Cstr, srcfile, srcfiles, {
    //     compile_cmd_arr = cstr_array_append(compile_cmd_arr, *srcfile);
    // });

    // Cstr_Array cflags;
    // switch (mode) {
    //     case MODE_BUILD: cflags = cstr_array_make(BUILD_CFLAGS, C_INCLUDES, NULL); break;
    //     case MODE_DEBUG: cflags = cstr_array_make(DEBUG_CFLAGS, C_INCLUDES, NULL); break;

    //     default: {
    //         PANIC("ERROR: Invalid mode");
    //     }
    // }

    // FOREACH_ARRAY(Cstr, cflag, cflags, {
    //     compile_cmd_arr = cstr_array_append(compile_cmd_arr, *cflag);
    // });

    // Cmd compile_cmd = { compile_cmd_arr };
    // INFO("CMD: %s", cmd_show(compile_cmd));
    // cmd_run_sync(compile_cmd);
}

void ensure_dirs(void)
{
    for (size_t i = 0; i < ARRAY_LEN(MODULES); i++) {
        const char * const module = MODULES[i];
        MKDIRS(PATH(SRCDIR, module));
        MKDIRS(PATH(BINDIR));
    }
}

void usage(FILE *stream, const char *program)
{
    fprintf(stream, "USAGE: %s <build|debug|clean>\n", program);
}

int main(int argc, const char **argv)
{
    assert(*argv != NULL);

    GO_REBUILD_URSELF(argc, argv);

    if (is_path1_modified_after_path2("./nobuild.common.h", argv[0])) {
        RENAME(argv[0], CONCAT(argv[0], ".old"));
        REBUILD_URSELF(argv[0], __FILE__);
        Cmd cmd = {
            .line = {
                .elems = (Cstr*) argv,
                .count = argc,
            },
        };
        INFO("CMD: %s", cmd_show(cmd));
        cmd_run_sync(cmd);
        exit(0);
    }
    
    const char *program = *argv++;

    const char *mode_text = *argv++;

    ensure_dirs();

    Mode mode;
    if (mode_text == NULL) {
        mode = 0;
    } else if (strcmp(mode_text, "build") == 0) {
        mode = MODE_BUILD;
    } else if (strcmp(mode_text, "debug") == 0) {
        mode = MODE_DEBUG;
    } else if (strcmp(mode_text, "clean") == 0) {
        INFO("Clean mode. Removing binaries...\n");

        RM(PATH(".", "nobuild.old"));
        for (size_t i = 0; i < ARRAY_LEN(MODULES); i++) {
            Cstr module = MODULES[i];
            Cstr module_path = PATH(SRCDIR, module);

            RM(PATH(BINDIR, module));

            FOREACH_FILE_IN_DIR(file, module_path, {
                if ((strcmp(file, "nobuild") == 0) 
                 || (starts_with(file, "nobuild") && ENDS_WITH(file, ".old")))
                {
                    RM(PATH(module_path, file));
                }
            });
        }

        return 0;
    } else {
        usage(stderr, program);
        fprintf(stderr, "ERROR: Invalid mode `%s`\n", mode_text);
        exit(1);
    }

    for (size_t i = 0; i < ARRAY_LEN(MODULES); i++) {
        const char * const module = MODULES[i];
        build_module(mode, module);
    }

    return 0;
}
