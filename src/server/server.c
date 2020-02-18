/*
 * Created by victoria on 15.02.20.
*/
#include "server.h"
#include "../lib/kv.h"
#include <conf/config.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

#if SERVER_USE_SSL

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/ioctl.h>

#endif

static in_port_t
get_in_port(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return (((struct sockaddr_in *) sa)->sin_port);
	}
	return (((struct sockaddr_in6 *) sa)->sin6_port);
}

static int
server_setup_ssl(struct server_s *server)
#if SERVER_USE_SSL
{
	SSL_CTX *ssl_ctx = NULL;
	const SSL_METHOD *method = TLS_server_method();

	if (!server->config->ssl)
	{
		server->ssl_ctx = NULL;
		return EXIT_SUCCESS;
	}

	ssl_ctx = SSL_CTX_new(method);
	if (ssl_ctx == NULL)
	{
		perror("unable to create ssl context");
		ERR_print_errors_fp(stderr);

		return EXIT_FAILURE;
	}

	SSL_CTX_set_ecdh_auto(ssl_ctx, 1);

	if (server->config->ssl_cert == NULL)
	{
		fprintf(stderr, "ssl.cert not set\n");
		return EXIT_FAILURE;
	}
	if (server->config->ssl_key == NULL)
	{
		fprintf(stderr, "ssl.key not set\n");
		return EXIT_FAILURE;
	}
	if (SSL_CTX_use_certificate_file(ssl_ctx, server->config->ssl_cert, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return EXIT_FAILURE;
	}
	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, server->config->ssl_key, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return EXIT_FAILURE;
	}

	server->ssl_ctx = ssl_ctx;

	return EXIT_SUCCESS;
}

#else
{
	return EXIT_SUCCESS;
}

#endif

int
server_setup(struct server_s *server, struct server_config_s *config)
{
	struct timeval tv;

	char port_s[16];
	struct addrinfo hints, *info = 0, *j;
	int status, sockfd;
	in_port_t port;
	int on = 1;

	server->is_running = 0;
	server->sock_fd = -1;
	server->config = config;
	server_setup_ssl(server);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	snprintf(port_s, 16, "%d", config->port);
	status = getaddrinfo(0, port_s, &hints, &info);
	if (status != 0)
	{
		printf("[error] getaddrinfo returned %d\n", status);
		return 1;
	}

	for (j = info; j != 0; j = j->ai_next)
	{
		sockfd = socket(j->ai_family, j->ai_socktype, j->ai_protocol);
		if (sockfd == -1)
		{
			printf("socket() failed\n");
			continue;
		}

		tv.tv_sec = 8;
		tv.tv_usec = 0;
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
		ioctl(sockfd, FIONBIO, &on);

		status = bind(sockfd, j->ai_addr, j->ai_addrlen);
		if (status == -1)
		{
			printf("bind() failed, errno %d\n", errno);
			close(sockfd);
			continue;
		}
		break;
	}
	if (info == 0)
	{
		printf("no addrinfo\n");
		return 1;
	}
	port = ntohs(get_in_port((struct sockaddr *) info->ai_addr));
	freeaddrinfo(info);

	/* fnctl(sockfd, F_SETFL, O_NONBLOCK); */

	listen(sockfd, LISTEN_BACKLOG);

	server->sock_fd = sockfd;
	server->is_running = 1;

	printf("server (%d) is listening on port %hd\n", server->sock_fd, port);

	return EXIT_SUCCESS;
}

int
server_cleanup(struct server_s *server)
{
	printf("cleaning up\n");
	shutdown(server->sock_fd, SHUT_RDWR);
	close(server->sock_fd);
#if SERVER_USE_SSL
	SSL_CTX_free(server->ssl_ctx);
#endif

	return EXIT_SUCCESS;
}
