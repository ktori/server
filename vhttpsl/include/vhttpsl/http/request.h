/*
 * Created by victoria on 16.02.20.
*/
#pragma once

#include "status.h"
#include <vhttpsl/bits/bytebuf.h>
#include <vhttpsl/http/methods.h>

struct http_request_s
{
	/* request line */
	enum http_method method;
	struct uri_s *uri;
	int version_major;
	int version_minor;

	/* headers */
	struct kv_list_s *headers;

	/* body */
	char *body;
	int length;
};

int
http_request_read(struct client_s *client, struct http_request_s *request, enum http_status *out_status);

int
http_request_init(struct http_request_s *request);

void
http_request_free(struct http_request_s *request);