/*
 * Created by victoria on 12/12/20.
 */

#pragma once

typedef int (*on_cl_accept_callback_t)(int fd, void *sv_data, void **cl_data);
/** must return -1 on error, 0 on success and 1 when done */
typedef int (*on_cl_event_callback_t)(int fd, void *sv_data, void *cl_data);
typedef int (*on_cl_close_callback_t)(int fd, void *sv_data, void *cl_data);
typedef int (*on_sv_destroy_callback_t)(void *sv_data);

struct listener_callbacks_s
{
	void *sv_data;

	on_cl_accept_callback_t on_cl_accept;
	on_cl_event_callback_t on_cl_event;
	on_cl_close_callback_t on_cl_close;
	on_sv_destroy_callback_t on_sv_destroy;
};

typedef struct listener_s
{
	int fd;
	int epoll_fd;
	int epoll_owned;
	struct listener_callbacks_s callbacks;
} * listener_t;

listener_t
listener_new(const char *name, int port, struct listener_callbacks_s callbacks, int epoll_fd);

int
listener_init(listener_t listener, const char *name, int port, struct listener_callbacks_s callbacks, int epoll_fd);

int
listener_destroy(listener_t listener);

int
listeners_poll(int epoll_fd);
