/*
 * Created by victoria on 15.02.20.
*/

#include <signal.h>
#include "lib/config.h"
#include "server/server.h"
#include "server.h"
#include "cluster/cluster.h"
#include <stdlib.h>
#include <stdio.h>
#include <conf/config.h>

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
	struct server_s server = {0};
	struct server_s server_ssl = {0};
	struct cluster_s cluster = {0};

	/*signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGSEGV, sighandler);*/
	signal(SIGCHLD, SIG_IGN);
	global_config = config_load("server.conf");

	setup_document_root();

	if (cluster_init(&cluster) != EXIT_SUCCESS)
	{
		perror("cluster_init()");
		return EXIT_FAILURE;
	}

#if SERVER_USE_SSL
	if (ssl_setup() != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
#endif
	if (server_setup(&server, global_config) != EXIT_SUCCESS)
	{
		perror("server_setup()");
		return EXIT_FAILURE;
	}
	if (cluster_add(&cluster, &server) != EXIT_SUCCESS)
	{
		perror("cluster_add()");
		return EXIT_FAILURE;
	}
	if (server_setup(&server_ssl, config_load("server-ssl.conf")) != EXIT_SUCCESS)
	{
		perror("server_setup()");
		return EXIT_FAILURE;
	}
	if (cluster_add(&cluster, &server_ssl) != EXIT_SUCCESS)
	{
		perror("cluster_add()");
		return EXIT_FAILURE;
	}
	if (cluster_run(&cluster) != EXIT_SUCCESS)
	{
		perror("cluster_run()");
		return EXIT_FAILURE;
	}
	if (cluster_destroy(&cluster) != EXIT_SUCCESS)
	{
		perror("cluster_destroy()");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}