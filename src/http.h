#ifndef HTTP_H
#define HTTP_H

#include "common.h"
#include "kv.h"

#define HTTP_URI_MAX_LENGTH 2048

enum http_status
{
	HTTP_OK = 200,

	HTTP_BAD_REQUEST = 400,
	HTTP_UNAUTHORIZED = 401,
	HTTP_FORBIDDEN = 403,
	HTTP_NOT_FOUND = 404,
	HTTP_METHOD_NOT_ALLOWED = 405,

	HTTP_SERVER_ERROR = 500,
	HTTP_NOT_IMPLEMENTED = 501
};

struct http_request_s
{
	int sockfd;
	char method[16];
	int version_major;
	int version_minor;

	char *body;
	int length;

	struct uri_s *uri;
	struct kv_list_s *headers;
};

struct http_request_s *
http_request_from_buffer(const char *buffer, size_t length);

void
http_request_free(struct http_request_s *request);

struct http_response_s
{
	int code;
	int version_major;
	int version_minor;

	char *body;
	int length;
	bool raw;

	struct kv_list_s *headers;
};

int
http_response_to_buffer(struct http_response_s *response,
						char *buffer,
						size_t length);

int
http_response_length(struct http_response_s *response);

void
http_response_free(struct http_response_s *response);

#endif /* HTTP_H */