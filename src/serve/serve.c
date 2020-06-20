/*
 * Created by victoria on 15.02.20.
*/

#include "serve.h"
#include "vhttpsl/bits/kv.h"
#include "file.h"
#include "index.h"
#include "vhttpsl/bits/path.h"
#include "../lib/config.h"
#include "../lib/http.h"
#include "error.h"
#include "../server.h"
#include "cgi.h"
#include "vhttpsl/http/url.h"
#include "../http/request.h"
#include "../http/response.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>

int
serve(struct server_s *server, struct http_request_s *request, struct http_response_s *response)
{
	char *uri_path;
	struct stat buf;

	memset(&buf, 0, sizeof(buf));

	response->version_major = 1;
	response->version_minor = 1;
	response->status = HTTP_S_OK;

	if (request == NULL || request->method != HTTP_METHOD_GET ||
		request->uri == NULL || request->uri->path == NULL)
	{
		serve_error(&server->config, response, 400, "Bad Request");
		return EXIT_FAILURE;
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
	uri_path = path_to_string(request->uri->path, server->config.root);
	printf("Requested URI = %s\n", uri_path);

	if (serve_cgi(response, request) == -2)
	{
		stat(uri_path, &buf);
		if (S_ISDIR(buf.st_mode))
		{
			free(uri_path);
			path_push(request->uri->path, server->config.index);
			uri_path = path_to_string(request->uri->path, server->config.root);
			if (stat(uri_path, &buf) != EXIT_SUCCESS)
			{
				path_pop(request->uri->path);
				free(uri_path);
				uri_path = path_to_string(request->uri->path, ".");
				serve_index(&server->config, response, uri_path);
			}
			else
			{
				serve_file(&server->config, response, uri_path, FALSE);
			}
		}
		else
		{
			serve_file(&server->config, response, uri_path, FALSE);
		}
	}

	free(uri_path);

	return EXIT_SUCCESS;
}
