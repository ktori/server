#include "config.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct kv_list_s *global_config = NULL;

static const char *config_loc_cached = NULL;

const char *
config_loc()
{
	if (config_loc_cached != NULL)
	{
		return config_loc_cached;
	}
	const char *config_env = getenv("CONFIG");
	if (config_env != NULL)
	{
		return (config_loc_cached = config_env);
	}
	return (config_loc_cached = "conf");
}

struct kv_list_s *
config_load(const char *filename)
{
	struct kv_list_s *result;
	FILE *cfg;
	char line[2048];

	result = kv_create();
	char config_path[512];
	snprintf(config_path, 512, "%s/%s", config_loc(), filename);
	cfg = fopen(config_path, "r");

	if (cfg == NULL)
	{
		fprintf(stderr, "could not read config file: %s\n", config_path);

		return NULL;
	}
	while (fgets(line, sizeof(line), cfg) != NULL)
	{
		if (*line != '#')
		{
			if (line[strlen(line) - 1] == '\n')
			{
				line[strlen(line) - 1] = '\0'; /* remove \n */
			}
			kv_push_from_line(result, line, '=', TRUE);
		}
	}

	return result;
}