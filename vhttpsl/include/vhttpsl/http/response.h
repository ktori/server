/*
 * Created by victoria on 16.02.20.
*/
#pragma once

#include "status.h"

struct client_s;

struct http_response_s
{
	/* context */

	/* status line */
	enum http_status status;

	int version_major;
	int version_minor;
	/* headers */
	struct kv_list_s *headers;

	/* body */
	char *body;
	unsigned long length;

	int raw;
};

int
http_response_write(struct http_response_s *response, struct client_s *client);

void
http_response_free(struct http_response_s *response);