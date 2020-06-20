//
// Created by victoria on 20.06.2020.
//

#include "vhttpsl/server.h"

#include <stdlib.h>
#include <stdio.h>

vhttpsl_server_t
vhttpsl_server_create()
{
	fprintf(stderr, "TODO: vhttpsl_server_create\n");

	return NULL;
}

void
vhttpsl_server_destroy(vhttpsl_server_t server)
{
	fprintf(stderr, "TODO: vhttpsl_server_destroy\n");
}

void
vhttpsl_server_add_route(vhttpsl_server_t server, const char *route, vhttpsl_callback_t callback)
{
	fprintf(stderr, "TODO: vhttpsl_server_add_route\n");
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