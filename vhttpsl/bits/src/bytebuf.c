#include <vhttpsl/bits/bytebuf.h>

#include <string.h>

struct bytebuf_s *
bytebuf_create(size_t start_size)
{
	struct bytebuf_s *result = calloc(1, sizeof(struct bytebuf_s));

	bytebuf_init(result, start_size);

	return result;
}

int
bytebuf_init(struct bytebuf_s *buf, size_t start_size)
{
	if (buf == NULL)
		return EXIT_FAILURE;

	buf->pos_read = 0;
	buf->pos_write = 0;

	buf->data = malloc(start_size);

	if (buf->data == NULL)
		return EXIT_FAILURE;

	buf->size = start_size;

	return EXIT_SUCCESS;
}

struct bytebuf_s *
bytebuf_from_pointer(char *ptr, size_t len)
{
	struct bytebuf_s *result;

	result = bytebuf_create(len);

	if (result != NULL)
	{
		memcpy(result->data, ptr, len);
	}

	return result;
}

void
bytebuf_destroy(struct bytebuf_s *buf)
{
	if (buf == NULL)
		return;
	free(buf->data);
	buf->data = NULL;
}

void
bytebuf_free(struct bytebuf_s **buf)
{
	if (buf == NULL || *buf == NULL)
		return;
	bytebuf_destroy(*buf);
	free(*buf);
	*buf = NULL;
}

void
bytebuf_ensure(struct bytebuf_s *buf, size_t size)
{
	while (buf->size < size)
	{
		buf->size *= 2;
		buf->data = realloc(buf->data, buf->size);
	}
}

char *
bytebuf_write_ptr(struct bytebuf_s *buf)
{
	return buf->data + buf->pos_write;
}

void
bytebuf_ensure_write(struct bytebuf_s *buf, size_t size)
{
	bytebuf_ensure(buf, buf->pos_write + size);
}

size_t
bytebuf_write_size(struct bytebuf_s *buf)
{
	return buf->size - buf->pos_write;
}

const char *
bytebuf_read_ptr(struct bytebuf_s *buf)
{
	return buf->data + buf->pos_read;
}
