/*
 * Created by victoria on 15.02.20.
*/

#include "server.h"
#include "client.h"
#include "../http.h"
#include "../serve/serve.h"

#include <stdio.h>
#include <stdlib.h>

int
server_accept(struct server_s *server, struct client_s *client)
{
	struct http_response_s *response;
	struct http_request_s *request;
	FILE *rq_log;
	char *rq_buffer;
	char *rs_buffer;
	int response_length;
	size_t request_length;

	if (client_read(client, &rq_buffer, &request_length) != EXIT_SUCCESS)
	{
		perror("reading request");
		return EXIT_FAILURE;
	}

	if (request_length >= 0)
	{
		rq_log = fopen("requests.log", "a");
		request = http_request_from_buffer(rq_buffer, request_length);
		fprintf(rq_log, "--- REQUEST ---\n\n%s\n", rq_buffer);
		fclose(rq_log);
		if (request == NULL)
		{
			fprintf(stderr, "NULL request");
			return EXIT_FAILURE;
		}
		request->sockfd = client->socket;
	}
	else
	{
		request = NULL;
	}

	response = calloc(1, sizeof(struct http_response_s));
	response->headers = kv_create();

	serve(request, response);

	response_length = http_response_length(response);
	rs_buffer = calloc(response_length + 2, 1);
	http_response_to_buffer(response, rs_buffer, response_length + 1);

	client_write(client, rs_buffer, response_length);

	if (request_length >= 0)
		free(rq_buffer);
	free(rs_buffer);

	http_response_free(response);
	if (request != NULL)
		http_request_free(request);

	return 0;
}