/*
 * Created by victoria on 20.06.2020.
 */

#include "server.h"
#include "app.h"

#include "socket_context.h"
#include "vhttpsl/server.h"

#include <errno.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <streams/buffered_pipe.h>
#include <streams/streams.h>
#include <sys/epoll.h>
#include <vhttpsl/listener.h>

vhttpsl_server_t
vhttpsl_server_create(int epoll_fd)
{
	vhttpsl_server_t server = calloc(1, sizeof(*server));

	server->epoll_owned = epoll_fd < 0;
	if (server->epoll_owned)
		server->epoll_fd = epoll_create1(0);
	else
		server->epoll_fd = epoll_fd;

	if (server->epoll_fd < 0)
	{
		perror("epoll_create1()");
		free(server);

		return NULL;
	}

	server->listeners_size = 2;
	server->listeners = calloc(server->listeners_size, sizeof(listener_t));

	return server;
}

void
vhttpsl_server_destroy(vhttpsl_server_t *server)
{
	size_t i;

	fprintf(stderr, "TODO: vhttpsl_server_destroy\n");

	for (i = 0; i < (*server)->listeners_count; ++i)
		listener_destroy((*server)->listeners[i]);

	for (i = 0; i < (*server)->apps_count; ++i)
		vhttpsl_app_destroy((*server)->apps[i]);

	free(*server);
	*server = NULL;
}

typedef struct sv_data_s
{
	vhttpsl_server_t server;
	vhttpsl_app_t app;
	SSL_CTX *ssl_ctx;
} * sv_data_t;

static sv_data_t
sv_data_new_http(vhttpsl_server_t server, vhttpsl_app_t app)
{
	sv_data_t result = calloc(1, sizeof(*result));

	if (!result)
		return NULL;

	result->server = server;
	result->app = app;
	result->ssl_ctx = NULL;

	return result;
}

static SSL_CTX *
create_ssl_ctx(const char *cert, const char *key)
{
	const SSL_METHOD *method = SSLv23_server_method();
	SSL_CTX *ctx = SSL_CTX_new(method);

	if (!ctx)
	{
		ERR_print_errors_fp(stderr);
		return NULL;
	}

	SSL_CTX_set_ecdh_auto(ctx, 1);

	if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return NULL;
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return NULL;
	}

	return ctx;
}

static sv_data_t
sv_data_new_https(vhttpsl_server_t server, vhttpsl_app_t app, const char *cert, const char *key)
{
	sv_data_t result = calloc(1, sizeof(*result));

	if (!result)
		return NULL;

	result->server = server;
	result->app = app;
	result->ssl_ctx = create_ssl_ctx(cert, key);

	if (!result->ssl_ctx)
	{
		fprintf(stderr, "unable to create ssl context");
		ERR_print_errors_fp(stderr);

		free(result);

		return NULL;
	}

	return result;
}

static int
server_cl_accept(int fd, sv_data_t sv_data, client_context_t *cl_data)
{
	*cl_data = client_context_create(fd, sv_data->app, sv_data->ssl_ctx);

	if (!*cl_data)
	{
		fprintf(stderr, "could not create client context\n");

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int
server_cl_event(int fd, sv_data_t sv_data, client_context_t cl_data)
{
	int eof_read = 0;

	switch (stream_buffered_pipe_pass(&cl_data->pipe_in))
	{
		case EXIT_SUCCESS:
			eof_read = 1;
		case PIPE_READ_ERROR:
			if (errno != EAGAIN)
				perror("request: socket read error");
			break;
		case PIPE_WRITE_ERROR:
			if (errno != EAGAIN)
				perror("request: application write error");
			break;
		default:
			perror("request: unknown IO error");
			break;
	}

	switch (stream_buffered_pipe_pass(&cl_data->pipe_out))
	{
		case EXIT_SUCCESS:
			eof_read = 1;
		case PIPE_READ_ERROR:
			if (errno != EAGAIN)
				perror("response: application read error");
			break;
		case PIPE_WRITE_ERROR:
			if (errno != EAGAIN)
				perror("response: socket write error");
			break;
		default:
			perror("response: unknown IO error");
			break;
	}

	/* verify that the client is still active */
	if (eof_read)
		return 1;

	return EXIT_SUCCESS;
}

static int
server_cl_close(int fd, sv_data_t sv_data, client_context_t cl_data)
{
	printf("client %d: closing connection\n", fd);

	client_context_destroy(cl_data);

	return EXIT_SUCCESS;
}

static int
server_sv_destroy(sv_data_t sv_data)
{
	if (sv_data->ssl_ctx)
	{
		SSL_CTX_free(sv_data->ssl_ctx);
	}

	return EXIT_SUCCESS;
}

static int
vhttpsl_server_add_listener(vhttpsl_server_t server, listener_t listener)
{
	if (server->listeners_size == server->listeners_count)
	{
		server->listeners = realloc(server->listeners, server->listeners_size * sizeof(listener_t));
		server->listeners_size *= 2;
	}

	server->listeners[server->listeners_count++] = listener;

	return EXIT_SUCCESS;
}

int
vhttpsl_server_listen_http(vhttpsl_server_t server, vhttpsl_app_t app, const char *name, int port)
{
	listener_t listener;
	struct listener_callbacks_s callbacks = { NULL,
											  (on_cl_accept_callback_t)server_cl_accept,
											  (on_cl_event_callback_t)server_cl_event,
											  (on_cl_close_callback_t)server_cl_close,
											  (on_sv_destroy_callback_t)server_sv_destroy };

	callbacks.sv_data = sv_data_new_http(server, app);

	listener = listener_new(name, port, callbacks, server->epoll_fd);
	if (!listener)
	{
		fprintf(stderr, "vhttpsl_server_listen_https: could not create listener\n");

		return EXIT_FAILURE;
	}

	vhttpsl_server_add_listener(server, listener);

	return EXIT_SUCCESS;
}

int
vhttpsl_server_listen_https(vhttpsl_server_t server,
							vhttpsl_app_t app,
							const char *name,
							int port,
							const char *cert,
							const char *key)
{
	listener_t listener;
	struct listener_callbacks_s callbacks = { NULL,
											  (on_cl_accept_callback_t)server_cl_accept,
											  (on_cl_event_callback_t)server_cl_event,
											  (on_cl_close_callback_t)server_cl_close,
											  (on_sv_destroy_callback_t)server_sv_destroy };

	callbacks.sv_data = sv_data_new_https(server, app, cert, key);

	listener = listener_new(name, port, callbacks, server->epoll_fd);
	if (!listener)
	{
		fprintf(stderr, "vhttpsl_server_listen_https: could not create listener\n");

		return EXIT_FAILURE;
	}

	vhttpsl_server_add_listener(server, listener);

	return EXIT_SUCCESS;
}

int
vhttpsl_server_poll(vhttpsl_server_t server)
{
	int ret;

	ret = listeners_poll(server->epoll_fd);

	if (ret)
	{
		fprintf(stderr, "vhttpsl_server_poll: listeners_poll returned %d\n", ret);

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

vhttpsl_app_t
vhttpsl_server_app_create(vhttpsl_server_t server, struct vhttpsl_callbacks_s callbacks)
{
	vhttpsl_app_t app = calloc(1, sizeof(*app));

	fprintf(stderr, "TODO: vhttpsl_app_create\n");
	app->callbacks = callbacks;

	if (server->apps_count == server->apps_size)
	{
		server->apps_size *= 2;
		server->apps = realloc(server->apps, server->apps_size * sizeof(vhttpsl_app_t));
	}

	server->apps[server->apps_count++] = app;

	return app;
}
