#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "cgi.h"
#include "config.h"
#include "http.h"
#include "mime.h"
#include "path.h"
#include "server.h"
#include "url.h"
#include "serve/serve.h"
#include <src/options.h>

char *documentroot;

int
get_client_addr(int sockfd, char **addr_out, int *port)
{
	struct sockaddr_storage addr;
	socklen_t addrlen;
	char *addr_s;

	addrlen = sizeof(addr);

	if (getpeername(sockfd, (struct sockaddr *) &addr, &addrlen) != SUCCESS)
	{
		return FAILURE;
	}
	addr_s = malloc(64);
	if (addr.ss_family == AF_INET)
	{
		*port = ((struct sockaddr_in *) &addr)->sin_port;
		inet_ntop(AF_INET, &((struct sockaddr_in *) &addr)->sin_addr, addr_s, 64);
	}
	else
	{
		*port = ((struct sockaddr_in6 *) &addr)->sin6_port;
		inet_ntop(AF_INET, &((struct sockaddr_in6 *) &addr)->sin6_addr, addr_s, 64);
	}

	*addr_out = addr_s;

	return SUCCESS;
}

void
setup_document_root(void)
{
	const char *cfg_root;
	char *tmp;
	char cwd[PATH_MAX + 1];
	struct path_s *path;

	cfg_root = kv_string(global_config, "root", "www");
	if (*cfg_root == '/')
	{
		documentroot = malloc(strlen(cfg_root) + 1);
		strcpy(documentroot, cfg_root);
	}
	else
	{
		getcwd(cwd, sizeof(cwd));
		path = path_make(cwd);
		path_cat(path, cfg_root);
		tmp = path_to_string(path, "");
		path_free(path);
		documentroot = malloc(strlen(tmp) + 1);
		strcpy(documentroot, tmp);
		free(tmp);
	}

	printf("Serving: %s\n", documentroot);
}