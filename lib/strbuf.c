#include "strbuf.h"

#include <stdlib.h>
#include <string.h>

struct strbuf {
    char *buffer;
    int size;
    int end;
};

strbuf_t *strbuf_new()
{
    strbuf_t *buf = calloc(1, sizeof(strbuf_t));
    buf->buffer = calloc(1, sizeof(char));
    buf->size = 1;
    buf->end = 0;

    return buf;
}

void strbuf_delete(strbuf_t *buf)
{
    free(buf->buffer);
    free(buf);
}

void strbuf_reset(strbuf_t *buf)
{
    buf->end = 0;
    memset(buf->buffer, 0, buf->size);
}

void strbuf_append(strbuf_t *buf, char c)
{
    buf->buffer[buf->end++] = c;
    if (buf->end == buf->size) {
        buf->size *= 2;
        buf->buffer = realloc(buf->buffer, buf->size);
        memset(buf->buffer + buf->end, 0, buf->size - buf->end);
    }
}

char *strbuf_buffer(strbuf_t *buf)
{
    return buf->buffer;
}
