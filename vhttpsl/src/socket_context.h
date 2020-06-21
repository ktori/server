//
// Created by victoria on 21.06.2020.
//

#pragma once

#include <vhttpsl/bits/bytebuf.h>

enum socket_context_type
{
	SCT_SERVER_SOCKET,
	SCT_CLIENT_SOCKET
};

struct socket_context_s
{
	int fd;
	enum socket_context_type type;
};

struct client_socket_context_s
{
	struct socket_context_s ctx;

	struct bytebuf_s read_buffer;
	struct bytebuf_s write_buffer;
};

struct server_socket_context_s
{
	struct socket_context_s ctx;
};

typedef union
{
	void *ptr;
	struct socket_context_s *ctx;
	struct client_socket_context_s *cl;
	struct server_socket_context_s *sv;
} socket_context_t;

socket_context_t
socket_context_create(int fd, enum socket_context_type type);

void
socket_context_destroy(socket_context_t context);