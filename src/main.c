/*
 * Created by victoria on 15.02.20.
*/

#include <signal.h>
#include "config.h"
#include "server/server.h"
#include "server.h"
#include <stdlib.h>
#include <stdio.h>
#include <src/options.h>

#if SERVER_USE_SSL
#include <openssl/ssl.h>
#endif

int
ssl_setup()
{
#if SERVER_USE_SSL
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
#endif
	return EXIT_SUCCESS;
}

int
ssl_cleanup()
{
#if SERVER_USE_SSL
	EVP_cleanup();
#endif
	return EXIT_SUCCESS;
}

int
main(int argc, const char **argv)
{
	/*signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGSEGV, sighandler);*/
	signal(SIGCHLD, SIG_IGN);
	global_config = config_load("server.conf");

	setup_document_root();

	struct server_s server;

#if SERVER_USE_SSL
	if (ssl_setup() != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
#endif
	if (server_setup(&server, global_config) != EXIT_SUCCESS)
	{
		fprintf(stderr, "srv_setup failed\n");
		return 1;
	}
	if (server_listen(&server) != EXIT_SUCCESS)
	{
		fprintf(stderr, "srv_listen failed\n");
	}
	if (server_cleanup(&server) != EXIT_SUCCESS)
	{
		fprintf(stderr, "srv_cleanup failed\n");
	}

	return EXIT_SUCCESS;
}