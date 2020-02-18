/*
 * Created by victoria on 15.02.20.
*/

#include <signal.h>
#include "lib/config.h"
#include "server/server.h"
#include "server.h"
#include "cluster/cluster.h"
#include "shutdown.h"
#include "conf/servers.h"
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
	struct cluster_s cluster = {0};
	int status = EXIT_SUCCESS;

	graceful_shutdown_install();

	signal(SIGCHLD, SIG_IGN);

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

	if (load_servers(&cluster) != EXIT_SUCCESS)
	{
		fprintf(stderr, "failed to load servers\n");
		cluster_destroy(&cluster);
		return EXIT_FAILURE;
	}
	if (cluster_run(&cluster) != EXIT_SUCCESS)
	{
		fprintf(stderr, "failed to start servers\n");
		cluster_destroy(&cluster);
		return EXIT_FAILURE;
	}
	if (cluster_listen(&cluster) != EXIT_SUCCESS)
	{
		perror("cluster_listen()");
		status = EXIT_FAILURE;
	}
	if (cluster_destroy(&cluster) != EXIT_SUCCESS)
	{
		perror("cluster_destroy()");
		status = EXIT_FAILURE;
	}

	return status;
}
