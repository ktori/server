/*
 * Created by victoria on 16.02.20.
*/
#pragma once

#include "../http/status.h"

struct client_s;

struct http_response_s
{
	enum http_status status;
	int version_major;
	int version_minor;

	char *body;
	unsigned long length;
	int raw;

	struct kv_list_s *headers;
};

int
http_response_write(struct http_response_s *response, struct client_s *client);

void
http_response_free(struct http_response_s *response);