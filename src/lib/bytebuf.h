#pragma once

#include <stdlib.h>

struct bytebuf_s
{
	char *data;
	size_t size;

	size_t pos_write;
	size_t pos_read;
};

struct bytebuf_s *
bytebuf_create(size_t start_size);

int
bytebuf_init(struct bytebuf_s *buf, size_t start_size);

void
bytebuf_destroy(struct bytebuf_s *buf);

void
bytebuf_free(struct bytebuf_s **buf);

void
bytebuf_ensure(struct bytebuf_s *buf, size_t size);

void
bytebuf_ensure_write(struct bytebuf_s *buf, size_t size);

char *
bytebuf_write_ptr(struct bytebuf_s *buf);

const char *
bytebuf_read_ptr(struct bytebuf_s *buf);

size_t
bytebuf_write_size(struct bytebuf_s *buf);

/*
void
bytebuf_putc(struct bytebuf_s *buf, char ch);

size_t
bytebuf_appendf(struct bytebuf_s *buf, const char *fmt, ...);

void
bytebuf_append(struct bytebuf_s *buf, char *ptr, size_t len);

struct bytebuf_s *
bytebuf_from_pointer(char *ptr, size_t len);
*/
