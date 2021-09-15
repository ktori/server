/*
 * Created by victoria on 2021-09-13.
 */

#include <stdio.h>
#include <vhttpsl/http/request.h>
#include <vhttpsl/http/response.h>
#include <vhttpsl/server.h>

/*
 * 	POST /cats
 * 		name=string
 * 	GET /cats
 * 	GET /cats/:id
 * 	PUT /cats/:id
 * 		name=string
 * 	DELETE /cats/:id
 */

typedef struct cat_s
{
	int id;
	char *name;
} * cat_t;

int next_cat_id = 1;
cat_t *cats;
size_t cats_size = 4;
size_t cats_count = 0;

cat_t
crud_cat_by_id(int id)
{
	size_t i = 0;

	for (i = 0; i < cats_size; ++i)
	{
		if (cats[i] && cats[i]->id == id)
			return cats[i];
	}

	return NULL;
}

int /* id */
crud_cat_create(const char *name, size_t name_len)
{
	size_t i = 0;

	if (cats_count >= cats_size)
	{
		return -1;
	}

	for (i = 0; i < cats_size; ++i)
	{
		if (!cats[i])
			break;
	}

	if (i >= cats_size)
	{
		fprintf(stderr, "should not happen\n");
		return -1;
	}

	cats[i] = calloc(1, sizeof(*cats[i]));
	cats[i]->name = calloc(name_len + 1, sizeof(char));
	strncpy(cats[i]->name, name, name_len);
	cats[i]->name[name_len] = 0;
	cats[i]->id = next_cat_id++;

	return cats[i]->id;
}

int
crud_cat_update(int id, const char *name, size_t name_len)
{
	cat_t cat = crud_cat_by_id(id);

	if (!cat)
	{
		return -1;
	}

	cat->name = realloc(cat->name, name_len + 1);
	strncpy(cat->name, name, name_len);
	cat->name[name_len] = 0;

	return cat->id;
}

int
crud_cat_delete(int id)
{
	size_t i = 0;

	for (i = 0; i < cats_size; ++i)
	{
		if (cats[i] && cats[i]->id == id)
		{
			free(cats[i]->name);
			free(cats[i]);
			cats[i] = NULL;
			cats_count--;

			return id;
		}
	}

	return -1;
}

struct handler_s
{
	enum http_method method;
	vhttpsl_http_callback_t callback;
};

typedef struct handler_node_s
{
	char *name;
	struct handler_s *handlers;
	size_t handlers_count;
	size_t handlers_size;
	struct handler_node_s *children;
	size_t children_count;
	size_t children_size;
} * handler_node_t;

static struct handler_node_s root = { NULL };

void
handler_node_init(handler_node_t node, const char *name)
{
	node->name = calloc(strlen(name) + 1, sizeof(char));
	strcpy(node->name, name);

	node->handlers_size = 4;
	node->handlers_count = 0;
	node->handlers = calloc(4, sizeof(*node->handlers));

	node->children_size = 4;
	node->children_count = 0;
	node->children = calloc(4, sizeof(*node->children));
}

handler_node_t
handler_node_new(const char *name)
{
	handler_node_t result = calloc(1, sizeof(*result));

	handler_node_init(result, name);

	return result;
}

void
handler_node_destroy(handler_node_t node)
{
	handler_node_t i = NULL;

	for (i = node->children; i < node->children + node->children_count; ++i)
	{
		handler_node_destroy(i);
	}

	free(node->children);
	free(node->handlers);
	free(node->name);
}

void
handler_node_free(handler_node_t node)
{
	handler_node_destroy(node);

	free(node);
}

vhttpsl_http_callback_t
find_handler(struct handler_node_s *start_from, const struct path_s *path, enum http_method method)
{
	struct path_node_s *path_node = path->head;
	struct handler_node_s *node = start_from;
	size_t child_idx = 0, method_idx = 0;
	int found = 0;

	while (path_node)
	{
		found = 0;
		for (child_idx = 0; child_idx < node->children_count; ++child_idx)
		{
			if (!strcmp(node->children[child_idx].name, path_node->name))
			{
				found = 1;
				node = &node->children[child_idx];
				break;
			}
		}

		if (!found)
			break; /* Not Found */

		path_node = path_node->next;

		found = 0;
		if (path_node == NULL)
		{
			for (method_idx = 0; method_idx < node->handlers_count; ++method_idx)
			{
				if (node->handlers[method_idx].method == method)
				{
					found = 1;
					break;
				}
			}
		}

		if (!found)
			break; /* Method Not Allowed */
	}

	if (!found)
		return NULL;

	return node->handlers[method_idx].callback;
}

