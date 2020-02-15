#include "router.h"

#include <stdlib.h>

struct route_node_s *route_head = NULL;

int
route(struct http_request_s *request, struct http_response_s *response)
{
	struct route_node_s *current;
	int status;

	current = route_head;
	while (current != NULL)
	{
		status = current->callback(request, response);
		if (status != -2)
		{
			return status;
		}
		current = current->next;
	}

	return -3;
}

void
add_server(server_fn server)
{
	struct route_node_s *node, *current;

	node = calloc(1, sizeof(struct route_node_s));

	node->callback = server;
	node->next = NULL;

	if (route_head == NULL)
	{
		route_head = node;
	}
	else
	{
		current = route_head;
		while (current != NULL)
		{
			if (current->next == NULL)
			{
				current->next = node;
				break;
			}
			current = current->next;
		}
	}
}