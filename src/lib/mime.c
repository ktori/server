#include "vhttpsl/bits/str.h"
#include "config.h"
#include "vhttpsl/bits/kv.h"

#include <string.h>
#include <stdio.h>
#include <yaml/src/document/document.h>
#include <yaml/src/yaml.h>
#include <yaml/src/document/value.h>
#include <yaml/src/document/kv.h>

struct mime_type_s
{
	const char *ext;
	const char *mime;
};

static struct mime_type_s *mime_types = NULL;
static size_t mime_types_count = 0;
static size_t mime_types_size = 0;

bool mime_init_done = FALSE;

static int
mime_from_yaml(struct yaml_document_s *document)
{
	struct yaml_map_s *map;
	struct yaml_kv_s *kv;
	size_t i;

	if (document->root.type != YVT_MAP)
		return EXIT_FAILURE;

	map = &document->root.body.map;
	mime_types_size = map->array_size;
	mime_types_count = map->array_size;
	mime_types = calloc(map->array_size, sizeof(struct mime_type_s));

	for (i = 0, kv = map->kv_array; kv < map->kv_array + map->array_size; ++kv, ++i)
	{
		if (kv->key.type != YVT_STRING || kv->value.type != YVT_STRING)
			return EXIT_FAILURE;

		mime_types[i].ext = kv->key.body.string;
		mime_types[i].mime = kv->value.body.string;

		kv->key.type = YVT_NULL;
		kv->value.type = YVT_NULL;
	}

	return EXIT_SUCCESS;
}

int
mimeinit(void)
{
	FILE *cfg;
	char line[2048];
	char config_path[512];
	struct yaml_s *yaml;
	struct yaml_document_s document;
	int status = EXIT_SUCCESS;

	snprintf(config_path, 512, "%s/%s", config_loc(), "mime.yaml");
	cfg = fopen(config_path, "r");

	if (cfg == NULL)
	{
		fprintf(stderr, "could not read config file: %s\n", config_path);

		return EXIT_FAILURE;
	}

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
		if (mime_from_yaml(&document) != EXIT_SUCCESS)
		{
			status = EXIT_FAILURE;
		}
	}

	yaml_free(&yaml);
	yaml_document_destroy(&document);
	fclose(cfg);

	mime_init_done = TRUE;
	return status;
}

const char *
mimetype(const char *filename)
{
	char *extension;
	struct mime_type_s *type;

	extension = strrchr(filename, '.') + 1;
	if (extension == (char *) 1)
		return "text/plain";

	if (mime_init_done == FALSE)
	{
		mimeinit();
	}

	for (type = mime_types; type < mime_types + mime_types_count; ++type)
		if (strcmp(type->ext, extension) == 0)
			return type->mime;

	return "text/plain";
}
