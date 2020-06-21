/*
 * Created by victoria on 16.02.20.
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "vhttpsl/http/url.h"
#include "../../../src/lib/crlf.h"
#include "../../../src/lib/http.h"
#include "vhttpsl/http/request.h"
#include "vhttpsl/bits/kv.h"
#include "../../../src/lib/crlf.h"
#include "vhttpsl/http/url.h"
#include "../../../src/server/client.h"
#include "vhttpsl/bits/bytebuf.h"
#include "vhttpsl/http/request_line.h"
#include "vhttpsl/http/headers.h"
#include "vhttpsl/http/status.h"

void
http_request_free(struct http_request_s *request)
{
	if (request->uri != NULL)
	{
		uri_free(request->uri);
		free(request->uri);
	}
	kv_free(request->headers);
	bytebuf_destroy(&request->read_buffer);
}

static const char *METHOD_NAMES[] = {
		NULL,
		"GET",
		"OPTIONS",
		"HEAD",
		"POST",
		"PUT",
		"PATCH",
		"DELETE",
		NULL
};

const char *
http_request_method_name(struct http_request_s *request)
{
	if (request->method < 0 || request->method > HTTP_METHOD_MAX)
	{
		errno = ERANGE;
		return NULL;
	}

	return METHOD_NAMES[request->method];
}

enum http_method
http_method_from_name(const char *buffer, size_t length)
{
	enum http_method i;
	for (i = 0; i < HTTP_METHOD_MAX; ++i)
	{
		if (METHOD_NAMES[i] == NULL)
			continue;
		if (strncmp(buffer, METHOD_NAMES[i], length) == 0)
			return i;
	}
	return HTTP_METHOD_UNKNOWN;
}

#define HTTP_METHOD_MAX_LENGTH 32
#define HTTP_TARGET_MAX_LENGTH 8192

/*
static int
http_request_read_request_line(struct http_request_s *request)
{
	size_t total_read = 0;
	size_t out_read;
	do
	{
		bytebuf_ensure_write(&request->read_buffer, 32);
		if (client_read_some(request->client, bytebuf_write_ptr(&request->read_buffer), bytebuf_write_size(&request->read_buffer), &out_read) != EXIT_SUCCESS)
			return EXIT_FAILURE;

		total_read += out_read;
		request->read_buffer.pos_write += out_read;
	} while (total_read < HTTP_METHOD_MAX_LENGTH);

	const char *i, *start = bytebuf_read_ptr(&request->read_buffer), *end = request->read_buffer.data + request->read_buffer.pos_write;
	for (i = start; i < end; ++i)
	{
		if (*i == ' ')
		{
			request->method = http_method_from_name(start, i - start);
			request->read_buffer.pos_read += (i - start + 1);
			break;
		}
	}
	if (request->method == HTTP_METHOD_UNKNOWN)
		return EXIT_FAILURE;

	total_read = 0;
	bool reading = TRUE;
	const char *target_start = bytebuf_read_ptr(&request->read_buffer);
	do {
		start = bytebuf_read_ptr(&request->read_buffer);
		end = request->read_buffer.data + request->read_buffer.pos_write;

		for (i = start; i < end; ++i)
		{
			if (*i == ' ' || *i == '\r')
			{
				if (*i == ' ')
					request->read_buffer.pos_read += 1;
				request->uri = uri_make(target_start, i - target_start);
				reading = FALSE;
				break;
			}
		}

		request->read_buffer.pos_read += (i - start);

		if (!reading)
			break;

		if (total_read >= HTTP_TARGET_MAX_LENGTH)
			return EXIT_FAILURE;

		bytebuf_ensure_write(&request->read_buffer, 256);
		if (client_read_some(request->client, bytebuf_write_ptr(&request->read_buffer), bytebuf_write_size(&request->read_buffer), &out_read) != EXIT_SUCCESS)
			return EXIT_FAILURE;
		total_read += out_read;
		request->read_buffer.pos_write += out_read;
	} while (reading);

	start = bytebuf_read_ptr(&request->read_buffer);
	if (*start == '\r')
	{
		if (*(start + 1) != '\n')
			return EXIT_FAILURE;
		request->version_major = 0;
		request->version_minor = 9;
		request->read_buffer.pos_read += 2;
		return EXIT_SUCCESS;
	}

	while (request->read_buffer.pos_write - request->read_buffer.pos_read < 7)
	{
		bytebuf_ensure_write(&request->read_buffer, 7);
		if (client_read_some(request->client, bytebuf_write_ptr(&request->read_buffer), bytebuf_write_size(&request->read_buffer), &out_read) != EXIT_SUCCESS)
			return EXIT_FAILURE;
	}

	if (strncmp(start, "HTTP/", 5) != 0)
		return EXIT_FAILURE;

	request->version_major = start[5] - '0';
	request->version_minor = start[7] - '0';

	if (!(request->version_major == 1 && (request->version_minor == 0 || request->version_minor == 1)))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
*/

#define HTTP_HEADER_MAX_LENGTH 8192

/*
static int
http_request_read_headers(struct http_request_s *request)
{
	struct bytebuf_s *buf = &request->read_buffer;

	const char *i, *start, *end, *header_start = NULL;
	size_t out_read, total_read = 0;
	bool reading = TRUE;

	do {
		start = bytebuf_read_ptr(buf);
		if (header_start == NULL)
			header_start = start;
		end = buf->data + buf->pos_write;

		for (i = start; i < end; ++i)
		{
			if (*i == '\r' && *(i + 1) == '\n')
			{
				kv_push_from_line(request->headers, header_start, i - header_start, ':', TRUE);
				header_start = NULL;

				if (*i == ' ')
					request->read_buffer.pos_read += 1;
				reading = FALSE;
				break;
			}
		}

		request->read_buffer.pos_read += (i - start);

		if (!reading)
			break;

		if (total_read >= HTTP_HEADER_MAX_LENGTH)
			return EXIT_FAILURE;

		bytebuf_ensure_write(buf, 256);
		if (client_read_some(request->client, bytebuf_write_ptr(buf), bytebuf_write_size(buf), &out_read) != EXIT_SUCCESS)
			return EXIT_FAILURE;
		total_read += out_read;
		request->read_buffer.pos_write += out_read;
	} while (reading);

	return EXIT_SUCCESS;
}
*/

int
http_request_read(struct client_s *client, struct http_request_s *request, enum http_status *out_status)
{
	memset(request, 0, sizeof(struct http_request_s));
	request->client = client;
	if (bytebuf_init(&request->read_buffer, 256) != EXIT_SUCCESS)
	{
		*out_status = HTTP_S_SERVER_ERROR;
		return EXIT_FAILURE;
	}

	request->headers = kv_create();

	/* read request-line */
/*	if (http_parse_request_line(request, out_status) != EXIT_SUCCESS)
		return EXIT_FAILURE;*/
	/* read headers */
	/*if (headers_read(request, out_status) != EXIT_SUCCESS)
		return EXIT_FAILURE;*/
	/* read body TODO */

	return EXIT_SUCCESS;
}

int
http_request_init(struct http_request_s *request)
{
	return EXIT_SUCCESS;
}
