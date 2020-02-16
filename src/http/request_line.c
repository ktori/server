/*
 * Created by victoria on 16.02.20.
*/

#include "request_line.h"
#include "request.h"
#include "../lib/url.h"
#include "../server/client.h"
#include "status.h"

int
request_line_read(struct http_request_s *request, enum http_status *out_status)
{
	struct bytebuf_s *buf = &request->read_buffer;

	enum state
	{
		RL_ERROR,
		RL_BEGIN,
		RL_METHOD,
		RL_METHOD_SP,
		RL_REQUEST_TARGET_START,
		RL_REQUEST_TARGET,
		RL_REQUEST_TARGET_SP,
		RL_HTTP_VERSION,
		RL_HTTP_VERSION_H,
		RL_HTTP_VERSION_HT,
		RL_HTTP_VERSION_HTT,
		RL_HTTP_VERSION_HTTP,
		RL_HTTP_VERSION_MAJOR,
		RL_HTTP_VERSION_POINT,
		RL_HTTP_VERSION_MINOR,
		RL_CR_END,
		RL_LF_END,
		RL_DONE
	};

	enum state current_state = RL_BEGIN;
	size_t out_read;
	size_t request_line_start = 0, request_target_start = 0;
	const char *current;
	bool consume;

	do
	{
		while (buf->pos_read == buf->pos_write)
		{
			bytebuf_ensure_write(buf, 256);
			if (client_read_some(request->client, bytebuf_write_ptr(buf), bytebuf_write_size(buf), &out_read) !=
				EXIT_SUCCESS)
			{
				*out_status = HTTP_S_BAD_REQUEST;
				return EXIT_FAILURE;
			}
			buf->pos_write += out_read;
		}

		current = bytebuf_read_ptr(&request->read_buffer);
		consume = TRUE;

		switch (current_state)
		{
			case RL_BEGIN:
				request_line_start = buf->pos_read;
				current_state = RL_METHOD;
				consume = FALSE;
				break;
			case RL_METHOD:
				if (*current == ' ')
				{
					request->method = http_method_from_name(buf->data + request_line_start,
															buf->pos_read - request_line_start);
					if (request->method == HTTP_METHOD_UNKNOWN)
					{
						*out_status = HTTP_S_BAD_REQUEST;
						current_state = RL_ERROR;
					}
					else
						current_state = RL_METHOD_SP;
					consume = FALSE;
				}
				break;
			case RL_METHOD_SP:
				if (*current != ' ')
				{
					current_state = RL_REQUEST_TARGET_START;
					consume = FALSE;
				}
				break;
			case RL_REQUEST_TARGET_START:
				request_target_start = buf->pos_read;
				current_state = RL_REQUEST_TARGET;
				consume = FALSE;
				break;
			case RL_REQUEST_TARGET:
				if (*current == ' ')
				{
					request->uri = uri_make(buf->data + request_target_start, buf->pos_read - request_target_start);
					current_state = RL_REQUEST_TARGET_SP;
					consume = FALSE;
					break;
				}
				break;
			case RL_REQUEST_TARGET_SP:
				if (*current != ' ')
				{
					current_state = RL_HTTP_VERSION;
					consume = FALSE;
				}
				break;
			case RL_HTTP_VERSION:
				if (*current == '\r')
				{
					request->version_major = 0;
					request->version_minor = 9;
					current_state = RL_LF_END;
				}
				else if (*current == 'H')
				{
					current_state = RL_HTTP_VERSION_H;
				}
				else
				{
					*out_status = HTTP_S_BAD_REQUEST;
					current_state = RL_ERROR;
				}
				break;
			case RL_HTTP_VERSION_H:
				if (*current == 'T')
					current_state = RL_HTTP_VERSION_HT;
				else
				{
					*out_status = HTTP_S_BAD_REQUEST;
					current_state = RL_ERROR;
				}
				break;
			case RL_HTTP_VERSION_HT:
				if (*current == 'T')
					current_state = RL_HTTP_VERSION_HTT;
				else
				{
					*out_status = HTTP_S_BAD_REQUEST;
					current_state = RL_ERROR;
				}
				break;
			case RL_HTTP_VERSION_HTT:
				if (*current == 'P')
					current_state = RL_HTTP_VERSION_HTTP;
				else
				{
					*out_status = HTTP_S_BAD_REQUEST;
					current_state = RL_ERROR;
				}
				break;
			case RL_HTTP_VERSION_HTTP:
				if (*current == '/')
					current_state = RL_HTTP_VERSION_MAJOR;
				else
				{
					*out_status = HTTP_S_BAD_REQUEST;
					current_state = RL_ERROR;
				}
				break;
			case RL_HTTP_VERSION_MAJOR:
				request->version_major = *current - '0';
				if (request->version_major != 1)
				{
					*out_status = HTTP_S_VERSION_NOT_SUPPORTED;
					current_state = RL_ERROR;
				}
				else
					current_state = RL_HTTP_VERSION_POINT;
				break;
			case RL_HTTP_VERSION_POINT:
				if (*current == '.')
					current_state = RL_HTTP_VERSION_MINOR;
				else
				{
					*out_status = HTTP_S_BAD_REQUEST;
					current_state = RL_ERROR;
				}
				break;
			case RL_HTTP_VERSION_MINOR:
				request->version_minor = *current - '0';
				if (request->version_minor < 0 || request->version_minor > 1)
				{
					*out_status = HTTP_S_VERSION_NOT_SUPPORTED;
					current_state = RL_ERROR;
				}
				else
					current_state = RL_CR_END;
				break;
			case RL_CR_END:
				if (*current == '\r')
					current_state = RL_LF_END;
				else
				{
					*out_status = HTTP_S_BAD_REQUEST;
					current_state = RL_ERROR;
				}
				break;
			case RL_LF_END:
				if (*current == '\n')
					current_state = RL_DONE;
				else
				{
					*out_status = HTTP_S_BAD_REQUEST;
					current_state = RL_ERROR;
				}
				break;
			default:
				break;
		}

		if (consume)
			buf->pos_read += 1;
	}
	while (current_state != RL_ERROR && current_state != RL_DONE);

	if (current_state == RL_ERROR)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
