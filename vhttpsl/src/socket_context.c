/*
 * Created by victoria on 21.06.2020.
 */

#include "socket_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <streams/backend/fd.h>

socket_context_t
client_context_create(int fd, struct vhttpsl_server_s *server)
{
	socket_context_t ctx = { 0 };
	struct client_socket_context_s *cl = calloc(1, sizeof(*cl));

	ctx.cl = cl;
	ctx.ctx->fd = fd;
	ctx.ctx->type = SCT_CLIENT_SOCKET;
	ctx.ctx->server = server;

	stream_init(&cl->socket_stream);
	stream_init(&cl->http_stream);

	fd_backend(&cl->socket_stream.backend, fd, fd, 0);

	http_session_init(&cl->session, server);

	stream_backend_init(&cl->http_stream.backend,
						&cl->session,
						(stream_backend_read_fn)http_session_read,
						(stream_backend_write_fn)http_session_write,
						NULL);

	stream_buffered_pipe_init(&cl->pipe_in, &cl->socket_stream, &cl->http_stream, SOC_BUF_SIZE);
	stream_buffered_pipe_init(&cl->pipe_out, &cl->http_stream, &cl->socket_stream, SOC_BUF_SIZE);

	return ctx;
}

socket_context_t
server_context_create(int fd, struct vhttpsl_server_s *server)
{
	socket_context_t ctx = { 0 };

	ctx.sv = calloc(1, sizeof(*ctx.sv));
	ctx.ctx->fd = fd;
	ctx.ctx->type = SCT_SERVER_SOCKET;
	ctx.ctx->server = server;

	return ctx;
}

socket_context_t
socket_context_create(int fd, enum socket_context_type type, struct vhttpsl_server_s *server)
{
	socket_context_t ctx = { 0 };

	switch (type)
	{
		case SCT_CLIENT_SOCKET:
			return client_context_create(fd, server);
		case SCT_SERVER_SOCKET:
			return server_context_create(fd, server);
		default:
			fprintf(stderr, "socket_context_create: unknown type %d\n", type);
			ctx.ctx = NULL;

			return ctx;
	}
}

void
socket_context_destroy(socket_context_t context)
{
	switch (context.ctx->type)
	{
		case SCT_CLIENT_SOCKET:
			http_session_destroy(&context.cl->session);
			stream_buffered_pipe_destroy(&context.cl->pipe_in);
			stream_buffered_pipe_destroy(&context.cl->pipe_out);
			stream_backend_destroy(&context.cl->socket_stream.backend);
			stream_backend_destroy(&context.cl->http_stream.backend);
			stream_destroy(&context.cl->http_stream);
			stream_destroy(&context.cl->socket_stream);

			break;
		case SCT_SERVER_SOCKET:
			break;
		default:
			fprintf(
			  stderr, "fatal: socket_context_destroy: unknown type %d, fd=%d\n", context.ctx->type, context.ctx->fd);

			exit(1);
	}

	free(context.ctx);
}