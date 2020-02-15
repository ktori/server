/*
 * Created by victoria on 15.02.20.
*/
#pragma once

#include <src/options.h>

#include "client.h"

#if SERVER_USE_SSL
#include <openssl/ossl_typ.h>
#endif

struct server_s
{
	int is_running;
	int sock_fd;
	struct kv_list_s *config;
#if SERVER_USE_SSL
	SSL_CTX *ssl_ctx;
#endif
};

struct client_s;

struct sockaddr_storage;

int
server_setup(struct server_s *server, struct kv_list_s *config);

int
server_listen(struct server_s *server);

int
server_accept(struct server_s *server, struct client_s *client);

int
server_cleanup(struct server_s *server);
