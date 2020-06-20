//
// Created by victoria on 20.06.2020.
//

#include <vhttpsl/server.h>

void
callback_root()
{

}

int
main(int argc, char **argv)
{
	vhttpsl_server_t server;

	server = vhttpsl_server_create();

	vhttpsl_server_add_route(server, "/", callback_root);

	vhttpsl_server_listen_http(server, 8080);

	vhttpsl_server_destroy(server);

	return 0;
}