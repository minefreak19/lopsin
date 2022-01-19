/////////////////
// HEADER PART //
/////////////////

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdbool.h>
#include <stddef.h>

#include "./util.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define DEFAULT_BUFFER_CAP 1024

#ifndef BUFFERDEF
# define BUFFERDEF
#endif

typedef struct Buffer {
    char *data;
    size_t size;
    size_t cap;
} Buffer;

BUFFERDEF void buffer_ensure(Buffer *, size_t);
BUFFERDEF Buffer *new_buffer(size_t cap);
BUFFERDEF void buffer_clear(Buffer *);
BUFFERDEF void buffer_free(Buffer *);
BUFFERDEF void buffer_append_char(Buffer *, char);
BUFFERDEF void buffer_append_bytes(Buffer *, void *ptr, size_t bytes);
BUFFERDEF void buffer_append_str(Buffer *, const char *str, size_t len);
BUFFERDEF void buffer_append_cstr(Buffer *, const char *cstr);
BUFFERDEF void buffer_append_fmt(Buffer *, const char *format, ...) PRINTF_FORMAT(2, 3);
BUFFERDEF void buffer_append_file(Buffer *, const char *path);
BUFFERDEF void buffer_rewind(Buffer *, size_t prev_sz);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BUFFER_H_ */

/////////////////
// SOURCE PART //
/////////////////
#ifdef BUFFER_IMPLEMENTATION
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

BUFFERDEF void buffer_ensure(Buffer *buf, size_t req)
{
    if (buf->cap >= req) return;

    buf->data = NOTNULL(realloc(buf->data, req));
}

BUFFERDEF Buffer *new_buffer(size_t cap)
{
    if (cap == 0) cap = DEFAULT_BUFFER_CAP;

    Buffer *result = NOTNULL(malloc(sizeof(Buffer)));

    *result = (Buffer) {
        .cap = cap,
        .data = NOTNULL(calloc(cap, sizeof(char))),
        .size = 0,
    };

    return result;
}

BUFFERDEF void buffer_clear(Buffer *buf) 
{
    assert(buf != NULL);
    buf->size = 0;
}

BUFFERDEF void buffer_free(Buffer *buf)
{
    assert(buf != NULL);

    if (buf->size > 0) {
        fprintf(stderr, "Attempted to free non-empty buffer. buffer_clear() should be called first.\n");
        exit(1);
    }

    free(buf->data);
    free(buf);
}

BUFFERDEF void buffer_append_char(Buffer *buf, char c)
{
    buffer_ensure(buf, buf->size + 1);
    buf->data[buf->size++] = c;
}

BUFFERDEF void buffer_append_bytes(Buffer *buf, void *ptr, size_t bytes)
{
    assert(buf != NULL);
    assert(ptr != NULL);

    if (bytes == 0) return;

    buffer_ensure(buf, buf->size + 1);

    memcpy(&buf->data[buf->size], ptr, bytes);
    buf->size += bytes;
}

BUFFERDEF void buffer_append_str(Buffer *buf, const char *str, size_t len)
{
    return buffer_append_bytes(buf, str, len);
}

BUFFERDEF void buffer_append_cstr(Buffer *buf, const char *cstr)
{
    assert(cstr != NULL);
    buffer_append_str(buf, cstr, strlen(cstr));
}

BUFFERDEF void buffer_append_fmt(Buffer *buf, const char *format, ...)
{
    assert(buf != NULL);
    assert(format != NULL);

    va_list vargs;
    va_start(vargs, format);

    // grab an estimate
    size_t len = vsnprintf(NULL, 0, format, vargs); 

    va_end(vargs);

    char *str = NOTNULL(malloc(len * sizeof(char)));

    va_start(vargs, format);
    // actually do the thing
    vsnprintf(str, len + 1, format, vargs);

    buffer_append_cstr(buf, str);

    free(str);
    va_end(vargs);
}

BUFFERDEF void buffer_append_file(Buffer *buf, const char *file_path)
{
    FILE *infile = fopen(file_path, "rb");

    if (infile == NULL) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n",
            file_path, strerror(errno));
        exit(1);
    }

    if (fseek(infile, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: could not seek in file %s: %s\n",
            file_path, strerror(errno));
        fclose(infile);
        exit(1);
    }
    int len = ftell(infile);

    if (len < 0) {
        fprintf(stderr, "ERROR: Could not obtain file size for %s: %s\n",
            file_path, strerror(errno));

        fclose(infile);
        exit(1);
    }

    if (fseek(infile, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: could not seek in file %s: %s\n",
            file_path, strerror(errno));
        fclose(infile);
        exit(1);
    }

    char *contents = NOTNULL(malloc((len) * sizeof(char)));
    if (fread(contents, sizeof(char), len, infile) == 0) {
        if (ferror(infile)) {
            fprintf(stderr, "ERROR: Could not read file %s: %s\n",
                file_path, strerror(errno));
        } else if (feof(infile)) {
            fprintf(stderr, "ERROR: Could not read file %s: %s\n",
                    file_path, "Reached end of file");
        } else {
            assert(false && "Unreachable");
        }
        fclose(infile);
        free(contents);
        exit(1);
    }

    buffer_append_str(buf, contents, len);

    free(contents);
    fclose(infile);
}

BUFFERDEF void buffer_rewind(Buffer * buf, size_t prev_sz)
{
    assert(buf != NULL);
    assert(prev_sz <= buf->size);
    
    buf->size = prev_sz;
}

#endif // BUFFER_IMPLEMENTATION
