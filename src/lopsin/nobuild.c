#define NOBUILD_IMPLEMENTATION
#include "../../nobuild.h"
#include "../../nobuild.common.h"

#include <string.h>

#define MODULE "lopsin"
#define OUTFILE PATH(BINDIR, MODULE)

#define MODULE_BUILD_CXXFLAGS BUILD_CXXFLAGS
#define MODULE_DEBUG_CXXFLAGS DEBUG_CXXFLAGS
#define MODULE_INCLUDES C_INCLUDES

#define EXTRA_SRCFILES  PATH(SRCDIR, "common", "util.c")

int main(int argc, const char **argv)
{
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

    INFO("Building module: \033[36;1m%s\033[0m", MODULE);
    Cstr srcpath = PATH(SRCDIR, MODULE);

    assert(argc >= 2);

    Mode mode = 0;

    if (strcmp(argv[1], "build") == 0) {
        mode = MODE_BUILD;
    } else if (strcmp(argv[1], "debug") == 0) {
        mode = MODE_DEBUG;
    } else {
        WARN("No mode specified. Using default mode.");
    }

    Cstr_Array cmdarr = {0};

    if (CC == NULL) {
        ERRO("CC environment variable undeclared.\n");
    }

    if (CXX == NULL) {
        ERRO("CXX environment variable undeclared.\n");
    }

    cmdarr = cstr_array_append(cmdarr, CXX);
    cmdarr = cstr_array_append(cmdarr, "-o");
    cmdarr = cstr_array_append(cmdarr, OUTFILE);

    Cstr_Array cflags;

    switch (mode) {
    
    case MODE_BUILD: {
        cflags = cstr_array_make(MODULE_BUILD_CXXFLAGS, MODULE_INCLUDES, NULL);
    } break;

    case MODE_DEBUG: {
        cflags = cstr_array_make(MODULE_DEBUG_CXXFLAGS, MODULE_INCLUDES, NULL);
    } break;

    }

    FOREACH_ARRAY(Cstr, cflag, cflags, {
        cmdarr = cstr_array_append(cmdarr, *cflag);
    });


    Cstr_Array srcfiles = 
#   ifdef EXTRA_SRCFILES
        cstr_array_make(EXTRA_SRCFILES, NULL);
#   else
        {0};
#   endif

    FOREACH_FILE_IN_DIR(srcfile, srcpath, {
        if (!(IS_DIR(srcfile))
          && (ENDS_WITH(srcfile, ".c")
              || ENDS_WITH(srcfile, ".cpp"))) 
        {
            if (strcmp(srcfile, "nobuild.c") != 0) {
                srcfiles = cstr_array_append(srcfiles, PATH(srcpath, srcfile));
            }
        }
    });


    FOREACH_ARRAY(Cstr, srcfile, srcfiles, {
        cmdarr = cstr_array_append(cmdarr, *srcfile);
    });

    Cmd cmd = { cmdarr };
    INFO("CMD: %s", cmd_show(cmd));
    cmd_run_sync(cmd);

    return 0;
}
