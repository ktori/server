/*
 * Created by victoria on 12/12/20.
 */

#include "listener_ctx.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vhttpsl/listener.h>

int
listener_destroy(listener_t listener)
{
	if (listener->epoll_owned)
		close(listener->epoll_fd);

	close(listener->fd);

	/* TODO: Destroy all open sessions */

	return EXIT_SUCCESS;
}

#define MAX_EVENTS 16

int
listeners_poll(int epoll_fd)
{
	int event_count, i;
	struct epoll_event events[MAX_EVENTS];
	lctx_t lctx;

	memset(events, 0, sizeof(events));

	event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
	if (event_count < 0)
	{
		perror("epoll_wait()");

		return EXIT_FAILURE;
	}

	for (i = 0; i < event_count; ++i)
	{
		lctx = events[i].data.ptr;

		if (lctx->magic != LCTX_MAGIC)
		{
			fprintf(stderr, "data header magic does not match: 0x%08x", lctx->magic);

			continue;
		}

		switch (lctx->type)
		{
			case LCTX_CLIENT:
				lctx_client_event(events + i);
				break;
			case LCTX_SERVER:
				lctx_server_event(events + i);
				break;
			default:
				fprintf(stderr, "unknown type in listener_update: %d\n", lctx->type);
				break;
		}
	}

	return EXIT_SUCCESS;
}
