/////////////////
// HEADER PART //
/////////////////

#ifndef BUFFER_H_
#define BUFFER_H_

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

BUFFERDEF Buffer *new_buffer(size_t cap);
BUFFERDEF void buffer_clear(Buffer *);
BUFFERDEF void buffer_free(Buffer *);
BUFFERDEF void buffer_append_char(Buffer *, char);
BUFFERDEF void buffer_append_str(Buffer *, const char *str, size_t len);
BUFFERDEF void buffer_append_cstr(Buffer *, const char *cstr);
BUFFERDEF void buffer_append_fmt(Buffer *, const char *format, ...) PRINTF_FORMAT(2, 3);
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

static void buffer_ensure(Buffer *buf, size_t req)
{
    if (buf->cap >= req) return;

    buf->data = NOTNULL(realloc(buf->data, req));
}

BUFFERDEF Buffer *new_buffer(size_t cap)
{
    if (cap == 0) cap = DEFAULT_TMP_BUF_CAP;

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
        PANIC("Attempted to free non-empty buffer. buffer_clear() should be called first.\n");
    }

    free(buf->data);
    free(buf);
}

BUFFERDEF void buffer_append_char(Buffer *buf, char c)
{
    buffer_ensure(buf, buf->size + 1);
    buf->data[buf->size++] = c;
}

BUFFERDEF void buffer_append_str(Buffer *buf, const char *str, size_t len)
{
    assert(str != NULL);
    assert(buf != NULL);

    if (len == 0) return;
    
    buffer_ensure(buf, buf->size + len);

    memcpy(&buf->data[buf->size], str, len);
    buf->size += len;
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

BUFFERDEF void buffer_rewind(Buffer * buf, size_t prev_sz)
{
    assert(buf != NULL);
    assert(prev_sz <= buf->size);
    
    buf->size = prev_sz;
}

#endif // BUFFER_IMPLEMENTATION
