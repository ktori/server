/*
 * Created by victoria on 21.06.2020.
 */

#pragma once

#include <vhttpsl/bits/bytebuf.h>

#include "http/session.h"

#define SOC_BUF_SIZE_IN 256
#define SOC_BUF_SIZE_OUT 256

struct vhttpsl_server_s;

enum socket_context_type
{
	SCT_SERVER_SOCKET,
	SCT_CLIENT_SOCKET
};

struct socket_context_s
{
	int fd;
	enum socket_context_type type;
	struct vhttpsl_server_s *server;
};

struct client_socket_context_s
{
	struct socket_context_s ctx;

	struct http_session_s session;

	struct
	{
		char data[SOC_BUF_SIZE_IN];
		size_t count;
		size_t offset;
	} buf_in;

	struct
	{
		char data[SOC_BUF_SIZE_OUT];
		size_t count;
		size_t offset;
	} buf_out;
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
socket_context_create(int fd, enum socket_context_type type, struct vhttpsl_server_s *server);

void
socket_context_destroy(socket_context_t context);
