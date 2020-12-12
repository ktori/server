/*
 * Created by victoria on 20.06.2020.
 */

#pragma once

#include "app.h"

#include <openssl/ssl.h>

struct listener_s;

struct vhttpsl_server_s
{
	int epoll_fd;
	int epoll_owned;
	struct listener_s **listeners;
	size_t listeners_size;
	size_t listeners_count;

	vhttpsl_app_t *apps;
	size_t apps_count;
	size_t apps_size;
};
