//
// Created by victoria on 20.06.2020.
//

#include "server.h"
#include "vhttpsl/server.h"
#include "socket_context.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <conf/config.h>
#include <errno.h>

vhttpsl_server_t
vhttpsl_server_create(vhttpsl_app_t app)
{
	vhttpsl_server_t server = calloc(1, sizeof(*server));

	fprintf(stderr, "TODO: vhttpsl_server_create\n");

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
	in_port_t in_port;
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

	listen(socket_fd, LISTEN_BACKLOG);

	server->socket_fd = socket_fd;

	event.events = EPOLLIN;
	event.data.ptr = socket_context_create(socket_fd, SCT_SERVER_SOCKET).ptr;

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

int
vhttpsl_server_poll(vhttpsl_server_t server)
{
	const int MAX_EVENTS = 16;
	int event_count, i, client_fd, read_ln, event_fd;
	struct epoll_event events[MAX_EVENTS], event;
	struct sockaddr addr;
	socklen_t addr_len;
	socket_context_t ctx;
	int needs_to_write = 0;

	if (server->needs_to_write)
	{
		event.events |= (unsigned) EPOLLOUT;
		epoll_ctl(server->epoll_fd, EPOLL_CTL_MOD, server->socket_fd, &event);
	}

	memset(events, 0, sizeof(events));

	event_count = epoll_wait(server->epoll_fd, events, MAX_EVENTS, -1);
	if (event_count < 0)
	{
		perror("epoll_wait in vhttpsl_server_poll");

		return EXIT_FAILURE;
	}

	for (i = 0; i < event_count; ++i)
	{
		ctx = (socket_context_t) events[i].data.ptr;
		event_fd = ctx.ctx->fd;

		if (event_fd == server->socket_fd)
		{
			/* client has connected */
			client_fd = accept(server->socket_fd, &addr, &addr_len);
			if (client_fd < 0)
			{
				perror("accept in vhttpsl_server_poll");

				return EXIT_FAILURE;
			}

			fprintf(stdout, "client has connected: %d\n", client_fd);
			fcntl(client_fd, F_SETFL, O_NONBLOCK);

			event.data.ptr = socket_context_create(client_fd, SCT_CLIENT_SOCKET).ptr;
			event.events = EPOLLIN;

			if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0)
			{
				perror("epoll_ctl for client socket in vhttpsl_server_poll");

				return EXIT_FAILURE;
			}
		}
		else
		{
			fprintf(stderr, "TODO: fd ready %d / %u\n", event_fd, events[i].events);

			bytebuf_ensure_write(&ctx.cl->read_buffer, 128);
			read_ln = read(event_fd, bytebuf_write_ptr(&ctx.cl->read_buffer), 128 /* TODO */);

			if (read_ln < 0)
			{
				perror("read()");

				socket_context_destroy(ctx);
				close(event_fd);

				continue;
			}

			if (read_ln == 0)
			{
				fprintf(stderr, "TODO: client %d has disconnected\n", event_fd);

				socket_context_destroy(ctx);
				close(event_fd);

				continue;
			}

			ctx.cl->read_buffer.pos_write += read_ln;

			fprintf(stdout, "read %d bytes from the client, total read = %u\n", read_ln,
					(unsigned) ctx.cl->read_buffer.pos_write);

			bytebuf_ensure_write(&ctx.cl->write_buffer, 128);
			memset(bytebuf_write_ptr(&ctx.cl->write_buffer), 'a', 128);
			ctx.cl->write_buffer.pos_write += 128;

			while (ctx.cl->write_buffer.pos_read < ctx.cl->write_buffer.pos_write)
			{
				read_ln = write(event_fd, bytebuf_read_ptr(&ctx.cl->write_buffer),
								ctx.cl->write_buffer.pos_write - ctx.cl->write_buffer.pos_read);

				if (read_ln < 0)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
						needs_to_write = 1;
					else
						perror("write()");

					continue;
				}
				if (read_ln == 0)
				{
					needs_to_write = 1;

					break;
				}

				ctx.cl->write_buffer.pos_read += read_ln;
			}

			/* TODO */
		}
	}

	if (!needs_to_write && server->needs_to_write)
	{
		server->needs_to_write = 0;
		event.events &= ~(unsigned) EPOLLOUT;
		epoll_ctl(server->epoll_fd, EPOLL_CTL_MOD, server->socket_fd, &event);
	}

	return EXIT_SUCCESS;
}
