//
// Created by victoria on 21.06.2020.
//

#pragma once

#include <vhttpsl/bits/bytebuf.h>
#include <vhttpsl/http/request.h>
#include <vhttpsl/http/headers.h>

enum session_write_step
{
	SWS_REQUEST_BEGIN,
	SWS_REQUEST_LINE,
	SWS_CR,
	SWS_LF,
	SWS_HEADERS_BEGIN,
	SWS_HEADERS,
	SWS_BODY_BEGIN,
	SWS_BODY,
	SWS_REQUEST_END,
	SWS_ERROR
};

struct http_session_read_state_s
{
	enum session_write_step step;
	enum session_write_step next_step;
	size_t segment_length;
};

typedef struct http_session_s
{
	struct bytebuf_s buf_in;
	struct bytebuf_s buf_out;
	struct http_session_read_state_s state;
	struct headers_read_state_s headers_read_state;
	struct http_request_s *request;
} *http_session_t;

int
http_session_init(http_session_t session);

void
http_session_destroy(http_session_t session);

int
http_session_read(http_session_t session, char *buf, size_t size);

int
http_session_write(http_session_t session, const char *buf, size_t size);