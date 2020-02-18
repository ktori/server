#include "config.h"
#include "str.h"
#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <yaml/src/yaml.h>
#include <yaml/src/document/document.h>
#include <yaml/src/document/value.h>
#include <yaml/src/document/kv.h>

static const char *config_loc_cached = NULL;

const char *
config_loc()
{
	const char *config_env;

	if (config_loc_cached != NULL)
	{
		return config_loc_cached;
	}
	config_env = getenv("CONFIG");
	if (config_env != NULL)
	{
		return (config_loc_cached = config_env);
	}
	return (config_loc_cached = "conf");
}

/* takes ownership of string, changes it to NULL in the map */
const char *
yaml_take_string(struct yaml_value_s *value)
{
	if (value->type != YVT_STRING)
		return NULL;
	value->type = YVT_NULL;
	return value->body.string;
}

struct yaml_kv_s *
yaml_find_kv(struct yaml_map_s *map, const char *string)
{
	size_t i;

	for (i = 0; i < map->array_size; ++i)
	{
		if (map->kv_array[i].key.type == YVT_STRING && strcmp(string, map->kv_array[i].key.body.string) == 0)
			return &map->kv_array[i];
	}

	return NULL;
}

/* takes ownership of string, changes it to NULL in the map */
const char *
yaml_extract_string(struct yaml_map_s *map, const char *key)
{
	struct yaml_kv_s *kv;

	kv = yaml_find_kv(map, key);
	if (kv == NULL)
		return NULL;

	return yaml_take_string(&kv->value);
}

static int
setup_document_root(struct server_config_s *config, const char *cfg_root)
{
	char *tmp;
	char cwd[PATH_MAX + 1];
	struct path_s *path;

	if (*cfg_root == '/')
	{
		config->root = malloc(strlen(cfg_root) + 1);
		strcpy((char *) config->root, cfg_root);
	}
	else
	{
		getcwd(cwd, sizeof(cwd));
		path = path_make(cwd);
		path_cat(path, cfg_root);
		tmp = path_to_string(path, "");
		path_free(path);
		config->root = malloc(strlen(tmp) + 1);
		strcpy((char *) config->root, tmp);
		free(tmp);
	}

	printf("Serving: %s\n", config->root);

	return EXIT_SUCCESS;
}

