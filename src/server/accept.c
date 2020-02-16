/*
 * Created by victoria on 15.02.20.
*/

#include "server.h"
#include "client.h"
#include "../lib/http.h"
#include "../serve/serve.h"
#include "../http/request.h"

#include <stdio.h>
#include <stdlib.h>

int
server_accept(struct server_s *server, struct client_s *client)
{
	struct http_response_s *response;
	struct http_request_s request = {};
	char *rs_buffer;
	int response_length;

	if (http_request_read(client, &request) != EXIT_SUCCESS)
	{
		perror("reading request");
		http_request_free(&request);
		return EXIT_FAILURE;
	}

	/*
	rq_log = fopen("requests.log", "a");
	fprintf(rq_log, "--- REQUEST ---\n\n%s\n", rq_buffer);
	fclose(rq_log);
	 */

	response = calloc(1, sizeof(struct http_response_s));
	response->headers = kv_create();

	serve(&request, response);

	response_length = http_response_length(response);
	rs_buffer = calloc(response_length + 2, 1);
	http_response_to_buffer(response, rs_buffer, response_length + 1);

	client_write(client, rs_buffer, response_length);

	free(rs_buffer);

	http_response_free(response);
	http_request_free(&request);

	return 0;
}