//
// Created by victoria on 21.06.2020.
//

#include <vhttpsl/bits/minmax.h>

#include "session.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <vhttpsl/bits/kv.h>

int
http_session_init(http_session_t session)
{
	memset(session, 0, sizeof(*session));

	bytebuf_init(&session->buf_in, 256);
	bytebuf_init(&session->buf_out, 256);
	headers_read_begin(&session->headers_read_state);

	return EXIT_SUCCESS;
}

void
http_session_destroy(http_session_t session)
{
	headers_read_end(&session->headers_read_state);
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
	size_t i = 0;
	int consume;
	int retval;
	size_t remaining;
	struct http_session_read_state_s s = session->state;

	while (i < size && s.step != SWS_ERROR)
	{
		consume = 1;

		switch (s.step)
		{
			case SWS_REQUEST_BEGIN:
				consume = 0;
				session->request = calloc(1, sizeof(*session->request));
				s.step = SWS_REQUEST_LINE;
				s.segment_length = 0;
				break;
			case SWS_REQUEST_LINE:
				if (buf[i] == '\r')
				{
					s.step = SWS_CR;
					s.next_step = SWS_HEADERS_BEGIN;
					consume = 0;
				}
				else
					++s.segment_length;
				break;
			case SWS_CR:
				if (buf[i] == '\r')
					s.step = SWS_LF;
				else
					s.step = SWS_ERROR;
				break;
			case SWS_LF:
				if (buf[i] == '\n')
					s.step = s.next_step;
				else
					s.step = SWS_ERROR;
				break;
			case SWS_HEADERS_BEGIN:
				headers_read_reset(&session->headers_read_state);
				session->request->headers = kv_create();
				s.step = SWS_HEADERS;
				consume = 0;
				break;
			case SWS_HEADERS:
				consume = 0;
				retval = headers_read(buf + i, (int) (size - i), &session->headers_read_state,
									  session->request->headers);
				if (!retval)
					s.step = SWS_BODY_BEGIN;
				else if (retval < 0)
					s.step = SWS_ERROR;
				else
					i += retval;
				break;
			case SWS_BODY_BEGIN:
				session->request->length = (int) headers_get_content_length(session->request->headers);
				if (session->request->length <= 0)
					s.step = SWS_REQUEST_END;
				else
				{
					s.segment_length = 0;
					if (session->request->length < 1024 * 1024)
						session->request->body = calloc(session->request->length, 1);
					else
						s.step = SWS_ERROR;
					s.step = SWS_BODY;
				}
				consume = 0;
				break;
			case SWS_BODY:
				consume = 0;
				remaining = MIN(size - i, session->request->length - s.segment_length);
				memcpy(session->request->body + s.segment_length, buf + i, remaining);
				s.segment_length += remaining;
				if (s.segment_length == session->request->length)
					s.step = SWS_REQUEST_END;
				break;
			case SWS_REQUEST_END:
				consume = 0;
				s.step = SWS_CR;
				s.next_step = SWS_REQUEST_BEGIN;
				printf("finished request\n");
				break;
			default:
				break;
		}

		if (consume)
			++i;
	}

	session->state = s;

	bytebuf_ensure_write(&session->buf_out, size);
	ptr = bytebuf_write_ptr(&session->buf_out);

	session->buf_out.pos_write += size;
	memcpy(ptr, buf, size);

	return size;
}
