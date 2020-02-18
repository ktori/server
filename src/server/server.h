/*
 * Created by victoria on 15.02.20.
*/
#pragma once

#include <conf/config.h>

#include "client.h"
#include "../lib/config.h"

#if SERVER_USE_SSL
#include <openssl/ossl_typ.h>
#endif

struct server_s
{
	int is_running;
	int sock_fd;
	struct server_config_s config;
#if SERVER_USE_SSL
	SSL_CTX *ssl_ctx;
#endif
};

struct client_s;

struct sockaddr_storage;

int
server_start(struct server_s *server);

int
server_listen(struct server_s *server);

int
server_accept(struct server_s *server, struct client_s *client);

int
server_cleanup(struct server_s *server);
