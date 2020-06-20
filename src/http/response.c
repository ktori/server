/*
 * Created by victoria on 16.02.20.
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../lib/http.h"
#include "response.h"
#include "vhttpsl/bits/kv.h"
#include "../server/client.h"

void
http_response_free(struct http_response_s *response)
{
	free(response->body);
	kv_free(response->headers);
}

int
http_response_write(struct http_response_s *response, struct client_s *client)
{
	struct kv_node_s *iter = NULL;
	size_t size, buffer_size;
	char *status_buffer;

	const char *s_message = status_message(response->status);
	size_t status_message_size = strlen(s_message) + 16;
	buffer_size = status_message_size + 1;
	status_buffer = malloc(buffer_size);

	status_message_size = snprintf(status_buffer,
								   buffer_size,
								   "HTTP/%1d.%1d %03d %s \r\n",
								   response->version_major,
								   response->version_minor,
								   response->status,
								   s_message);

	if (status_message_size < 0)
	{
		free(status_buffer);
		return EXIT_FAILURE;
	}

	if (client_write(client, status_buffer, status_message_size) != EXIT_SUCCESS)
	{
		free(status_buffer);
		return EXIT_FAILURE;
	}

	if (!response->raw)
	{
		iter = response->headers->head;
		while (iter != NULL)
		{
			size = strlen(iter->key) + 2 + strlen(iter->value) + 2;
			if (size + 1 > buffer_size)
			{
				status_buffer = realloc(status_buffer, size + 1);
				buffer_size = size + 1;
			}
			snprintf(status_buffer, buffer_size, "%s: %s\r\n", iter->key, iter->value);
			client_write(client, status_buffer, size);
			iter = iter->next;
		}
		client_write(client, "\r\n", 2);
	}

	free(status_buffer);

	if (response->body)
	{
		if (client_write(client, response->body, response->length) != EXIT_SUCCESS)
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
