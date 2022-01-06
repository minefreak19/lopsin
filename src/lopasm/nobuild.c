#define NOBUILD_IMPLEMENTATION
#include "../../nobuild.h"
#include "../../nobuild.common.h"

#include <string.h>

#define MODULE "lopasm"
#define OUTFILE PATH(BINDIR, MODULE)

#define MODULE_BUILD_CFLAGS BUILD_CFLAGS
#define MODULE_DEBUG_CFLAGS DEBUG_CFLAGS
#define MODULE_INCLUDES C_INCLUDES

#define EXTRA_SRCFILES PATH(SRCDIR, "lopsinvm", "lopsinvm.c")

int main(int argc, const char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    printf("==============================\n");
    printf("Building module: %s\n", MODULE);
    printf("==============================\n");
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

    cmdarr = cstr_array_append(cmdarr, CC);
    cmdarr = cstr_array_append(cmdarr, "-o");
    cmdarr = cstr_array_append(cmdarr, OUTFILE);

    Cstr_Array cflags;

    switch (mode) {
    
    case MODE_BUILD: {
        cflags = cstr_array_make(MODULE_BUILD_CFLAGS, MODULE_INCLUDES, NULL);
    } break;

    case MODE_DEBUG: {
        cflags = cstr_array_make(MODULE_DEBUG_CFLAGS, MODULE_INCLUDES, NULL);
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
          && (ENDS_WITH(srcfile, ".c"))) 
        {
            if (strcmp(srcfile, "nobuild.c") != 0)
                srcfiles = cstr_array_append(srcfiles, PATH(srcpath, srcfile));
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
