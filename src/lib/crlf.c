#include "crlf.h"

#include <stdlib.h>
#include <string.h>

int
crlf_array_from_buffer(char *buffer, size_t size, struct crlf_array_s **array)
{
	char *begin = buffer;
	size_t i = 0, count = 0, length = 0;

	struct crlf_marker_s *marker = NULL, *prev = NULL;

	struct crlf_array_s *result = malloc(sizeof(struct crlf_array_s));

	result->buffer = buffer;
	result->first = NULL;

	for (i = 0; i < size; ++i)
	{
		if (i > 0 && buffer[i - 1] == '\r' && buffer[i] == '\n')
		{
			marker = malloc(sizeof(struct crlf_marker_s));
			marker->next = NULL;
			if (prev != NULL)
			{
				prev->next = marker;
			}
			if (result->first == NULL)
			{
				result->first = marker;
			}
			marker->loc = begin;
			marker->size = length - 1;
			prev = marker;
			begin = buffer + i + 1;
			length = 0;
			++count;
		}
		else
		{
			++length;
		}
	}

	if (length > 0)
	{
		marker = malloc(sizeof(struct crlf_marker_s));
		marker->next = NULL;
		if (prev != NULL)
		{
			prev->next = marker;
		}
		if (result->first == NULL)
		{
			result->first = marker;
		}
		marker->loc = begin;
		marker->size = length - 1;
	}

	*array = result;
	return count;
}

char *
crlf_get_string(struct crlf_marker_s *marker)
{
	char *result = malloc(marker->size + 1);

	memcpy(result, marker->loc, marker->size);
	result[marker->size] = '\0';
	return result;
}

void
crlf_array_free(struct crlf_array_s *array)
{
	struct crlf_marker_s *marker, *next;

	marker = array->first;
	while (marker != NULL)
	{
		next = marker->next;
		free(marker);
		marker = next;
	}
}