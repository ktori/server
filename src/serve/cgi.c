/*
 * Created by victoria on 15.02.20.
*/

#include "cgi.h"
#include "../cgi.h"
#include "../lib/url.h"
#include "error.h"
#include "../exec.h"
#include "../server.h"

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

int
serve_cgi(struct http_response_s *response, struct http_request_s *request)
{
	struct kv_list_s *env;
	struct kv_list_s *args;
	char **env_arr;
	char **args_arr;
	char *cmd;

	struct path_s *script;
	char *script_path;
	int status;

	char *cgi_output;
	int cgi_output_length;

	char *cgi_input;
	int cgi_input_length;

	cgi_input = "";
	cgi_input_length = 0;

	script = request->uri->path;

	if (cgi_is_script(script) != TRUE)
	{
		return -2;
	}
	if (cgi_prepare_environment(request, script, &cmd, &env, &args) != EXIT_SUCCESS)
	{
		serve_error(response, HTTP_SERVER_ERROR, "Internal Server Error");
		return EXIT_FAILURE;
	}

	script_path = path_to_string(script, documentroot);

	env_arr = kv_to_env(env);
	args_arr = kv_to_args(args);

	if (cmd == NULL)
	{
		cmd = script_path;
	}

	status = pexec(cmd,
				   (const char **) args_arr,
				   (const char **) env_arr,
				   cgi_input,
				   cgi_input_length,
				   &cgi_output,
				   &cgi_output_length);

	if (status == EXIT_SUCCESS)
	{
		response->raw = TRUE;
		response->body = cgi_output;
		response->length = cgi_output_length;
	}
	else
	{
		fprintf(stderr, "cgi exec failed: %d\n", status);
		serve_error(response, HTTP_SERVER_ERROR, "Internal Server Error");
	}

	kv_free(env);
	kv_free(args);
	string_array_free(env_arr);
	string_array_free(args_arr);
	free(script_path);

	return status;
}