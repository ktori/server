//
// Created by victoria on 20.06.2020.
//

#pragma once

typedef struct vhttpsl_app_s *vhttpsl_app_t;

struct vhttpsl_server_s
{
	int socket_fd;
	int epoll_fd;
	int needs_to_write;
	vhttpsl_app_t app;
};