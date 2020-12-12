/*
 * Created by victoria on 12/12/20.
 */

#include "listener_ctx.h"

#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vhttpsl/listener.h>

static int
set_socket_timeout(int fd, int seconds)
{
	struct timeval tv;

	tv.tv_sec = seconds;
	tv.tv_usec = 0;

	return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
}

static int
create_socket(const char *name, int port)
{
	char port_s[16];
	struct addrinfo hints, *info = 0, *j;
	int status;
	int fd = -1;
	int on = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

	snprintf(port_s, 16, "%d", port);

	status = getaddrinfo(name, port_s, &hints, &info);
	if (status != 0)
	{
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(status));
		return -1;
	}

	for (j = info; j != 0; j = j->ai_next)
	{
		fd = socket(j->ai_family, j->ai_socktype, j->ai_protocol);
		if (fd < 0)
		{
			perror("socket()");
			continue;
		}

		set_socket_timeout(fd, 8); /* FIXME: Hardcode */
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

		if (bind(fd, j->ai_addr, j->ai_addrlen) < 0)
		{
			perror("bind()");
			close(fd);
			fd = -1;
			continue;
		}

		break;
	}

	freeaddrinfo(info);

	return fd;
}

listener_t
listener_new(const char *name, int port, struct listener_callbacks_s callbacks, int epoll_fd)
{
	listener_t result = calloc(1, sizeof(*result));

	if (!result)
		return NULL;

	if (listener_init(result, name, port, callbacks, epoll_fd))
	{
		free(result);

		return NULL;
	}

	return result;
}

int
listener_init(listener_t listener, const char *name, int port, struct listener_callbacks_s callbacks, int epoll_fd)
{
	int socket_fd;
	struct epoll_event event;

	socket_fd = create_socket(name, port);
	if (socket_fd < 0)
		return EXIT_FAILURE;

	fcntl(socket_fd, F_SETFL, O_NONBLOCK);

	if (listen(socket_fd, 128))
	{
		perror("listen()");
		close(socket_fd);

		return EXIT_FAILURE;
	}

	if (epoll_fd < 0)
	{
		listener->epoll_owned = 1;
		epoll_fd = epoll_create1(0);
	}
	else
		listener->epoll_owned = 0;

	if (epoll_fd < 0)
	{
		perror("epoll_create1()");
		close(socket_fd);

		return EXIT_FAILURE;
	}

	listener->fd = socket_fd;
	listener->callbacks = callbacks;

	event.events = EPOLLIN;
	event.data.ptr = lctx_sv_new(listener);

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event))
	{
		perror("epoll_ctl()");
		close(epoll_fd);
		close(socket_fd);

		return EXIT_FAILURE;
	}

	listener->epoll_fd = epoll_fd;

	return EXIT_SUCCESS;
}
