/*
 * Created by victoria on 15.02.20.
*/

#include "error.h"
#include "../lib/http.h"
#include "../lib/mime.h"
#include "../http/response.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int
serve_file(struct http_response_s *response, const char *filename, char noerr)
{
	FILE *file;
	int length;
	printf("trying to serve %s\n", filename);
	if (access(filename, R_OK) != EXIT_SUCCESS)
	{
		if (noerr == 0)
		{
			if (errno == ENOENT)
			{
				serve_error(response, 404, "Not Found");
			}
			else if (errno == EACCES)
			{
				serve_error(response, 403, "Forbidden");
			}
		}
		return EXIT_FAILURE;
	}

	file = fopen(filename, "r");
	if (file == NULL)
	{
		if (noerr == 0)
		{
			serve_error(response, 500, "Internal server error");
		}
		return EXIT_FAILURE;
	}
	fseek(file, 0L, SEEK_END);
	length = ftell(file);
	fseek(file, 0L, SEEK_SET);
	response->body = malloc(length);
	fread(response->body, 1, length, file);
	fclose(file);
	response->length = length;
	kv_push(response->headers, "Content-Type", mimetype(filename));
	printf("Served file %s with length %d\n", filename, length);
	return EXIT_SUCCESS;
}