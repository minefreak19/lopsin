#define NOBUILD_IMPLEMENTATION
#include "./nobuild.h"
#include "./nobuild.common.h"

#include <stdio.h>
#include <stdlib.h>

const char * const MODULES[] = {
    "lopsinvm",
    "lopasm",
};

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
    fprintf(stream, "USAGE: %s <build|debug>\n", program);
}

int main(int argc, const char **argv)
{
    assert(*argv != NULL);

    GO_REBUILD_URSELF(argc, argv);
    
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
