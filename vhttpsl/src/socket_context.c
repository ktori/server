//
// Created by victoria on 21.06.2020.
//

#include "socket_context.h"

#include <stdlib.h>

socket_context_t
socket_context_create(int fd, enum socket_context_type type)
{
	socket_context_t ctx = calloc(1, sizeof(*ctx));

	ctx->fd = fd;
	ctx->type = type;

	return ctx;
}

void
socket_context_destroy(socket_context_t context)
{
	free(context);
}