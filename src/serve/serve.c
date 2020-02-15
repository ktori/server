/*
 * Created by victoria on 15.02.20.
*/

#include "serve.h"
#include "../lib/kv.h"
#include "file.h"
#include "index.h"
#include "../lib/path.h"
#include "../lib/config.h"
#include "../lib/http.h"
#include "error.h"
#include "../server.h"
#include "cgi.h"
#include "../lib/url.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>

int
serve(struct http_request_s *request, struct http_response_s *response)
{
	char clength[16];
	char *uri_path;
	struct stat buf;

	memset(&buf, 0, sizeof(buf));

	response->version_major = 1;
	response->version_minor = 1;
	response->code = 200;

	if (request == NULL || !STREQ(request->method, "GET") ||
		request->uri == NULL || request->uri->path == NULL)
	{
		serve_error(response, 400, "Bad Request");
		return FAILURE;
	}

	/*    printf("%s %s HTTP %d.%d\n", request->method, request->uri->complete,
	   request->version_major, request->version_minor); header =
	   request->headers.first; while (header != NULL)
		{
			printf("%s: %s\n", header->name, header->value);
			header = header->next;
		}

		printf("URI breakdown:\n");
		printf("%s :// %s @ %s : %s %s ? %s # %s\n", request->uri->scheme,
	   request->uri->userinfo, request->uri->host, request->uri->port,
	   request->uri->spath, request->uri->querystring, request->uri->fragment);
		printf("Body: %d bytes long\n", request->length);
	  */
	uri_path = path_to_string(request->uri->path, documentroot);
	printf("Requested URI = %s\n", uri_path);

	if (serve_cgi(response, request) == SKIPPED)
	{
		stat(uri_path, &buf);
		if (S_ISDIR(buf.st_mode))
		{
			free(uri_path);
			path_push(request->uri->path, kv_string(global_config, "index", "index.html"));
			uri_path = path_to_string(request->uri->path, documentroot);
			if (stat(uri_path, &buf) != SUCCESS)
			{
				path_pop(request->uri->path);
				free(uri_path);
				uri_path = path_to_string(request->uri->path, ".");
				serve_index(response, uri_path);
			}
			else
			{
				serve_file(response, uri_path, FALSE);
			}
		}
		else
		{
			serve_file(response, uri_path, FALSE);
		}
	}

	free(uri_path);

	snprintf(clength, 15, "%zu", response->length);
	kv_push(response->headers, "Content-Length", clength);
	kv_push(response->headers, "Server", "server.c");

	return EXIT_SUCCESS;
}