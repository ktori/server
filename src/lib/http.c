#include "http.h"
#include "crlf.h"
#include "url.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct http_request_s *
http_request_from_buffer(const char *buffer, size_t length)
{
	enum reading_state
	{
		METHOD,
		URI,
		VERSION_MAJOR,
		VERSION_MINOR
	};

	struct crlf_array_s *array;
	struct http_request_s *request;
	struct crlf_marker_s *current;
	char *request_line;
	char *uri;
	char *line;
	int fields, i, begin;
	enum reading_state state;

	request = calloc(1, sizeof(struct http_request_s));
	request->headers = kv_create();
	fields = crlf_array_from_buffer((char *) buffer, length, &array);
	if (fields < 1)
	{
		crlf_array_free(array);
		return NULL;
	}

	request_line = crlf_get_string(array->first);

	state = METHOD;
	begin = 0;
	for (i = 0; i < strlen(request_line); ++i)
	{
		if (state == METHOD)
		{
			if (request_line[i] == ' ')
			{
				state = URI;
				begin = i + 1;
				strncpy(request->method, request_line, i);
				request->method[i] = '\0';
				continue;
			}
			if (i >= 15)
			{
				free(request_line);
				crlf_array_free(array);
				return NULL;
			}
		}
		else if (state == URI)
		{
			if (request_line[i] == ' ')
			{
				uri = malloc(i - begin + 1);
				strncpy(uri, request_line + begin, i - begin);
				uri[i - begin] = '\0';
				request->uri = uri_make(uri);
				state = VERSION_MAJOR;
				begin = i + 6; /* " HTTP/" */
				i += 5;
				free(uri);
				continue;
			}
			if (i >= HTTP_URI_MAX_LENGTH)
			{
				free(request_line);
				crlf_array_free(array);
				return NULL;
			}
		}
		else if (state == VERSION_MAJOR)
		{
			request->version_major = request_line[i] - '0';
			i++;
			state = VERSION_MINOR;
		}
		else if (state == VERSION_MINOR)
		{
			request->version_minor = request_line[i] - '0';
			break;
		}
	}

	if (state != VERSION_MINOR || (request->uri == NULL))
	{
		free(request_line);
		crlf_array_free(array);
		return NULL;
	}

	request->body = NULL;
	request->length = 0;
	current = array->first;
	while (current != NULL)
	{
		line = crlf_get_string(current);
		kv_push_from_line(request->headers, line, ':', TRUE);
		free(line);
		if (request->body != NULL)
		{
			request->length += (current->size + 2);
		}
		if (current->size == 0)
		{
			request->body = current->loc + 2;
		}
		current = current->next;
	}
	request->length -= 2;
	free(request_line);
	crlf_array_free(array);

	return request;
}

void
http_request_free(struct http_request_s *request)
{
	if (request->uri != NULL)
	{
		uri_free(request->uri);
		free(request->uri);
	}
	kv_free(request->headers);
}

int
http_response_to_buffer(struct http_response_s *response,
						char *buffer,
						size_t length)
{
	struct kv_node_s *iter = NULL;
	int size = 0, total = 0;

	size = snprintf(buffer,
					length,
					"HTTP/%d.%d %d \r\n",
					response->version_major,
					response->version_minor,
					response->code);
	buffer += size;
	total += size;

	if (size == length)
		return FAILURE;

	if (response->raw != TRUE)
	{
		iter = response->headers->head;
		while (iter != NULL)
		{
			size = strlen(iter->key) + 2 + strlen(iter->value) + 2;
			if (total + size + 1 > length)
				return FAILURE;
			sprintf(buffer, "%s: %s\r\n", iter->key, iter->value);
			total += size;
			buffer += size;
			iter = iter->next;
		}

		if (total + response->length + 3 > length)
			return FAILURE;

		sprintf(buffer, "\r\n");
		buffer += 2;
		total += 2;
	}
	memcpy(buffer, response->body, response->length);
	total += response->length;

	return total;
}

int
http_response_length(struct http_response_s *response)
{
	struct kv_node_s *iter = NULL;
	int size = 0, total = 0;
	size = fmtlen("HTTP/%d.%d %d \r\n",
				  response->version_major,
				  response->version_minor,
				  response->code);
	total += size;
	iter = response->headers->head;
	while (iter != NULL)
	{
		/* ": "                      "\r\n" */
		size = strlen(iter->key) + 2 + strlen(iter->value) + 2;
		total += size;
		iter = iter->next;
	}

	total += 2; /* \r\n */
	total += response->length;

	return total;
}

void
http_response_free(struct http_response_s *response)
{
	free(response->body);
	kv_free(response->headers);
}