#include "util.h"

#include <stdio.h>
#include <stdlib.h>

void *notnull_impl(const char *file, int line, const char *func, const char *expr, void *ptr)
{
    if (ptr == NULL) {
#ifdef _DEBUG
        fprintf(stderr, "%s:%d: %s: ERROR: `%s` == NULL\n", file, line, func, expr);
#else
        (void) file;
        (void) line;
        (void) func;
        (void) expr;

        fprintf(stderr, "ERROR: Null pointer allocated. Possibly out of memory.\n");
#endif
        exit(1);
    }
    return ptr;
}
