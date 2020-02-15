#pragma once

/*

#include "common.h"

struct bytebuf_s
{
    char   *data;
    size_t  size;

    size_t  pos_write;
};

struct bytebuf_s*
bytebuf_alloc(size_t alloc);

struct bytebuf_s*
bytebuf_from_pointer(char *ptr, size_t len);

void
bytebuf_free(struct bytebuf_s *buf);

void
bytebuf_ensure(struct bytebuf_s *buf, size_t size);

char*
bytebuf_write_ptr(struct bytebuf_s *buf);

void
bytebuf_putc(struct bytebuf_s *buf, char ch);

size_t
bytebuf_appendf(struct bytebuf_s *buf, const char *fmt, ...);

void
bytebuf_append(struct bytebuf_s *buf, char *ptr, size_t len);

*/