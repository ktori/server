#include "vhttpsl/bits/kv.h"
#include "vhttpsl/bits/str.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct kv_list_s *
kv_create(void)
{
	return calloc(1, sizeof(struct kv_list_s));
}

void
kv_free(struct kv_list_s *list)
{
	kv_free_list(list);
	free(list);
}

void
kv_free_list(struct kv_list_s *list)
{
	struct kv_node_s *current, *next;

	if (list == NULL)
		return;

	current = list->head;
	while (current != NULL)
	{
		next = current->next;
		kv_free_node(current);
		current = next;
	}
}

void
kv_free_node(struct kv_node_s *node)
{
	free(node->key);
	free(node->value);
	free(node);
}

void
kv_push(struct kv_list_s *list, const char *key, const char *value)
{
	if (key == NULL)
		key = "";
	if (value == NULL)
		value = "";

	kv_push_n(list, key, strlen(key), value, strlen(value));
}

void
kv_push_n(struct kv_list_s *list, const char *key, size_t key_length, const char *value, size_t value_length)
{
	struct kv_node_s *node;

	list->count++;
	node = calloc(1, sizeof(struct kv_node_s));

	node->key = calloc(key_length + 1, sizeof(char));
	node->value = calloc(value_length + 1, sizeof(char));
	strncpy(node->key, key, key_length);
	strncpy(node->value, value, value_length);

	node->prev = list->tail;

	if (list->head == NULL)
	{
		list->head = node;
	}

	if (list->tail != NULL)
	{
		list->tail->next = node;
	}
	list->tail = node;
}

void
kv_push_from_line(struct kv_list_s *list,
				  const char *line,
				  size_t line_length,
				  char delim,
				  int trim_whitespace)
{
	struct kv_node_s *node;

	node = kv_from_line(line, line_length, delim, trim_whitespace);

	list->count++;
	node->prev = list->tail;

	if (list->head == NULL)
	{
		list->head = node;
	}

	if (list->tail != NULL)
	{
		list->tail->next = node;
	}
	list->tail = node;
}

void
kv_pop(struct kv_list_s *list)
{
	struct kv_node_s *node;

	node = list->tail;

	if (node != NULL)
	{
		list->count--;
		if (node->prev != NULL)
		{
			node->prev->next = NULL;
		}
		list->tail = node->prev;
		kv_free_node(node);
	}
	if (list->head == node)
	{
		list->head = NULL;
	}
}

struct kv_node_s *
kv_from_line(const char *line, size_t line_length, char delim, int trim_whitespace)
{
	struct kv_node_s *node;
	int i, begin, end;
	char *key, *value, *tmp;
	int key_read;

	key_read = 0;
	node = calloc(1, sizeof(struct kv_node_s));
	key = NULL;
	value = NULL;
	begin = trim_whitespace ? -1 : 0;
	end = -1;
	for (i = 0; i <= strlen(line); ++i)
	{
		if (line[i] == '\0' || (key_read == 0 && line[i] == delim))
		{
			if (begin >= 0)
			{
				tmp = substr(line, begin, (end >= 0 ? end : i) - begin + 1);
				if (key_read)
				{
					value = tmp;
				}
				else
				{
					key = tmp;
					key_read = 1;
				}
			}
			begin = (trim_whitespace ? -1 : (i + 1));
			end = -1;
		}
		else if (trim_whitespace)
		{
			if (isspace(line[i]) == 0)
			{
				if (begin < 0)
				{
					begin = i;
				}
				end = i;
			}
		}
	}

	if (key == NULL)
	{
		key = malloc(1);
		*key = '\0';
	}
	if (value == NULL)
	{
		value = malloc(1);
		*value = '\0';
	}

	node->key = key;
	node->value = value;

	return node;
}

struct kv_node_s *
kv_find(struct kv_list_s *list, const char *key)
{
	struct kv_node_s *current;

	if (list == NULL)
		return NULL;

	current = list->head;
	while (current != NULL)
	{
		if (STRIEQ(current->key, key))
		{
			return current;
		}
		current = current->next;
	}

	return NULL;
}

void
kv_set(struct kv_list_s *list, const char *key, const char *value)
{
	struct kv_node_s *node;

	node = kv_find(list, key);
	if (node == NULL)
	{
		kv_push(list, key, value);
	}
	else
	{
		free(node->value);
		node->value = malloc(strlen(value) + 1);
		strcpy(node->value, value);
	}
}

int
kv_isset(struct kv_list_s *list, const char *key)
{
	return kv_find(list, key) != NULL;
}

const char *
kv_string(struct kv_list_s *list, const char *key, const char *fallback)
{
	struct kv_node_s *node;

	node = kv_find(list, key);
	if (node != NULL)
	{
		return node->value;
	}
	return fallback;
}

int
kv_int(struct kv_list_s *list, const char *key, int fallback)
{
	struct kv_node_s *node;
	int value;

	value = fallback;
	node = kv_find(list, key);
	if (node != NULL)
	{
		sscanf(node->value, "%d", &value);
	}
	return value;
}

char **
kv_array(struct kv_list_s *list, const char *key, char delim)
{
	struct kv_node_s *node;

	node = kv_find(list, key);
	if (node != NULL)
	{
		return array_from_string(node->value, delim);
	}
	return empty_string_array();
}