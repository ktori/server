/*
 * Created by victoria on 16.02.20.
*/
#include "status.h"

#include <unistd.h>

struct status_message_s
{
	enum http_status status;
	const char *message;
};

static const struct status_message_s MESSAGES[] = {
		{HTTP_S_OK,                     "OK"},

		{HTTP_S_BAD_REQUEST,            "Bad Request"},
		{HTTP_S_UNAUTHORIZED,           "Unauthorized"},
		{HTTP_S_FORBIDDEN,              "Forbidden"},
		{HTTP_S_NOT_FOUND,              "Not Found"},
		{HTTP_S_METHOD_NOT_ALLOWED,     "Method Not Allowed"},
		{HTTP_S_NOT_ACCEPTABLE,         "Not Acceptable"},
		{HTTP_S_REQUEST_TIMEOUT,        "Request Timeout"},
		{HTTP_S_LENGTH_REQUIRED,        "Length Required"},
		{HTTP_S_PAYLOAD_TOO_LARGE,      "Payload Too Large"},
		{HTTP_S_URI_TOO_LONG,           "URI Too Long"},
		{HTTP_S_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
		{HTTP_S_EXPECTATION_FAILED,     "Expectation Failed"},
		{HTTP_S_HEADER_TOO_LARGE,       "Request Header Fields Too Large"},

		{HTTP_S_SERVER_ERROR,           "Internal Server Error"},
		{HTTP_S_NOT_IMPLEMENTED,        "Not Implemented"},
		{HTTP_S_BAD_GATEWAY,            "Bad Gateway"},
		{HTTP_S_SERVICE_UNAVAILABLE,    "Service Unavailable"},
		{HTTP_S_GATEWAY_TIMEOUT,        "Gateway Timeout"},
		{HTTP_S_VERSION_NOT_SUPPORTED,  "HTTP Version Not Supported"}
};

const char *
status_message(enum http_status status)
{
	size_t i;
	for (i = 0; i < sizeof(MESSAGES) / sizeof(struct status_message_s); ++i)
	{
		if (MESSAGES[i].status == status)
			return MESSAGES[i].message;
	}
	return "Unknown";
}
