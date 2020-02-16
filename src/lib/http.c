#include "http.h"
#include "crlf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		return EXIT_FAILURE;

	if (response->raw != TRUE)
	{
		iter = response->headers->head;
		while (iter != NULL)
		{
			size = strlen(iter->key) + 2 + strlen(iter->value) + 2;
			if (total + size + 1 > length)
				return EXIT_FAILURE;
			sprintf(buffer, "%s: %s\r\n", iter->key, iter->value);
			total += size;
			buffer += size;
			iter = iter->next;
		}

		if (total + response->length + 3 > length)
			return EXIT_FAILURE;

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