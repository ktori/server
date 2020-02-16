/*
 * Created by victoria on 15.02.20.
*/

#include "server.h"
#include "client.h"
#include "../lib/http.h"
#include "../serve/serve.h"
#include "../http/request.h"
#include "../http/status.h"
#include "../http/response.h"

#include <stdio.h>
#include <stdlib.h>

static int
server_make_response(struct client_s *client, struct http_response_s *response)
{
	struct http_request_s request = {0};

	if (http_request_read(client, &request, &response->status) != EXIT_SUCCESS)
	{
		perror("reading request");
		http_request_free(&request);
		return EXIT_FAILURE;
	}

	serve(&request, response);
	http_request_free(&request);

	return EXIT_SUCCESS;
}

int
server_accept(struct server_s *server, struct client_s *client)
{
	struct http_response_s response = {.version_major = 1, .version_minor = 1, 0};
	int exit_status;

	response.headers = kv_create();
	kv_push(response.headers, "Server", "server.c");
	kv_push(response.headers, "Connection", "close");

	exit_status = server_make_response(client, &response);

	if (response.body != NULL)
	{
		char content_length[16];
		snprintf(content_length, 15, "%zu", response.length);
		kv_push(response.headers, "Content-Length", content_length);
	}

	/*
	rq_log = fopen("requests.log", "a");
	fprintf(rq_log, "--- REQUEST ---\n\n%s\n", rq_buffer);
	fclose(rq_log);
	 */

	if (http_response_write(&response, client) != EXIT_SUCCESS)
		exit_status = EXIT_FAILURE;

	http_response_free(&response);

	return exit_status;
}