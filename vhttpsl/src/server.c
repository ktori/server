/*
 * Created by victoria on 20.06.2020.
 */

#include "server.h"
#include "vhttpsl/server.h"
#include "socket_context.h"

#include <streams/streams.h>
#include <streams/backend/fd.h>
#include <streams/buffered_pipe.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

vhttpsl_server_t
vhttpsl_server_create(vhttpsl_app_t app)
{
	vhttpsl_server_t server = calloc(1, sizeof(*server));

	server->app = app;

	return server;
}

void
vhttpsl_server_destroy(vhttpsl_server_t *server)
{
	fprintf(stderr, "TODO: vhttpsl_server_destroy\n");

	free(*server);
	*server = NULL;
}

int
vhttpsl_server_listen_http(vhttpsl_server_t server, int port)
{
	struct timeval tv;

	char port_s[16];
	struct addrinfo hints, *info = 0, *j;
	int status, socket_fd;
	int on = 1;
	struct epoll_event event;

	server->socket_fd = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	snprintf(port_s, 16, "%d", port);
	status = getaddrinfo(0, port_s, &hints, &info);
	if (status != 0)
	{
		printf("[error] getaddrinfo returned %d\n", status);
		return 1;
	}

	for (j = info; j != 0; j = j->ai_next)
	{
		socket_fd = socket(j->ai_family, j->ai_socktype, j->ai_protocol);
		if (socket_fd < 0)
		{
			printf("socket() failed\n");
			continue;
		}

		tv.tv_sec = 8;
		tv.tv_usec = 0;
		setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

		status = bind(socket_fd, j->ai_addr, j->ai_addrlen);
		if (status < 0)
		{
			perror("bind");
			close(socket_fd);
			continue;
		}
		break;
	}
	if (info == 0)
	{
		printf("no addrinfo\n");
		return 1;
	}
	freeaddrinfo(info);

	fcntl(socket_fd, F_SETFL, O_NONBLOCK);

	listen(socket_fd, 128);

	server->socket_fd = socket_fd;

	event.events = EPOLLIN;
	event.data.ptr = socket_context_create(socket_fd, SCT_SERVER_SOCKET, server).ptr;

	server->epoll_fd = epoll_create1(0);
	if (server->epoll_fd < 0)
	{
		perror("epoll_create1");
		return EXIT_FAILURE;
	}

	if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) < 0)
	{
		perror("epoll_ctl");
		return EXIT_FAILURE;
	}

	printf("server (%d) is listening on port %d\n", server->socket_fd, port);

	return EXIT_SUCCESS;
}

int
vhttpsl_server_listen_https(vhttpsl_server_t server, int port)
{
	fprintf(stderr, "TODO: vhttpsl_server_listen_https\n");

	return EXIT_FAILURE;
}

#define BUFFER_SIZE 256

static int
process_client_event(struct epoll_event *event);

static int
process_server_event(struct epoll_event *event);

#define MAX_EVENTS 16

int
vhttpsl_server_poll(vhttpsl_server_t server)
{
	int event_count, i;
	struct epoll_event events[MAX_EVENTS];
	socket_context_t ctx;
	/*int needs_to_write = server->needs_to_write;
	server->needs_to_write = 0;*/

/*	if (needs_to_write)
	{
		event.events |= (unsigned) EPOLLOUT;
		epoll_ctl(server->epoll_fd, EPOLL_CTL_MOD, server->socket_fd, &event);
	}*/

	memset(events, 0, sizeof(events));

	event_count = epoll_wait(server->epoll_fd, events, MAX_EVENTS, -1);
	if (event_count < 0)
	{
		perror("epoll_wait in vhttpsl_server_poll");

		return EXIT_FAILURE;
	}

	for (i = 0; i < event_count; ++i)
	{
		ctx.ptr = events[i].data.ptr;

		switch (ctx.ctx->type)
		{
			case SCT_SERVER_SOCKET:
				process_server_event(events + i);
				break;
			case SCT_CLIENT_SOCKET:
				process_client_event(events + i);
				break;
			default:
				fprintf(stderr, "unknown type in vhttpsl_server_poll: %d\n", ctx.ctx->type);
				break;
		}
	}

/*	if (needs_to_write && !server->needs_to_write)
	{
		event.events &= ~(unsigned) EPOLLOUT;
		epoll_ctl(server->epoll_fd, EPOLL_CTL_MOD, server->socket_fd, &event);
	}*/

	return EXIT_SUCCESS;
}

static int
process_server_event(struct epoll_event *event)
{
	socket_context_t ctx;
	int client_fd;
	struct sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	struct epoll_event ev = {0};

	ctx.ptr = event->data.ptr;

	/* client has connected */
	client_fd = accept(ctx.ctx->server->socket_fd, &addr, &addr_len);
	if (client_fd < 0)
	{
		perror("accept in vhttpsl_server_poll");
		return EXIT_FAILURE;
	}

	fprintf(stdout, "client has connected: %d\n", client_fd);
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	/* create client context */
	ev.data.ptr = socket_context_create(client_fd, SCT_CLIENT_SOCKET, ctx.ctx->server).ptr;
	ev.events = EPOLLIN;

	if (epoll_ctl(ctx.ctx->server->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0)
	{
		perror("epoll_ctl for client socket in vhttpsl_server_poll");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int
process_client_event(struct epoll_event *event)
{
	socket_context_t ctx;
	int eof_read = 0;

	ctx.ptr = event->data.ptr;

	switch (stream_buffered_pipe_pass(&ctx.cl->pipe_in)) {
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

	switch (stream_buffered_pipe_pass(&ctx.cl->pipe_out)) {
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
	{
		printf("client %d: closing connection\n", ctx.ctx->fd);
		epoll_ctl(ctx.ctx->server->epoll_fd, EPOLL_CTL_DEL, ctx.ctx->fd, NULL);
		close(ctx.ctx->fd);
		socket_context_destroy(ctx);
	}

	return EXIT_SUCCESS;
}
