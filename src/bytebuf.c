
/*

#include <stdarg.h>
#include <string.h>

struct bytebuf_s*
bytebuf_alloc(size_t alloc)
{
    struct bytebuf_s *result;

    result = calloc(1, sizeof(struct bytebuf_s));

    if (result != NULL)
    {
        result->data = malloc(alloc);
        result->size = alloc;
    }

    return result;
}

struct bytebuf_s*
bytebuf_from_pointer(char *ptr, size_t len)
{
    struct bytebuf_s *result;

    result = bytebuf_alloc(len);

    if (result != NULL)
    {
        memcpy(result->data, ptr, len);
    }

    return result;
}

void
bytebuf_free(struct bytebuf_s *buf)
{
    free(buf->data);
}

void
bytebuf_ensure(struct bytebuf_s *buf, size_t size)
{
    while (buf->size < size)
    {
        buf->size *= 2;
    }
}

char*
bytebuf_write_ptr(struct bytebuf_s *buf)
{
    return buf->data + buf->pos_write;
}

void
bytebuf_putc(struct bytebuf_s *buf, char ch)
{
    bytebuf_ensure(buf, buf->size + 1);
    *bytebuf_write_ptr(buf) = ch;
    ++buf->pos_write;
}

size_t
bytebuf_appendf(struct bytebuf_s *buf, const char *fmt, ...)
{
    va_list list;
    size_t written;
    size_t i;
    char   temp[64];

    va_start(list, fmt);
    written = 0;

    for (i = 0; i < strlen(fmt); ++i)
    {

    }

    va_end(list);
}

void
bytebuf_append(struct bytebuf_s *buf, char *ptr, size_t len)
{
    bytebuf_ensure(buf, buf->size + len);
    memcpy(bytebuf_write_ptr(buf), ptr, len);
    buf->pos_write += len;
}

*/