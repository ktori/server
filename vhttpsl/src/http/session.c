/*
 * Created by victoria on 21.06.2020.
 */

#include <vhttpsl/bits/minmax.h>

#include "session.h"
#include "../server.h"
#include "../app.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <vhttpsl/bits/kv.h>
#include <errno.h>
#include <vhttpsl/http/request_line.h>

int
http_session_init(http_session_t session, struct vhttpsl_server_s *server)
{
	memset(session, 0, sizeof(*session));

	session->server = server;

	bytebuf_init(&session->buf_in, 256);
	bytebuf_init(&session->buf_out, 256);
	headers_read_begin(&session->headers_read_state);
	bytebuf_init(&session->state.request_line_buffer, 128);

	return EXIT_SUCCESS;
}

void
http_session_destroy(http_session_t session)
{
	bytebuf_destroy(&session->state.request_line_buffer);
	headers_read_end(&session->headers_read_state);
	bytebuf_destroy(&session->buf_in);
	bytebuf_destroy(&session->buf_out);

	if (session->request)
	{
		http_request_free(session->request);
		free(session->request);
	}
	headers_write_end(&session->write_state.headers_state);
}

int
http_session_read(http_session_t session, char *buf, size_t size)
{
	size_t i = 0, remaining;
	int retval;
	struct http_session_write_state_s s = session->write_state;
	response_list_node_t head = session->res_list_head;
	http_response_t res = head ? &head->response : NULL;

	if (!res)
	{
		errno = EAGAIN;
		return -1;
	}

	while (i < size && res && s.step != RSW_ERROR)
	{
		switch (s.step)
		{
			case RSW_BEGIN:
				s.step = RSW_STATUS_BEGIN;
				break;
			case RSW_STATUS_BEGIN:
				s.segment_length = 0;
				s.status_string_length = snprintf(s.status_string_buffer, 64, "HTTP/%1d.%1d %03d %s",
												  res->version_major, res->version_minor,
												  res->status, status_message(res->status));
				s.step = RSW_STATUS;
				break;
			case RSW_STATUS:
				remaining = MIN(size - i, s.status_string_length - s.segment_length);
				memcpy(buf + i, s.status_string_buffer + s.segment_length, remaining);
				s.segment_length += remaining;
				i += remaining;
				if (s.segment_length == s.status_string_length)
				{
					s.step = RSW_CR;
					if (res->headers)
						s.next_step = RSW_HEADERS_BEGIN;
					else if (res->body)
						s.next_step = RSW_BODY_BEGIN;
					else
						s.next_step = RSW_END_RESET;
				}
				break;
			case RSW_HEADERS_BEGIN:
				s.step = RSW_HEADERS;
				headers_write_begin(&s.headers_state, res->headers);
				break;
			case RSW_HEADERS:
				retval = headers_write(buf + i, (int) (size - i), &s.headers_state);
				i += retval;
				if (retval == 0)
				{
					if (res->length)
						s.step = RSW_BODY_BEGIN;
					else
						s.step = RSW_END_RESET;
				}
				break;
			case RSW_BODY_BEGIN:
				s.segment_length = 0;
				s.step = RSW_BODY;
				break;
			case RSW_BODY:
				remaining = MIN(size - i, res->length - s.segment_length);
				memcpy(buf + i, res->body + s.segment_length, remaining);
				s.segment_length += remaining;
				i += remaining;
				if (s.segment_length == res->length)
					s.step = RSW_END_RESET;
				break;
			case RSW_CR:
				buf[i++] = '\r';
				s.step = RSW_LF;
				break;
			case RSW_LF:
				buf[i++] = '\n';
				s.step = s.next_step;
				break;
			case RSW_END_RESET:
				if (head == session->res_list_tail)
					session->res_list_tail = NULL;
				session->res_list_head = head->next;
				http_response_free(res);
				free(head);
				head = session->res_list_head;
				res = head ? &head->response : NULL;
				s.step = RSW_BEGIN;
			case RSW_ERROR:
				break;
		}
	}

	return i;
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
	response_list_node_t res_node;

	while (i < size && s.step != SWS_ERROR)
	{
		consume = 1;

		switch (s.step)
		{
			case SWS_REQUEST_BEGIN:
				consume = 0;
				if (session->request)
				{
					http_request_free(session->request);
					free(session->request);
				}
				session->request = calloc(1, sizeof(*session->request));
				s.step = SWS_REQUEST_LINE;
				s.segment_length = 0;
				s.request_line_buffer.pos_write = 0;
				s.request_line_buffer.pos_read = 0;
				break;
			case SWS_REQUEST_LINE:
				if (buf[i] == '\r')
				{
					http_parse_request_line(s.request_line_buffer.data, s.request_line_buffer.pos_write,
											session->request);
					s.step = SWS_CR;
					s.next_step = SWS_HEADERS_BEGIN;
					consume = 0;
				}
				else
				{
					bytebuf_ensure_write(&s.request_line_buffer, 1);
					*bytebuf_write_ptr(&s.request_line_buffer) = buf[i];
					s.request_line_buffer.pos_write++;
					++s.segment_length;
				}
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
				/* add a response to the list */
				res_node = calloc(1, sizeof(*res_node));
				if (session->res_list_tail)
					session->res_list_tail->next = res_node;
				if (!session->res_list_head)
					session->res_list_head = res_node;
				session->res_list_tail = res_node;
				vhttpsl_app_execute(session->server->app, session->request, &res_node->response);
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
