#include "exec.h"
#include "lib/kv.h"
#include "lib/path.h"
#include "server.h"
#include "lib/url.h"
#include "lib/config.h"
#include "http/request.h"
#include "server/client.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char *
cgi_toupper(const char *string)
{
	char *result;
	int i;

	result = malloc(strlen(string) + 1);
	for (i = 0; i < strlen(string) + 1; ++i)
	{
		if (string[i] == '-')
			result[i] = '_';
		else
			result[i] = toupper(string[i]);
	}

	return result;
}

bool
cgi_is_script(struct path_s *path)
{
	struct path_node_s *node;
	int status;
	char *tmp;
	struct stat statbuf;

	if (path->head == NULL)
	{
		return FALSE;
	}

	tmp = strmake("loose.%s", path->tail->extension);

	if (kv_int(global_config, tmp, FALSE) == FALSE)
	{
		node = path->head;
		while (node != NULL)
		{
			if (STREQ(node->name, kv_string(global_config, "cgi", "cgi-bin")) == TRUE)
			{
				break;
			}
			if (node->next == NULL)
			{
				return FALSE;
			}
			node = node->next;
		}
	}

	free(tmp);

	tmp = path_to_string(path, documentroot);
	status = stat(tmp, &statbuf);
	free(tmp);

	if (status != EXIT_SUCCESS)
	{
		return FALSE;
	}
	tmp = strmake("exec.%s", path->tail->extension);
	if (statbuf.st_mode & S_IXUSR || (kv_int(global_config, tmp, FALSE) == TRUE))
	{
		if (S_ISREG(statbuf.st_mode))
		{
			return TRUE;
		}
	}
	return FALSE;
}

int
cgi_prepare_environment(struct http_request_s *request,
						struct path_s *script,
						char **cmd,
						struct kv_list_s **env_list,
						struct kv_list_s **arg_list)
{
	struct kv_list_s *env;
	struct kv_list_s *args;
	struct kv_node_s *header;

	char *addr;
	int port;
	int i;
	char port_s[16];
	char *path_rel;
	char *path_abs;
	char **interp;

	char *tmp1, *tmp2;

	if (get_client_addr(request->client->socket, &addr, &port) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	snprintf(port_s, 16, "%d", port);

	path_rel = path_to_string(script, "");
	path_abs = path_to_string(script, documentroot);

	env = kv_create();
	args = kv_create();

	kv_push(env, "DOCUMENT_ROOT", documentroot);
	kv_push(env, "PATH", "/usr/local/bin:/usr/bin:/bin");
	kv_push(env, "QUERY_STRING", request->uri->querystring);
	kv_push(env, "REMOTE_ADDR", addr);
	/* kv_push("REMOTE_HOST", ""); */
	kv_push(env, "REMOTE_PORT", port_s);
	/* kv_push("REMOTE_USER", ""); */
	const char *method = http_request_method_name(request);
	if (method != NULL)
		kv_push(env, "REQUEST_METHOD", method);
	if (request->uri->scheme != NULL)
		kv_push(env, "REQUEST_SCHEME", request->uri->scheme);
	if (request->uri->spath != NULL)
		kv_push(env, "REQUEST_URI", request->uri->spath);
	kv_push(env, "SCRIPT_FILENAME", path_abs);
	kv_push(env, "SCRIPT_NAME", path_rel);
	/*kv_push("SERVER_ADMIN", "");
	kv_push("SERVER_NAME", "");
	kv_push("SERVER_PORT", "");*/
	kv_push(env, "SERVER_SOFTWARE", "server.c");
	kv_push(env, "REDIRECT_STATUS", "1"); /* PHP fix */

	header = request->headers->head;

	while (header != NULL)
	{
		tmp1 = malloc(strlen(header->key) + 6);
		tmp2 = cgi_toupper(header->key);
		sprintf(tmp1, "HTTP_%s", tmp2);
		kv_push(env, tmp1, header->value);
		free(tmp1);
		free(tmp2);
		header = header->next;
	}

	tmp1 = strmake("cgi.%s", script->tail->extension);
	interp = kv_array(global_config, tmp1, ' ');
	free(tmp1);

	i = 0;
	while (interp[i] != NULL)
	{
		printf("interp arg %d: %s\n", i, interp[i]);
		kv_push(args, interp[i], "");
		++i;
	}
	*cmd = interp[0];

	kv_push(args, path_abs, "");

	*env_list = env;
	*arg_list = args;

	free(addr);
	free(path_rel);
	free(path_abs);

	return EXIT_SUCCESS;
}