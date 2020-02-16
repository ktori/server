/*
 * Created by victoria on 15.02.20.
*/
#pragma once

#include <conf/config.h>

#include <sys/socket.h>

#if SERVER_USE_SSL

#include <openssl/ossl_typ.h>

#endif

struct server_s;

struct client_s
{
	int socket;
	struct sockaddr_storage addr;
	socklen_t addr_size;
#if SERVER_USE_SSL
	SSL *ssl;
#endif
};

int
client_accept(struct server_s *server, struct client_s *client);

int
client_setup(struct server_s *server, struct client_s *client);

int
client_read(struct client_s *client, char **out, size_t *out_length);

int
client_read_some(struct client_s *client, char *out, size_t buffer_size, size_t *out_length);

int
client_write(struct client_s *client, const char *data, size_t length);

int
client_close(struct client_s *client);
