/*
 * Created by victoria on 15.02.20.
*/

#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int
server_listen(struct server_s *server)
{
	struct client_s client;

	while (server->is_running)
	{
		if (client_accept(server, &client) != EXIT_SUCCESS)
		{
			/*if (errno != EAGAIN)
	printf("accept error %d\n", errno);*/
			if (errno == EBADF)
			{
				printf("Bad socket FD\n");
				server->is_running = 0;
				return EXIT_FAILURE;
			}

			continue;
		}

		printf("client accepted (%d)\n", client.socket);
#if USE_FORKS
		if (fork() == 0)
		{
			client_setup(server, &client);
			server_accept(server, &client);
			client_close(&client);
			close(server->sock_fd);
			exit(0);
		}
		else
		{
			client_close(&client);
		}
#else
		if (client_setup(server, &client) != EXIT_SUCCESS)
		{
			fprintf(stderr, "client_setup()");
			client_close(&client);
			continue;
		}
		if (server_accept(server, &client) != EXIT_SUCCESS)
		{
			fprintf(stderr, "server_accept()");
		}
		client_close(&client);
#endif
	}

	return EXIT_SUCCESS;
}