#pragma once

#include <stdlib.h>

typedef struct kv_node_s
{
	char *key;
	char *value;

	struct kv_node_s *prev;
	struct kv_node_s *next;
} *kv_node_t;

typedef struct kv_list_s
{
	int count;

	struct kv_node_s *head;
	struct kv_node_s *tail;
} *kv_list_t;

struct kv_list_s *
kv_create(void);

void
kv_free(struct kv_list_s *list);

void
kv_free_list(struct kv_list_s *list);

void
kv_free_node(struct kv_node_s *node);

void
kv_push(struct kv_list_s *list, const char *key, const char *value);

void
kv_push_n(struct kv_list_s *list, const char *key, size_t key_length, const char *value, size_t value_length);

void
kv_push_from_line(struct kv_list_s *list,
				  const char *line,
				  size_t line_length,
				  char delim,
				  int trim_whitespace);

void
kv_pop(struct kv_list_s *list);

struct kv_node_s *
kv_from_line(const char *line, size_t line_length, char delim, int trim_whitespace);

struct kv_node_s *
kv_find(struct kv_list_s *list, const char *key);

int
kv_isset(struct kv_list_s *list, const char *key);

void
kv_set(struct kv_list_s *list, const char *key, const char *value);

const char *
kv_string(struct kv_list_s *list, const char *key, const char *fallback);

int
kv_int(struct kv_list_s *list, const char *key, int fallback);

char **
kv_array(struct kv_list_s *list, const char *key, char delim);