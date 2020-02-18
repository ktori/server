/*
 * Created by victoria on 15.02.20.
*/

#include "cluster.h"
#include "../server/server.h"

#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>

int
cluster_init(struct cluster_s *cluster)
{
	cluster->count = 0;
	cluster->capacity = 1;
	cluster->servers = calloc(cluster->capacity, sizeof(struct server_s));

	return EXIT_SUCCESS;
}

int
cluster_add(struct cluster_s *cluster, struct server_s *server)
{
	struct server_s *alloc;

	if (cluster_alloc(cluster, &alloc) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	memcpy(alloc, server, sizeof(struct server_s));

	return EXIT_SUCCESS;
}

int
cluster_alloc(struct cluster_s *cluster, struct server_s **server_out)
{
	if (cluster->count == cluster->capacity)
	{
		cluster->capacity *= 2;
		cluster->servers = realloc(cluster->servers, sizeof(struct server_s) * cluster->capacity);
		memset(cluster->servers + cluster->count, 0, (cluster->capacity - cluster->count) * sizeof(struct server_s));
	}

	*server_out = cluster->servers + cluster->count;
	memset(cluster->servers + cluster->count, 0, sizeof(struct server_s));
	cluster->count += 1;

	return EXIT_SUCCESS;
}

int
cluster_run(struct cluster_s *cluster)
{
	struct server_s *i = cluster->servers, *end = cluster->servers + cluster->count;

	for (; i < end; ++i)
	{
		if (server_start(i) != EXIT_SUCCESS)
			return EXIT_FAILURE; /* TODO: Stop started servers */
	}

	return EXIT_SUCCESS;
}

int
cluster_listen(struct cluster_s *cluster)
{
	int timeout = 60 * 1000;
	struct pollfd *fds = calloc(cluster->count, sizeof(struct pollfd));
	size_t i;

	for (i = 0; i < cluster->count; ++i)
	{
		fds[i].fd = cluster->servers[i].sock_fd;
		fds[i].events = POLLIN;
	}

	do
	{
		int result = poll(fds, cluster->count, timeout);
		if (result < 0)
		{
			perror("poll()");
			free(fds);
			return EXIT_FAILURE;
		}
		if (result == 0)
			continue;
		for (i = 0; i < cluster->count; ++i)
		{
			if (fds[i].revents == 0)
				continue;
			if (fds[i].revents != POLLIN)
			{
				fprintf(stderr, "unexpected revents: %d\n", fds[i].revents);
				free(fds);
				return EXIT_FAILURE;
			}
			do
			{
				struct client_s client;
				if (client_accept(&cluster->servers[i], &client) != EXIT_SUCCESS)
				{
					if (errno != EAGAIN)
						perror("client_accept()");
					break;
				}
				printf("client connected: %d\n", client.socket);
#if USE_FORKS
				if (fork() == 0)
				{
					if (client_setup(&cluster->servers[i], &client) != EXIT_SUCCESS)
					{
						perror("client_setup()");
						exit(EXIT_FAILURE);
					}
					if (server_accept(&cluster->servers[i], &client) != EXIT_SUCCESS)
					{
						perror("server_accept()");
						exit(EXIT_FAILURE);
					}
					client_close(&client);
					close(cluster->servers[i].sock_fd);
					exit(EXIT_SUCCESS);
				}
				else
				{
					client_close(&client);
				}
#else
				if (client_setup(&cluster->servers[i], &client) != EXIT_SUCCESS)
				{
					perror("client_setup()");
					client_close(&client);
					break;
				}
				if (server_accept(&cluster->servers[i], &client) != EXIT_SUCCESS)
				{
					perror("server_accept()");
					client_close(&client);
					break;
				}
				printf("done serving client: %d\n", client.socket);
				client_close(&client);
#endif
			}
			while (1);
		}
	}
	while (1);

	return EXIT_SUCCESS;
}

int
cluster_destroy(struct cluster_s *cluster)
{
	size_t i;
	int status = EXIT_SUCCESS;

	for (i = 0; i < cluster->count; ++i)
	{
		if (server_cleanup(&cluster->servers[i]) != EXIT_SUCCESS)
		{
			perror("server_cleanup()");
			status = EXIT_FAILURE;
		}
	}

	free(cluster->servers);

	return status;
}