void
register_handler(const char *path, enum http_method method, vhttpsl_http_callback_t callback)
{
	struct path_s *handler_path = path_make(path);
	struct path_node_s *path_node = handler_path->head;
	struct handler_node_s *node = &root, *i = NULL;
	size_t j;

	while (path_node)
	{
		for (i = node->children; i < node->children + node->children_count; ++i)
		{
			if (!strcmp(i->name, path_node->name))
				break;
		}

		if (i >= node->children + node->children_count)
		{
			if (node->children_count >= node->children_size)
			{
				node->children_size *= 2;
				node->children = realloc(node->children, node->children_size * sizeof(*node->children));
			}

			i = &node->children[node->children_count];
			handler_node_init(i, path_node->name);
			node->children_count += 1;
		}

		if (path_node->next == NULL)
		{
			for (j = 0; j < i->handlers_count; ++j)
			{
				if (i->handlers[j].method == method)
				{
					fprintf(stderr, "Handler already registered for %d %s\n", method, path);
					path_free(handler_path);

					return;
				}
			}

			if (i->handlers_count >= i->handlers_size)
			{
				i->handlers_size *= 2;
				i->handlers = realloc(i->handlers, i->handlers_size * sizeof(*i->handlers));
			}

			j = i->handlers_count;
			i->handlers_count++;

			i->handlers[j].callback = callback;
			i->handlers[j].method = method;
		}

		path_node = path_node->next;
	}

	path_free(handler_path);
}

void
callback_root(vhttpsl_app_t app, void *user_data, http_request_t request, http_response_t response)
{
	vhttpsl_http_callback_t callback = NULL;
	char buf[32];

	response->status = HTTP_S_OK;
	response->version_major = 1;
	response->version_minor = 1;

	fprintf(stderr, "Got request for %s\n", request->uri->spath);

	/* Processing here */
	switch (request->method)
	{
		case HTTP_METHOD_GET:
			/* GET /cats, /cats/:id */
			break;
		case HTTP_METHOD_POST:
			/* POST /cats */
			break;
		case HTTP_METHOD_PUT:
			/* PUT /cats */
			break;
		case HTTP_METHOD_DELETE:
			/* DELETE /cats */
			break;
		default:
			/* 405 Method Not Allowed */
			break;
	}

	callback = find_handler(&root, request->uri->path, request->method);
	if (callback) {
		callback(app, user_data, request, response);
		if (response->body) {
			response->length = strlen(response->body);
		}
	}
	else
	{
		fprintf(stderr, "could not find callback\n");

		response->status = HTTP_S_NOT_FOUND;
	}

	snprintf(buf, 32, "%d", (int)response->length);
	response->headers = kv_create();
	kv_push(response->headers, "Content-Length", buf);
	kv_push(response->headers, "Content-Type", "application/json");
	kv_push(response->headers, "Connection", "close");
}

void
callback_cats_create(vhttpsl_app_t app, void *user_data, http_request_t request, http_response_t response)
{
	int id = 0;
	const char *name;
	size_t name_len;

	fprintf(stderr, "CREATE CAT, BODY = %s\n", request->body);

	name = request->body;
	name_len = strlen(request->body);

	id = crud_cat_create(name, name_len);

	if (id < 0)
		response->status = HTTP_S_FORBIDDEN;
	else {
		response->body = calloc(name_len + 64, 1);
		snprintf(response->body, name_len + 64, "{\"id\": %d, \"name\": \"%s\"}", id, name);
	}
}
void
callback_cats_read(vhttpsl_app_t app, void *user_data, http_request_t request, http_response_t response)
{
}

void
callback_cats_read_id(vhttpsl_app_t app, void *user_data, http_request_t request, http_response_t response)
{
}

void
callback_cats_update(vhttpsl_app_t app, void *user_data, http_request_t request, http_response_t response)
{
	crud_cat_update(0, "Unnamed", 8);
}

void
callback_cats_delete(vhttpsl_app_t app, void *user_data, http_request_t request, http_response_t response)
{
	crud_cat_delete(0);
}

int
main(int argc, char **argv)
{
	vhttpsl_server_t server;
	vhttpsl_app_t app;
	struct vhttpsl_callbacks_s callbacks = { callback_root, NULL, NULL };

	cats = calloc(4, sizeof(cat_t));
	handler_node_init(&root, "");

	register_handler("cats", HTTP_METHOD_POST, callback_cats_create);
	register_handler("cats", HTTP_METHOD_GET, callback_cats_read);
	register_handler("cats/:id", HTTP_METHOD_GET, callback_cats_read_id);
	register_handler("cats", HTTP_METHOD_PUT, callback_cats_update);
	register_handler("cats", HTTP_METHOD_DELETE, callback_cats_delete);

	server = vhttpsl_server_create(-1);
	app = vhttpsl_server_app_create(server, callbacks);

	vhttpsl_server_listen_http(server, app, NULL, 8080);

	while (vhttpsl_server_poll(server) == 0)
		;

	vhttpsl_server_destroy(&server);

	return 0;
}
