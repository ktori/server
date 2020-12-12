/*
 * Created by victoria on 20.06.2020.
 */

#pragma once

#include <openssl/ssl.h>

struct listener_s;
struct vhttpsl_app_s;

struct vhttpsl_server_s
{
	int epoll_fd;
	struct vhttpsl_app_s *app;
	struct listener_s **listeners;
	size_t listeners_size;
	size_t listeners_count;
};
