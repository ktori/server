#pragma once

#include "../lib/str.h"
#include "../lib/http.h"
#include "../http/request.h"

typedef enum http_status (*server_fn)(struct http_request_s *,
									  struct http_response_s *);

int
route(struct http_request_s *request, struct http_response_s *response);

struct route_node_s
{
	server_fn callback;

	struct route_node_s *next;
};

extern struct route_node_s *route_head;

void
add_server(server_fn server);

/*http_status
serve_index(struct http_request_s *request, struct http_response_s *response);
*/
