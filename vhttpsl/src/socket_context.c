//
// Created by victoria on 21.06.2020.
//

#include "socket_context.h"

#include <stdlib.h>
#include <stdio.h>

socket_context_t
socket_context_create(int fd, enum socket_context_type type, struct vhttpsl_server_s *server)
{
	socket_context_t ctx = {};

	switch (type)
	{
		case SCT_CLIENT_SOCKET:
			ctx.cl = calloc(1, sizeof(*ctx.cl));
			ctx.ctx->fd = fd;
			ctx.ctx->type = SCT_CLIENT_SOCKET;
			ctx.ctx->server = server;

			http_session_init(&ctx.cl->session, server);

			return ctx;
		case SCT_SERVER_SOCKET:
			ctx.sv = calloc(1, sizeof(*ctx.sv));
			ctx.ctx->fd = fd;
			ctx.ctx->type = SCT_SERVER_SOCKET;
			ctx.ctx->server = server;

			return ctx;
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

			break;
		case SCT_SERVER_SOCKET:
			break;
		default:
			fprintf(stderr, "fatal: socket_context_destroy: unknown type %d, fd=%d\n", context.ctx->type,
					context.ctx->fd);

			exit(1);
	}

	free(context.ctx);
}