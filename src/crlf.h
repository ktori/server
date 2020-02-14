#ifndef CRLF_H
#define CRLF_H

#include "common.h"

struct crlf_marker_s
{
	char *loc;
	int size;

	struct crlf_marker_s *next;
};

char *
crlf_get_string(struct crlf_marker_s *marker);

struct crlf_array_s
{
	char *buffer;

	struct crlf_marker_s *first;
};

int
crlf_array_from_buffer(char *buffer, size_t size, struct crlf_array_s **array);

void
crlf_array_free(struct crlf_array_s *array);

#endif /* CRLF_H */