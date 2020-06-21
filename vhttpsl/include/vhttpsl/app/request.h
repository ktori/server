/*
 * Created by victoria on 20.06.2020.
 */

#pragma once

#include <vhttpsl/http/url.h>
#include "../http/methods.h"

typedef struct vhttpsl_app_request_s
{
	enum http_method method;
	struct uri_s *uri;
	int version_major;
	int version_minor;

	kv_list_t headers;

	char *body;
	int length;
} *vhttpsl_app_request_t;

void
vhttpsl_app_request_destroy(vhttpsl_app_request_t request);