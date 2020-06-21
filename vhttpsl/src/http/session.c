//
// Created by victoria on 21.06.2020.
//

#include <vhttpsl/bits/minmax.h>

#include "session.h"

#include <stdlib.h>
#include <memory.h>

int
http_session_init(http_session_t session)
{
	memset(session, 0, sizeof(*session));

	bytebuf_init(&session->buf_in, 256);
	bytebuf_init(&session->buf_out, 256);

	return EXIT_SUCCESS;
}

void
http_session_destroy(http_session_t session)
{
	bytebuf_destroy(&session->buf_in);
	bytebuf_destroy(&session->buf_out);
}

int
http_session_read(http_session_t session, char *buf, size_t size)
{
	const char *ptr = bytebuf_read_ptr(&session->buf_out);
	size_t count = MIN(size, session->buf_out.pos_write - session->buf_out.pos_read);

	if (count)
	{
		session->buf_out.pos_read += count;
		memcpy(buf, ptr, count);
	}

	return count;
}

int
http_session_write(http_session_t session, const char *buf, size_t size)
{
	char *ptr;

	bytebuf_ensure_write(&session->buf_out, size);
	ptr = bytebuf_write_ptr(&session->buf_out);

	session->buf_out.pos_write += size;
	memcpy(ptr, buf, size);

	return size;
}
