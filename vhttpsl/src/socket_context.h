//
// Created by victoria on 21.06.2020.
//

#pragma once

enum socket_context_type
{
	SCT_SERVER_SOCKET,
	SCT_CLIENT_SOCKET
};

typedef struct socket_context_s
{
	int fd;
	enum socket_context_type type;
} *socket_context_t;

socket_context_t
socket_context_create(int fd, enum socket_context_type type);

void
socket_context_destroy(socket_context_t context);