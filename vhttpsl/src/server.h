/*
 * Created by victoria on 20.06.2020.
 */

#pragma once

struct vhttpsl_app_s;

struct vhttpsl_server_s
{
	int socket_fd;
	int epoll_fd;
	int needs_to_write;
	struct vhttpsl_app_s *app;
};