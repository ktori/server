/*
 * Created by victoria on 11/10/20.
 */

#include <stdio.h>
#include <vhttpsl/app.h>
#include <vhttpsl/http/request.h>
#include <vhttpsl/http/response.h>
#include <vhttpsl/server.h>
#include <vhttpsl/ssl.h>
#include <yaml/callbacks.h>
#include <yaml/document/document.h>
#include <yaml/yaml.h>

void
callback_root(vhttpsl_app_t app, void *user_data, http_request_t request, http_response_t response)
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
	snprintf(response->body,
			 512,
			 "Hello, world!\n"
			 "Request URI: %s\n"
			 "Authorization: %s\n"
			 "HTTP Method: %s\n",
			 request->uri->spath,
			 node ? node->value : "NONE",
			 http_method_to_string(request->method));
	response->length = strlen(response->body);

	snprintf(buf, 32, "%d", (int)response->length);
	response->headers = kv_create();
	kv_push(response->headers, "Content-Length", buf);
	kv_push(response->headers, "Content-Type", "text/plain");
	kv_push(response->headers, "Connection", "close");
}

#define MAX_LINE_LENGTH 512

static int
yaml_read_file(struct yaml_s *yaml, struct document_load_ctx_s *ctx, const char *path)
{
	FILE *input;
	char buffer[MAX_LINE_LENGTH];
	int status = EXIT_SUCCESS;

	input = fopen(path, "r");

	if (input == NULL)
	{
		perror("fopen");

		return EXIT_FAILURE;
	}

	while (fgets(buffer, MAX_LINE_LENGTH, input))
	{
		if (yaml_in(yaml, buffer, strlen(buffer), ctx) != EXIT_SUCCESS)
		{
			fprintf(stderr, "YAML error\n");
			status = EXIT_FAILURE;
			break;
		}
	}

	fclose(input);

	return status;
}

static void
yaml_load_app()
{
	struct yaml_s *yaml;
	struct yaml_document_s document;

	yaml_create(&yaml);

	yaml_document_init(&document);
	yaml_document_bind(&document, yaml);

	if (EXIT_SUCCESS == yaml_read_file(yaml, document.ctx, "conf/manul.yaml"))
	{
	}

	yaml_document_destroy(&document);
	yaml_free(&yaml);
}

int
main(int argc, char **argv)
{
	vhttpsl_app_t app;
	vhttpsl_server_t server;
	struct vhttpsl_callbacks_s callbacks = { callback_root, NULL, NULL };

	yaml_load_app();

	vhttpsl_init_openssl();

	app = vhttpsl_app_create(callbacks);

	server = vhttpsl_server_create(-1);

	vhttpsl_server_listen_http(server, app, NULL, 8080);
	vhttpsl_server_listen_https(server, app, NULL, 8081, "conf/ssl/cert.pem", "conf/ssl/key.pem");

	while (vhttpsl_server_poll(server) == 0)
		;

	vhttpsl_server_destroy(&server);

	vhttpsl_app_destroy(&app);

	vhttpsl_cleanup_openssl();

	return 0;
}
