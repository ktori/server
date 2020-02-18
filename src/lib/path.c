#include "path.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct path_node_s *
path_push(struct path_s *path, const char *name)
{
	char *ext;
	struct path_node_s *new = malloc(sizeof(struct path_node_s));
	int length = strlen(name);

	memset(new, 0, sizeof(struct path_node_s));

	new->name = malloc(length + 1);
	strcpy(new->name, name);
	ext = strrchr(name, '.');
	if (ext == NULL)
	{
		new->extension = malloc(1);
		*new->extension = '\0';
	}
	else
	{
		new->extension = malloc(strlen(ext));
		strcpy(new->extension, ext + 1);
	}

	path->length += (length + 1);

	new->next = NULL;
	new->prev = path->tail;

	if (path->head == NULL)
	{
		path->head = new;
	}
	if (path->tail != NULL)
	{
		path->tail->next = new;
	}
	path->tail = new;

	return new;
}

void
path_pop(struct path_s *path)
{
	struct path_node_s *node = path->tail;
	if (node != NULL)
	{
		path->length -= (1 + strlen(node->name));
		if (node->prev != NULL)
		{
			node->prev->next = NULL;
		}
		free(node->name);
		free(node);
		if (path->head == node)
		{
			path->head = NULL;
		}
		path->tail = node->prev;
	}
}

void
path_free(struct path_s *path)
{
	struct path_node_s *node, *next;

	node = path->head;
	while (node != NULL)
	{
		next = node->next;
		free(node->name);
		free(node->extension);
		free(node);
		node = next;
	}
}

void
path_cat(struct path_s *path, const char *subpath)
{
	size_t i;
	size_t begin = 0;
	int path_length = strlen(subpath);
	char *temp_name;

	for (i = 0; i <= path_length; ++i)
	{
		if (subpath[i] == '/' || subpath[i] == '\0')
		{
			if ((i - begin > 0))
			{
				temp_name = malloc(i - begin + 1);

				strncpy(temp_name, subpath + begin, i - begin);
				temp_name[i - begin] = '\0';

				if (STREQ(".", temp_name))
				{ ;
				}
				else if (STREQ("..", temp_name))
				{
					path_pop(path);
				}
				else
				{
					path_push(path, temp_name);
				}
				free(temp_name);
			}
			begin = i + 1;
		}
	}
}

struct path_s *
path_make(const char *path)
{
	struct path_s *result = malloc(sizeof(struct path_s));
	size_t i;
	size_t begin = 0;
	int path_length = strlen(path);
	char *temp_name;

	memset(result, 0, sizeof(struct path_s));

	for (i = 0; i <= path_length; ++i)
	{
		if (path[i] == '/' || path[i] == '\0')
		{
			if ((i - begin > 0))
			{
				temp_name = malloc(i - begin + 1);

				strncpy(temp_name, path + begin, i - begin);
				temp_name[i - begin] = '\0';

				if (STREQ(".", temp_name))
				{ ;
				}
				else if (STREQ("..", temp_name))
				{
					path_pop(result);
				}
				else
				{
					path_push(result, temp_name);
				}
				free(temp_name);
			}
			begin = i + 1;
		}
	}

	return result;
}

char *
path_to_string(struct path_s *path, const char *root)
{
	char *result = NULL, *ptr = NULL;
	struct path_node_s *current = NULL;

	result = malloc(path->length + strlen(root) + 2);
	sprintf(result, "%s/", root);
	ptr = result + strlen(root) + 1;

	current = path->head;
	while (current != NULL)
	{
		strcpy(ptr, current->name);
		ptr += strlen(current->name);
		if (current->next != NULL)
		{
			ptr[0] = '/';
			ptr++;
		}
		else
		{
			ptr[0] = '\0';
		}
		current = current->next;
	}

	return result;
}
