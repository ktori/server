//
// Created by victoria on 20.06.2020.
//

#pragma once

#include <stddef.h>

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

enum http_method
http_method_from_string(const char *buffer, size_t length);

const char *
http_method_to_string(enum http_method method);