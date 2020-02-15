#pragma once

struct path_node_s
{
	char *name;
	char *extension;

	struct path_node_s *prev;
	struct path_node_s *next;
};

struct path_s
{
	int length;

	struct path_node_s *head;
	struct path_node_s *tail;
};

struct path_node_s *
path_push(struct path_s *path, const char *name);

void
path_cat(struct path_s *path, const char *subpath);

void
path_pop(struct path_s *path);

void
path_free(struct path_s *path);

struct path_s *
path_make(const char *path);

char *
path_to_string(struct path_s *path, const char *root);