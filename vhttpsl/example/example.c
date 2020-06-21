//
// Created by victoria on 20.06.2020.
//

#include <vhttpsl/app.h>
#include <vhttpsl/server.h>

void
callback_root()
{

}

int
main(int argc, char **argv)
{
	vhttpsl_app_t app;
	vhttpsl_server_t server;

	app = vhttpsl_app_create();

	vhttpsl_app_route_add(app, "/", callback_root);

	server = vhttpsl_server_create(app);

	vhttpsl_server_listen_http(server, 8080);

	while (vhttpsl_server_poll(server) == 0);

	vhttpsl_server_destroy(&server);

	vhttpsl_app_destroy(&app);

	return 0;
}