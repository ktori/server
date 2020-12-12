/*
 * Created by victoria on 12/12/20.
 */

#pragma once

#include <vhttpsl/listener.h>

#define LCTX_MAGIC 0xC00FC00F

struct epoll_event;

enum lctx_type
{
	LCTX_SERVER = 1,
	LCTX_CLIENT = 2
};

typedef struct lctx_s
{
	unsigned magic;
	int fd;
	int type;
	listener_t listener;
	void *cl_data;
} * lctx_t;

lctx_t
lctx_sv_new(listener_t listener);

int
lctx_client_event(struct epoll_event *event);

int
lctx_server_event(struct epoll_event *event);
