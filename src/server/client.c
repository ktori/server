/*
 * Created by victoria on 15.02.20.
*/

#include "server.h"
#include "client.h"
#include "../def.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#if SERVER_USE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

int
client_accept(struct server_s *server, struct client_s *client)
{
	client->ssl = NULL;
	client->addr_size = sizeof(client->addr);
	client->socket =
			accept(server->sock_fd, (struct sockaddr *) &client->addr, &client->addr_size);

	if (client->socket < 0)
	{
		if (errno != EAGAIN)
			perror("accept()");
		return EXIT_FAILURE;
	}

	client->eof = FALSE;

	return EXIT_SUCCESS;
}

int
client_setup(struct server_s *server, struct client_s *client)
{
#if SERVER_USE_SSL
	if (server->ssl_ctx)
	{
		SSL *ssl = SSL_new(server->ssl_ctx);
		SSL_set_fd(ssl, client->socket);
		if (SSL_accept(ssl) <= 0)
		{
			client->ssl = NULL;
			ERR_print_errors_fp(stderr);
			SSL_shutdown(ssl);
			SSL_free(ssl);
			return EXIT_FAILURE;
		}
		client->ssl = ssl;
	}
	else
		client->ssl = NULL;
#endif

	client->server = server;

	return EXIT_SUCCESS;
}

int
client_read_some(struct client_s *client, char *out, size_t buffer_size, size_t *out_length)
{
	int received;

	if (client->eof)
		return EXIT_FAILURE;

#if SERVER_USE_SSL
	if (client->ssl)
		received = SSL_read(client->ssl, out, buffer_size);
	else
		received = read(client->socket, out, buffer_size);
#else
	received = read(client->socket, out, buffer_size);
#endif

	if (received < 0)
	{
		perror("read error");
		return EXIT_FAILURE;
	}
	if (received == 0)
	{
		client->eof = TRUE;
	}

	*out_length = received;
	return EXIT_SUCCESS;
}

/*
 *  TODO Content-Length
 */
int
client_read(struct client_s *client, char **out, size_t *out_length)
{
	size_t received;
	char *buffer;
	size_t total = 0;
	size_t size = 128;
	const char *headers_end;

	buffer = malloc(size);

	do
	{
		if (client_read_some(client, buffer + total, size - total - 1, &received) != EXIT_SUCCESS)
		{
			free(buffer);
			return EXIT_FAILURE;
		}
		if (received == 0)
		{
			*out = buffer;
			*out_length = total;
			return EXIT_SUCCESS;
		}
		total += received;
		if (total >= size - 1)
		{
			size *= 2;
			buffer = realloc(buffer, size);
		}
		headers_end = strstr(buffer + total - received, "\r\n\r\n");
		if (headers_end != NULL)
		{
			/* TODO POST request (body) */
			*out = buffer;
			*out_length = headers_end - buffer;
			return EXIT_SUCCESS;
		}
	}
	while (1);
}

int
client_write(struct client_s *client, const char *data, size_t length)
{
	int sent;

	if (client->eof)
		return EXIT_FAILURE;

	while (length > 0)
	{

#if SERVER_USE_SSL
		if (client->ssl)
			sent = SSL_write(client->ssl, data, length);
		else
			sent = write(client->socket, data, length);
#else
		sent = write(client->socket, data, length);
#endif

		if (sent < 1 || sent > length)
			return EXIT_FAILURE;
		data += sent;
		length -= sent;
	}

	return EXIT_SUCCESS;
}

int
client_close(struct client_s *client)
{
#if SERVER_USE_SSL
	if (client->ssl)
	{
		SSL_shutdown(client->ssl);
		SSL_free(client->ssl);
	}
#endif

	close(client->socket);

	return EXIT_SUCCESS;
}
