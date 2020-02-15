/*
 * Created by victoria on 15.02.20.
*/
#pragma once

#include <stdlib.h>

struct cluster_s
{
	size_t count;
	size_t capacity;
	struct server_s *servers;
};

int
cluster_init(struct cluster_s *cluster);

int
cluster_add(struct cluster_s *cluster, struct server_s *server);

int
cluster_run(struct cluster_s *cluster);

int
cluster_destroy(struct cluster_s *cluster);
