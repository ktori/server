#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "cgi.h"
#include "config.h"
#include "http.h"
#include "mime.h"
#include "path.h"
#include "server.h"
#include "url.h"
#include "serve/serve.h"
#include <src/options.h>

#if SERVER_USE_SSL

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#endif

#define LISTEN_BACKLOG 10
#define FORK_CLIENTS TRUE

char *documentroot;
#if SERVER_USE_SSL
SSL_CTX *ssl_ctx;
#endif

/*
 *  TODO Content-Length
 */
#if SERVER_USE_SSL

int
recv_http(SSL *ssl, char **buf)
#else

int
recv_http(int sock, char **buf)
#endif
{
	int total = 0, received = 0, size = 128;
	char *buffer;

	buffer = malloc(size);

#if SERVER_USE_SSL
	received = SSL_read(ssl, buffer, size - 1);
#else
	received = recv(sock, buffer, size - 1, 0);
#endif

	while (received >= 0)
	{
		if (received == 0)
		{
			*buf = buffer;
			return total;
		}
		else
		{
			total += received;
			if (strstr(buffer, "\r\n\r\n") != NULL)
			{
				/* TODO POST request (body) */
				*buf = buffer;
				return total;
			}
			if (total >= size - 1)
			{
				size *= 2;
				buffer = realloc(buffer, size);
			}
		}
#if SERVER_USE_SSL
		received = SSL_read(ssl, buffer + total, size - total - 1);
#else
		received = recv(sock, buffer + total, size - total - 1, 0);
#endif
	}
	free(buffer);
	return received;
}

int
recv_all(int sock, char *buffer, int length)
{
	return recv(sock, buffer, length, 0);
	/*int total = 0;
	int received;
	do {
		received = recv(sock, buffer, length, 0);
		printf("received %d\n", received);
		if (received > 0) {
			buffer += received;
			length -= received;
			total += received;
		}
	} while (received > 0 && length > 0);
	return total;*/
}

#if SERVER_USE_SSL

int
send_all(SSL *ssl, char *buffer, int length)
{
	int sent;
	while (length > 0)
	{
		sent = SSL_write(ssl, buffer, length);
		if (sent < 1)
			return 1;
		buffer += sent;
		length -= sent;
	}
	return 0;
}

#else

int
send_all(int sock, char *buffer, int length)
{
	int sent;
	while (length > 0)
	{
		sent = send(sock, buffer, length, 0);
		if (sent < 1)
			return 1;
		buffer += sent;
		length -= sent;
	}
	return 0;
}

#endif

int server_socket = -1;
char server_running = 1;

void
sighandler(int signum)
{
	printf("Received signal %d\n", signum);
	if (server_socket >= 0)
		srv_cleanup();
	exit(0);
}

in_port_t
get_in_port(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return (((struct sockaddr_in *) sa)->sin_port);
	}
	return (((struct sockaddr_in6 *) sa)->sin6_port);
}

int
get_client_addr(int sockfd, char **addr_out, int *port)
{
	struct sockaddr_storage addr;
	socklen_t addrlen;
	char *addr_s;

	addrlen = sizeof(addr);

	if (getpeername(sockfd, (struct sockaddr *) &addr, &addrlen) != SUCCESS)
	{
		return FAILURE;
	}
	addr_s = malloc(64);
	if (addr.ss_family == AF_INET)
	{
		*port = ((struct sockaddr_in *) &addr)->sin_port;
		inet_ntop(AF_INET, &((struct sockaddr_in *) &addr)->sin_addr, addr_s, 64);
	}
	else
	{
		*port = ((struct sockaddr_in6 *) &addr)->sin6_port;
		inet_ntop(AF_INET, &((struct sockaddr_in6 *) &addr)->sin6_addr, addr_s, 64);
	}

	*addr_out = addr_s;

	return SUCCESS;
}

int
srv_setup(int *sockfd_out)
{
#if SERVER_USE_SSL
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	const SSL_METHOD *method = TLS_server_method();
	ssl_ctx = SSL_CTX_new(method);
	if (ssl_ctx == NULL)
	{
		perror("unable to create ssl context");
		ERR_print_errors_fp(stderr);
		return EXIT_FAILURE;
	}
	SSL_CTX_set_ecdh_auto(ssl_ctx, 1);

	const char *cert_path = kv_string(global_config, "ssl.cert", NULL);
	if (cert_path == NULL)
	{
		fprintf(stderr, "ssl.cert not set\n");
		return -1;
	}
	const char *key_path = kv_string(global_config, "ssl.key", NULL);
	if (key_path == NULL)
	{
		fprintf(stderr, "ssl.key not set\n");
		return -1;
	}
	if (SSL_CTX_use_certificate_file(ssl_ctx, cert_path, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return -1;
	}
	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key_path, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return -1;
	}
#endif

	struct addrinfo hints, *info = 0, *j;
	int status, sockfd;
	in_port_t port;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(0, kv_string(global_config, "port", "8080"), &hints, &info);
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
	*sockfd_out = sockfd;

	printf("server (%d) is listening on port %hd\n", *sockfd_out, port);
	return 0;
}

