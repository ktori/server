/*
 * Created by victoria on 21.06.2020.
 */

#include <vhttpsl/http/methods.h>

#include <errno.h>
#include <string.h>

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

enum http_method
http_method_from_string(const char *buffer, size_t length)
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

const char *
http_method_to_string(enum http_method method)
{
	if (method < 0 || method > HTTP_METHOD_MAX)
	{
		errno = ERANGE;
		return NULL;
	}

	return METHOD_NAMES[method];
}