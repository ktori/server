/*
 * Created by victoria on 21.06.2020.
 */

#pragma once

#include <vhttpsl/bits/bytebuf.h>
#include <vhttpsl/http/request.h>
#include <vhttpsl/http/headers.h>
#include <vhttpsl/app.h>
#include <vhttpsl/http/response.h>

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

enum session_response_write_step
{
	RSW_BEGIN,
	RSW_STATUS_BEGIN,
	RSW_STATUS,
	RSW_HEADERS_BEGIN,
	RSW_HEADERS,
	RSW_BODY_BEGIN,
	RSW_BODY,
	RSW_CR,
	RSW_LF,
	RSW_END_RESET,
	RSW_ERROR
};

struct http_session_read_state_s
{
	enum session_write_step step;
	enum session_write_step next_step;
	size_t segment_length;
	struct bytebuf_s request_line_buffer;
};

typedef struct response_list_node_s
{
	struct http_response_s response;

	struct response_list_node_s *next;
} *response_list_node_t;

struct http_session_write_state_s
{
	enum session_response_write_step step;
	enum session_response_write_step next_step;

	size_t segment_length;
	struct headers_write_state_s headers_state;
	char status_string_buffer[64];
	size_t status_string_length;
};

typedef struct http_session_s
{
	struct bytebuf_s buf_in;
	struct bytebuf_s buf_out;
	struct http_session_read_state_s state;
	struct headers_read_state_s headers_read_state;
	struct http_request_s *request;
	struct vhttpsl_app_s *app;
	struct http_session_write_state_s write_state;

	response_list_node_t res_list_head;
	response_list_node_t res_list_tail;
} *http_session_t;

int
http_session_init(http_session_t session, struct vhttpsl_app_s *app);

void
http_session_destroy(http_session_t session);

size_t
http_session_read(http_session_t session, char *buf, size_t size);

size_t
http_session_write(http_session_t session, const char *buf, size_t size);
