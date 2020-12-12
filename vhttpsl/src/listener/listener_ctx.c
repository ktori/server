/*
 * Created by victoria on 12/12/20.
 */

#include "listener_ctx.h"

#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

lctx_t
lctx_cl_new(lctx_t server, int fd)
{
	lctx_t result = calloc(1, sizeof(*result));

	if (!result)
		return NULL;

	result->magic = LCTX_MAGIC;
	result->type = LCTX_CLIENT;
	result->fd = fd;
	result->listener = server->listener;
	server->listener->callbacks.on_cl_accept(fd, server->listener->callbacks.sv_data, &result->cl_data);

	return result;
}

lctx_t
lctx_sv_new(listener_t listener)
{
	lctx_t result = calloc(1, sizeof(*result));

	if (!result)
		return NULL;

	result->magic = LCTX_MAGIC;
	result->type = LCTX_SERVER;
	result->fd = listener->fd;
	result->listener = listener;
	result->cl_data = NULL;

	return result;
}

void
lctx_cl_delete(lctx_t lctx)
{
	lctx->listener->callbacks.on_cl_close(lctx->fd, lctx->listener->callbacks.sv_data, lctx->cl_data);

	free(lctx);
}

int
lctx_server_event(struct epoll_event *event)
{
	lctx_t data;
	int client_fd;
	struct sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	struct epoll_event ev = { 0 };

	data = event->data.ptr;

	/* client has connected */
	client_fd = accept(data->fd, &addr, &addr_len);
	if (client_fd < 0)
	{
		perror("accept in vhttpsl_server_poll");
		return EXIT_FAILURE;
	}

	fprintf(stdout, "client has connected: %d\n", client_fd);
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	/* create client context */
	ev.data.ptr = lctx_cl_new(data, client_fd);
	ev.events = EPOLLIN;

	if (epoll_ctl(data->listener->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0)
	{
		perror("epoll_ctl for client socket in vhttpsl_server_poll");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int
lctx_client_event(struct epoll_event *event)
{
	lctx_t data;
	int ret, fd;

	data = event->data.ptr;

	ret = data->listener->callbacks.on_cl_event(data->fd, data->listener->callbacks.sv_data, data->cl_data);

	if (ret < 0)
	{
		fprintf(stderr, "on_event returned %d\n", ret);

		return EXIT_FAILURE;
	}

	if (ret > 0)
	{
		if (epoll_ctl(data->listener->epoll_fd, EPOLL_CTL_DEL, data->fd, NULL) < 0)
		{
			perror("epoll_ctl removing from set");

			return EXIT_FAILURE;
		}

		fd = data->fd;
		lctx_cl_delete(data);

		close(fd);
	}

	return EXIT_SUCCESS;
}
