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
#include "lib/config.h"
#include "lib/http.h"
#include "lib/mime.h"
#include "lib/path.h"
#include "server.h"
#include "lib/url.h"
#include "serve/serve.h"
#include <conf/config.h>

int
get_client_addr(int sockfd, char **addr_out, int *port)
{
	struct sockaddr_storage addr;
	socklen_t addrlen;
	char *addr_s;

	addrlen = sizeof(addr);

	if (getpeername(sockfd, (struct sockaddr *) &addr, &addrlen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
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

	return EXIT_SUCCESS;
}