int
srv_process_socket(int sockfd, struct sockaddr_storage *addr)
{
	struct http_response_s *response;
	struct http_request_s *request;
	FILE *rq_log;
	char *rq_buffer;
	char *rs_buffer;
	int response_length;
	int request_length;

#if SERVER_USE_SSL
	SSL *ssl = SSL_new(ssl_ctx);
	SSL_set_fd(ssl, sockfd);
	if (SSL_accept(ssl) <= 0)
	{
		ERR_print_errors_fp(stderr);
		SSL_shutdown(ssl);
		SSL_free(ssl);
		return -1;
	}
	request_length = recv_http(ssl, &rq_buffer);
#else
	request_length = recv_http(sockfd, &rq_buffer);
#endif

	if (request_length >= 0)
	{
		rq_log = fopen("requests.log", "a");
		request = http_request_from_buffer(rq_buffer, request_length);
		fprintf(rq_log, "--- REQUEST ---\n\n%s\n", rq_buffer);
		fclose(rq_log);
		request->sockfd = sockfd;
	}
	else
	{
		request = NULL;
	}

	response = calloc(1, sizeof(struct http_response_s));
	response->headers = kv_create();

	serve(request, response);

	response_length = http_response_length(response);
	rs_buffer = calloc(response_length + 2, 1);
	http_response_to_buffer(response, rs_buffer, response_length + 1);
#if SERVER_USE_SSL
	send_all(ssl, rs_buffer, response_length);
#else
	send_all(sockfd, rs_buffer, response_length);
#endif

	if (request_length >= 0)
		free(rq_buffer);
	free(rs_buffer);

	http_response_free(response);
	if (request != NULL)
		http_request_free(request);

#if SERVER_USE_SSL
	SSL_shutdown(ssl);
	SSL_free(ssl);
#endif

	return 0;
}

int
srv_listen()
{
	int client_socket;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;

	while (server_running)
	{
		client_addr_size = sizeof(client_addr);
		client_socket =
				accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_size);
		if (client_socket >= 0)
		{
			printf("client accepted (%d)\n", client_socket);
#if FORK_CLIENTS == TRUE
			if (fork() == 0)
			{
				srv_process_socket(client_socket, &client_addr);
				close(client_socket);
				close(server_socket);
				exit(0);
			}
#else
			srv_process_socket(client_socket, &client_addr);
#endif
			close(client_socket);
		}
		else
		{
			/*if (errno != EAGAIN)
				printf("accept error %d\n", errno);*/
			if (errno == EBADF)
			{
				printf("Bad socket FD\n");
				srv_cleanup();
				exit(1);
			}
		}
	}
	return 0;
}

int
srv_cleanup()
{
	printf("cleaning up\n");
	shutdown(server_socket, SHUT_RDWR);
	close(server_socket);
#if SERVER_USE_SSL
	SSL_CTX_free(ssl_ctx);
	EVP_cleanup();
#endif
	return 0;
}

void
setup_document_root(void)
{
	const char *cfg_root;
	char *tmp;
	char cwd[PATH_MAX + 1];
	struct path_s *path;

	cfg_root = kv_string(global_config, "root", "www");
	if (*cfg_root == '/')
	{
		documentroot = malloc(strlen(cfg_root) + 1);
		strcpy(documentroot, cfg_root);
	}
	else
	{
		getcwd(cwd, sizeof(cwd));
		path = path_make(cwd);
		path_cat(path, cfg_root);
		tmp = path_to_string(path, "");
		path_free(path);
		documentroot = malloc(strlen(tmp) + 1);
		strcpy(documentroot, tmp);
		free(tmp);
	}

	printf("Serving: %s\n", documentroot);
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

	if (srv_setup(&server_socket) != 0)
	{
		printf("srv_setup failed\n");
		return 1;
	}
	if (srv_listen() != 0)
	{
		printf("srv_listen failed\n");
	}
	if (srv_cleanup() != 0)
	{
		printf("srv_cleanup failed\n");
	}
	return 0;
}