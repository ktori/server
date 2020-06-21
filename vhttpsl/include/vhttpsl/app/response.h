/*
 * Created by victoria on 20.06.2020.
 */

#pragma once

#include <vhttpsl/http/status.h>
#include <vhttpsl/bits/kv.h>

typedef struct vhttpsl_response_s
{
	enum http_status status;

	int version_major;
	int version_minor;

	kv_list_t headers;

	char *body;
	size_t length;
} *vhttpsl_response_t;

int
vhttpsl_response_destroy(vhttpsl_response_t response);

int
vhttpsl_response_to_buffer(vhttpsl_response_t response, char *buf, size_t size);