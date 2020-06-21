/*
 * Created by victoria on 16.02.20.
*/

#include "vhttpsl/http/request_line.h"
#include "vhttpsl/http/request.h"
#include "vhttpsl/http/url.h"

typedef int bool;
#define TRUE 1
#define FALSE 0

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

int
http_parse_request_line(const char *buf, int size, struct http_request_s *request)
{
	enum state current_state = RL_BEGIN;
	const char *request_line_start = 0, *request_target_start = 0;
	const char *current = buf;
	bool consume;

	do
	{
		consume = TRUE;

		switch (current_state)
		{
			case RL_BEGIN:
				request_line_start = current;
				current_state = RL_METHOD;
				consume = FALSE;
				break;

			case RL_METHOD:
				if (*current == ' ')
				{
					request->method = http_method_from_string(request_line_start, current - request_line_start);
					if (request->method == HTTP_METHOD_UNKNOWN)
						current_state = RL_ERROR;
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
				request_target_start = current;
				current_state = RL_REQUEST_TARGET;
				consume = FALSE;
				break;
			case RL_REQUEST_TARGET:
				if (*current == ' ')
				{
					request->uri = uri_make(request_target_start, current - request_target_start);
					current_state = RL_REQUEST_TARGET_SP;
					consume = FALSE;
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
					current_state = RL_ERROR;
				break;
			case RL_HTTP_VERSION_H:
				if (*current == 'T')
					current_state = RL_HTTP_VERSION_HT;
				else
					current_state = RL_ERROR;
				break;
			case RL_HTTP_VERSION_HT:
				if (*current == 'T')
					current_state = RL_HTTP_VERSION_HTT;
				else
					current_state = RL_ERROR;
				break;
			case RL_HTTP_VERSION_HTT:
				if (*current == 'P')
					current_state = RL_HTTP_VERSION_HTTP;
				else
					current_state = RL_ERROR;
				break;
			case RL_HTTP_VERSION_HTTP:
				if (*current == '/')
					current_state = RL_HTTP_VERSION_MAJOR;
				else
					current_state = RL_ERROR;
				break;
			case RL_HTTP_VERSION_MAJOR:
				request->version_major = *current - '0';
				if (request->version_major != 1)
				{
					/* TODO: HTTP version not supported status code */
					current_state = RL_ERROR;
				}
				else
					current_state = RL_HTTP_VERSION_POINT;
				break;
			case RL_HTTP_VERSION_POINT:
				if (*current == '.')
					current_state = RL_HTTP_VERSION_MINOR;
				else
					current_state = RL_ERROR;
				break;
			case RL_HTTP_VERSION_MINOR:
				request->version_minor = *current - '0';
				if (request->version_minor < 0 || request->version_minor > 1)
				{
					/* TODO: HTTP version not supported status code */
					current_state = RL_ERROR;
				}
				else
					current_state = RL_CR_END;
				break;
			case RL_CR_END:
				if (*current == '\r')
					current_state = RL_LF_END;
				else
					current_state = RL_ERROR;
				break;
			case RL_LF_END:
				if (*current == '\n')
					current_state = RL_DONE;
				else
					current_state = RL_ERROR;
				break;
			default:
				break;
		}

		if (consume)
			current += 1;
	}
	while ((current <= buf + size) && current_state != RL_ERROR && current_state != RL_DONE);

	if (current_state != RL_DONE)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
