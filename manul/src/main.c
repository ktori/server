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
#include <yaml/document/kv.h>
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

static struct yaml_kv_s *
yaml_kv_get_safe(struct yaml_value_s *from, const char *key, enum yaml_value_type type)
{
	struct yaml_kv_s *kv;

	if (from->type != YVT_MAP)
	{
		fprintf(stderr, "could not pick key %s, wrong type %d\n", key, from->type);
		return NULL;
	}

	kv = yaml_kv_get(&from->body.map, key);

	if (!kv)
	{
		fprintf(stderr, "could not pick key %s, missing key\n", key);
		return NULL;
	}
	if (kv->value.type != type)
	{
		fprintf(stderr, "could not pick key %s, unexpected type %d (expected %d)\n", key, kv->value.type, type);
		return NULL;
	}

	return kv;
}

static int
load_listener(struct yaml_value_s *from, vhttpsl_server_t server, vhttpsl_app_t app)
{
	struct yaml_kv_s *kv;
	int port;
	const char *cert;
	const char *key;

	kv = yaml_kv_get_safe(from, "port", YVT_INT);
	if (!kv)
		return EXIT_FAILURE;

	port = kv->value.body.integer;

	kv = yaml_kv_get_safe(from, "type", YVT_STRING);
	if (!kv)
		return EXIT_FAILURE;

	if (!strcmp(kv->value.body.string, "http"))
	{
		if (vhttpsl_server_listen_http(server, app, NULL, port))
		{
			perror("vhttpsl_server_listen_http");

			return EXIT_FAILURE;
		}
	}
	else if (!strcmp(kv->value.body.string, "https"))
	{
		kv = yaml_kv_get_safe(from, "cert", YVT_STRING);
		if (!kv)
			return EXIT_FAILURE;
		cert = kv->value.body.string;
		kv = yaml_kv_get_safe(from, "key", YVT_STRING);
		if (!kv)
			return EXIT_FAILURE;
		key = kv->value.body.string;

		if (vhttpsl_server_listen_https(server, app, NULL, port, cert, key))
		{
			perror("vhttpsl_server_listen_https");

			return EXIT_FAILURE;
		}
	}
	else
	{
		fprintf(stderr, "listener: unknown type %s\n", kv->value.body.string);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static vhttpsl_app_t
app_from_map(struct yaml_value_s *map, vhttpsl_server_t server)
{
	struct yaml_kv_s *kv;
	size_t i;
	struct vhttpsl_callbacks_s callbacks = { callback_root, NULL, NULL };
	vhttpsl_app_t app;

	kv = yaml_kv_get_safe(map, "listeners", YVT_SEQUENCE);
	if (!kv)
		return NULL;

	app = vhttpsl_server_app_create(server, callbacks);

	for (i = 0; i < kv->value.body.sequence.count; ++i)
	{
		if (kv->value.body.sequence.values[i].type != YVT_MAP)
		{
			fprintf(stderr, "unexpected item type: %d at index %lu\n", kv->value.body.sequence.values[i].type, i);
			continue;
		}

		load_listener(&kv->value.body.sequence.values[i], server, app);
	}

	return app;
}

static int
apps_from_yaml_document(struct yaml_value_s *root, vhttpsl_server_t server)
{
	struct yaml_kv_s *kv;
	size_t i;

	if (root->type != YVT_MAP)
	{
		fprintf(stderr, "config root is not a map, got %d instead\n", root->type);
		return EXIT_FAILURE;
	}

	kv = yaml_kv_get_safe(root, "manul", YVT_MAP);
	if (!kv)
		return EXIT_FAILURE;
	kv = yaml_kv_get_safe(&kv->value, "apps", YVT_SEQUENCE);
	if (!kv)
		return EXIT_FAILURE;

	for (i = 0; i < kv->value.body.sequence.count; ++i)
	{
		if (kv->value.body.sequence.values[i].type != YVT_MAP)
		{
			fprintf(stderr, "unexpected item type: %d at index %lu\n", kv->value.body.sequence.values[i].type, i);
			continue;
		}

		app_from_map(&kv->value.body.sequence.values[i], server);
	}

	return EXIT_SUCCESS;
}

static void
yaml_load_app(vhttpsl_server_t server)
{
	struct yaml_s *yaml;
	struct yaml_document_s document;

	yaml_create(&yaml);

	yaml_document_init(&document);
	yaml_document_bind(&document, yaml);

	if (EXIT_SUCCESS == yaml_read_file(yaml, document.ctx, "conf/manul.yaml"))
	{
		apps_from_yaml_document(&document.root, server);
	}

	yaml_document_destroy(&document);
	yaml_free(&yaml);
}

int
main(int argc, char **argv)
{
	vhttpsl_server_t server;

	vhttpsl_init_openssl();

	server = vhttpsl_server_create(-1);

	yaml_load_app(server);

	while (vhttpsl_server_poll(server) == 0)
		;

	vhttpsl_server_destroy(&server);

	vhttpsl_cleanup_openssl();

	return 0;
}
