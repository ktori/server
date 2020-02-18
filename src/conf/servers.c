/*
 * Created by victoria on 19.02.20.
*/

#include "servers.h"
#include "config.h"
#include "../server/server.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

#define CONF_SERVERS "servers"

int
load_server(struct cluster_s *cluster, const char *config_name)
{
	struct server_s *server = NULL;

	if (cluster_alloc(cluster, &server) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	if (config_load(&server->config, config_name) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int
load_servers(struct cluster_s *cluster)
{
	const char *config_path = get_config_path();
	size_t config_path_len = strlen(config_path), buffer_size = config_path_len + 1 + sizeof(CONF_SERVERS);
	char *buffer = malloc(buffer_size);
	DIR *dir = NULL;
	struct dirent *dirent;
	struct stat stat_res;

	snprintf(buffer, buffer_size, "%s/%s", config_path, CONF_SERVERS);
	dir = opendir(buffer);

	if (dir == NULL)
	{
		fprintf(stderr, "could not open servers config: %s\n", buffer);
		free(buffer);
		return EXIT_FAILURE;
	}

	free(buffer);

	while ((dirent = readdir(dir)))
	{
		buffer_size = config_path_len + 1 + sizeof(CONF_SERVERS) + 1 + strlen(dirent->d_name) + 1;
		buffer = malloc(buffer_size);
		snprintf(buffer, buffer_size, "%s/%s/%s", config_path, CONF_SERVERS, dirent->d_name);
		if (stat(buffer, &stat_res) != EXIT_SUCCESS)
		{
			free(buffer);
			closedir(dir);
			perror("stat()");
			return EXIT_FAILURE;
		}

		if (!S_ISREG(stat_res.st_mode))
		{
			free(buffer);
			continue;
		}

		if (load_server(cluster, buffer) != EXIT_SUCCESS)
		{
			fprintf(stderr, "error loading server: %s\n", buffer);
			free(buffer);
			closedir(dir);
			return EXIT_FAILURE;
		}

		free(buffer);
	}

	closedir(dir);

	return EXIT_SUCCESS;
}
