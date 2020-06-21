/*
 * Created by victoria on 20.06.2020.
 */

#include <vhttpsl/app.h>
#include <vhttpsl/server.h>
#include <vhttpsl/http/response.h>

#include <stdio.h>
#include <vhttpsl/http/request.h>

void
callback_root(http_request_t request, http_response_t response)
{
	char buf[32];
	kv_node_t node;

	node = request->headers ? request->headers->head : NULL;
	while (node)
	{
		if (0 == strcmp("Authorization", node->key))
			break;
		node = node->next;
	}

	response->status = HTTP_S_OK;
	response->version_major = 1;
	response->version_minor = 1;

	response->body = calloc(512, 1);
	snprintf(response->body, 512, "Hello, world!\n"
								  "Request URI: %s\n"
								  "Authorization: %s\n"
								  "HTTP Method: %s\n", request->uri->spath, node ? node->value : "NONE",
			 http_method_to_string(request->method));
	response->length = strlen(response->body);

	snprintf(buf, 32, "%d", (int) response->length);
	response->headers = kv_create();
	kv_push(response->headers, "Content-Length", buf);
	kv_push(response->headers, "Content-Type", "text/plain");
}

int
main(int argc, char **argv)
{
	vhttpsl_app_t app;
	vhttpsl_server_t server;

	app = vhttpsl_app_create();

	vhttpsl_app_route_add(app, "/", callback_root);

	server = vhttpsl_server_create(app);

	vhttpsl_server_listen_http(server, 8080);

	while (vhttpsl_server_poll(server) == 0);

	vhttpsl_server_destroy(&server);

	vhttpsl_app_destroy(&app);

	return 0;
}