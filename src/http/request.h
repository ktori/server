/*
 * Created by victoria on 16.02.20.
*/
#pragma once

#include "../lib/bytebuf.h"
#include "status.h"

enum http_method
{
	HTTP_METHOD_UNKNOWN,
	HTTP_METHOD_GET,
	HTTP_METHOD_OPTIONS,
	HTTP_METHOD_HEAD,
	HTTP_METHOD_POST,
	HTTP_METHOD_PUT,
	HTTP_METHOD_PATCH,
	HTTP_METHOD_DELETE,
	HTTP_METHOD_CUSTOM,
	HTTP_METHOD_MAX = HTTP_METHOD_CUSTOM
};

struct http_request_s
{
	struct client_s *client;
	enum http_method method;

	int version_major;
	int version_minor;

	char *body;
	int length;

	struct bytebuf_s read_buffer;
	struct uri_s *uri;
	struct kv_list_s *headers;
};

struct http_request_s *
http_request_from_buffer(const char *buffer, unsigned long length);

int
http_request_read(struct client_s *client, struct http_request_s *request, enum http_status *out_status);

void
http_request_free(struct http_request_s *request);

const char *
http_request_method_name(struct http_request_s *request);

enum http_method
http_method_from_name(const char *buffer, size_t length);