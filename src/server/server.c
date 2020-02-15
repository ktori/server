/*
 * Created by victoria on 15.02.20.
*/
#include "server.h"
#include "../kv.h"
#include <src/options.h>

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
	ssl_ctx = SSL_CTX_new(method);
	if (ssl_ctx == NULL)
	{
		perror("unable to create ssl context");
		ERR_print_errors_fp(stderr);

		return EXIT_FAILURE;
	}

	SSL_CTX_set_ecdh_auto(ssl_ctx, 1);

	const char *cert_path = kv_string(server->config, "ssl.cert", NULL);
	if (cert_path == NULL)
	{
		fprintf(stderr, "ssl.cert not set\n");
		return EXIT_FAILURE;
	}
	const char *key_path = kv_string(server->config, "ssl.key", NULL);
	if (key_path == NULL)
	{
		fprintf(stderr, "ssl.key not set\n");
		return EXIT_FAILURE;
	}
	if (SSL_CTX_use_certificate_file(ssl_ctx, cert_path, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return EXIT_FAILURE;
	}
	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key_path, SSL_FILETYPE_PEM) <= 0)
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
server_setup(struct server_s *server, struct kv_list_s *config)
{
	server->config = config;
	server_setup_ssl(server);

	struct addrinfo hints, *info = 0, *j;
	int status, sockfd;
	in_port_t port;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(0, kv_string(config, "port", "8080"), &hints, &info);
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

	struct timeval tv;
	tv.tv_sec = 8;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));
	/* fnctl(sockfd, F_SETFL, O_NONBLOCK); */

	listen(sockfd, LISTEN_BACKLOG);

	server->sock_fd = sockfd;

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
