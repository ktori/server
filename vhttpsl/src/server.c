//
// Created by victoria on 20.06.2020.
//

#include "server.h"
#include "vhttpsl/server.h"

#include <stdlib.h>
#include <stdio.h>

vhttpsl_server_t
vhttpsl_server_create(vhttpsl_app_t app)
{
	vhttpsl_server_t server = calloc(1, sizeof(*server));

	fprintf(stderr, "TODO: vhttpsl_server_create\n");

	return server;
}

void
vhttpsl_server_destroy(vhttpsl_server_t *server)
{
	fprintf(stderr, "TODO: vhttpsl_server_destroy\n");

	free(*server);
	*server = NULL;
}

void
vhttpsl_server_listen_http(vhttpsl_server_t server, int port)
{
	fprintf(stderr, "TODO: vhttpsl_server_listen_http\n");
}

void
vhttpsl_server_listen_https(vhttpsl_server_t server, int port)
{
	fprintf(stderr, "TODO: vhttpsl_server_listen_https\n");
}