static int
config_from_yaml(struct server_config_s *config, struct yaml_document_s *document)
{
	struct yaml_map_s *root, *errors, *cgi, *types, *ssl;
	struct yaml_kv_s *kv;
	const char *cfg_root;
	size_t i;

	if (document->root.type != YVT_MAP)
		return EXIT_FAILURE;

	root = &document->root.body.map;

	kv = yaml_find_kv(root, "port");
	if (kv == NULL || kv->value.type != YVT_INT)
		return EXIT_FAILURE;
	config->port = kv->value.body.integer;

	config->index = yaml_extract_string(root, "index");
	cfg_root = yaml_extract_string(root, "root");
	if (cfg_root == NULL)
		return EXIT_FAILURE;
	setup_document_root(config, cfg_root);

	kv = yaml_find_kv(root, "errors");
	if (kv != NULL)
	{
		if (kv->value.type != YVT_MAP)
			return EXIT_FAILURE;
		errors = &kv->value.body.map;
		config->errors_size = errors->array_size;
		config->errors_count = config->errors_size;
		config->errors = calloc(config->errors_size, sizeof(struct error_config_entry_s));
		for (i = 0; i < errors->array_size; ++i)
		{
			kv = &errors->kv_array[i];
			if (kv->key.type != YVT_INT)
				return EXIT_FAILURE;
			if (kv->value.type != YVT_STRING)
				return EXIT_FAILURE;
			config->errors[i].code = kv->key.body.integer;
			config->errors[i].document = yaml_take_string(&kv->value);
		}
	}
	kv = yaml_find_kv(root, "cgi");
	if (kv != NULL)
	{
		if (kv->value.type != YVT_MAP)
			return EXIT_FAILURE;
		cgi = &kv->value.body.map;

		config->cgi_bin = yaml_extract_string(cgi, "root");
		if (config->cgi_bin == NULL)
			return EXIT_FAILURE;

		kv = yaml_find_kv(cgi, "types");
		if (kv == NULL || kv->value.type != YVT_MAP)
			return EXIT_FAILURE;

		types = &kv->value.body.map;

		config->cgi_size = types->array_size;
		config->cgi_count = config->cgi_size;
		config->cgi = calloc(config->cgi_size, sizeof(struct cgi_config_entry_s));
		for (i = 0; i < types->array_size; ++i)
		{
			kv = &types->kv_array[i];
			if (kv->key.type != YVT_STRING)
				return EXIT_FAILURE;
			if (kv->value.type != YVT_MAP)
				return EXIT_FAILURE;
			config->cgi[i].extension = yaml_take_string(&kv->key);
			cgi = &kv->value.body.map;
			config->cgi[i].command = yaml_extract_string(cgi, "command");
			if (config->cgi[i].command == NULL)
				return EXIT_FAILURE;
			kv = yaml_find_kv(cgi, "exec");
			config->cgi[i].exec = (kv != NULL && kv->value.type == YVT_INT) ? kv->value.body.integer : 0;
			kv = yaml_find_kv(cgi, "loose");
			config->cgi[i].loose = (kv != NULL && kv->value.type == YVT_INT) ? kv->value.body.integer : 0;
		}
	}

	kv = yaml_find_kv(root, "ssl");
	if (kv != NULL)
	{
		if (kv->value.type != YVT_MAP)
			return EXIT_FAILURE;
		ssl = &kv->value.body.map;
		config->ssl = 1;
		kv = yaml_find_kv(ssl, "cert");
		if (kv == NULL || kv->value.type != YVT_STRING)
			return EXIT_FAILURE;
		config->ssl_cert = yaml_take_string(&kv->value);
		kv = yaml_find_kv(ssl, "key");
		if (kv == NULL || kv->value.type != YVT_STRING)
			return EXIT_FAILURE;
		config->ssl_key = yaml_take_string(&kv->value);
	}

	return EXIT_SUCCESS;
}

int
config_load(struct server_config_s *config, const char *filename)
{
	FILE *cfg;
	char line[2048];
	char config_path[512];
	struct yaml_s *yaml;
	struct yaml_document_s document;
	int status = EXIT_SUCCESS;

	snprintf(config_path, 512, "%s/%s", config_loc(), filename);
	cfg = fopen(config_path, "r");

	if (cfg == NULL)
	{
		fprintf(stderr, "could not read config file: %s\n", config_path);

		return EXIT_FAILURE;
	}

	memset(config, 0, sizeof(struct server_config_s));

	yaml_create(&yaml);
	yaml_document_init(&document);
	yaml_document_bind(&document, yaml);

	while (fgets(line, sizeof(line), cfg) != NULL)
	{
		if (yaml_in(yaml, line, strlen(line), document.ctx) != EXIT_SUCCESS)
		{
			fprintf(stderr, "YAML loading error\n");
			status = EXIT_FAILURE;
		}
	}

	if (status == EXIT_SUCCESS)
	{
		if (config_from_yaml(config, &document) != EXIT_SUCCESS)
		{
			config_destroy(config);
			status = EXIT_FAILURE;
		}
	}

	yaml_free(&yaml);
	yaml_document_destroy(&document);
	fclose(cfg);

	return status;
}

int
config_destroy(struct server_config_s *config)
{
	size_t i;

	free((void *) config->index);
	free((void *) config->root);
	free((void *) config->cgi_bin);

	for (i = 0; i < config->errors_count; ++i)
		free((void *) config->errors[i].document);
	free(config->errors);

	for (i = 0; i < config->cgi_count; ++i)
	{
		free((void *) config->cgi[i].extension);
		free((void *) config->cgi[i].command);
	}
	free(config->cgi);

	memset(config, 0, sizeof(struct server_config_s));

	return 0;
}
