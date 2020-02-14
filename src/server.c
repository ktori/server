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

#define LISTEN_BACKLOG 10
#define FORK_CLIENTS TRUE

struct kv_list_s *config;
char *documentroot;

/*
 *  TODO Content-Length
 */
int
recv_http(int sock, char **buf)
{
	int total = 0, received = 0, size = 128;
	char *buffer;

	buffer = malloc(size);

	received = recv(sock, buffer, size - 1, 0);

	while (received >= 0)
	{
		if (received == 0)
		{
			*buf = buffer;
			return total;
		}
		else
		{
			total += received;
			if (strstr(buffer, "\r\n\r\n") != NULL)
			{
				/* TODO POST request (body) */
				*buf = buffer;
				return total;
			}
			if (total >= size - 1)
			{
				size *= 2;
				buffer = realloc(buffer, size);
			}
		}
		received = recv(sock, buffer + total, size - total - 1, 0);
	}
	free(buffer);
	return received;
}

int
recv_all(int sock, char *buffer, int length)
{
	return recv(sock, buffer, length, 0);
	/*int total = 0;
	int received;
	do {
		received = recv(sock, buffer, length, 0);
		printf("received %d\n", received);
		if (received > 0) {
			buffer += received;
			length -= received;
			total += received;
		}
	} while (received > 0 && length > 0);
	return total;*/
}

int
send_all(int sock, char *buffer, int length)
{
	int sent;
	while (length > 0)
	{
		sent = send(sock, buffer, length, 0);
		if (sent < 1)
			return 1;
		buffer += sent;
		length -= sent;
	}
	return 0;
}

int server_socket = -1;
char server_running = 1;

void
sighandler(int signum)
{
	printf("Received signal %d\n", signum);
	if (server_socket >= 0)
		srv_cleanup();
	exit(0);
}

in_port_t
get_in_port(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return (((struct sockaddr_in *) sa)->sin_port);
	}
	return (((struct sockaddr_in6 *) sa)->sin6_port);
}

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

int
srv_setup(int *sockfd_out)
{
	struct addrinfo hints, *info = 0, *j;
	int status, sockfd;
	in_port_t port;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(0, kv_string(config, "port", "8080"), &hints, &info);
	if (status != 0)
	{
		printf("[error] getaddrinfo returned %d\n", status);
		return 1;
	}

	for (j = info; j != 0; j = j->ai_next)
	{
		sockfd = socket(j->ai_family, j->ai_socktype, j->ai_protocol);
		if (sockfd == -1)
		{
			printf("socket() failed\n");
			continue;
		}
		status = bind(sockfd, j->ai_addr, j->ai_addrlen);
		if (status == -1)
		{
			printf("bind() failed, errno %d\n", errno);
			close(sockfd);
			continue;
		}
		break;
	}
	if (info == 0)
	{
		printf("no addrinfo\n");
		return 1;
	}
	port = ntohs(get_in_port((struct sockaddr *) info->ai_addr));
	freeaddrinfo(info);

	struct timeval tv;
	tv.tv_sec = 8;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	/* fnctl(sockfd, F_SETFL, O_NONBLOCK); */

	listen(sockfd, LISTEN_BACKLOG);
	*sockfd_out = sockfd;

	printf("server (%d) is listening on port %hd\n", *sockfd_out, port);
	return 0;
}

void
serve_string(struct http_response_s *response, const char *string)
{
	int len = strlen(string);
	kv_push(response->headers, "Content-Type", "text/plain");
	response->body = malloc(len);
	strncpy(response->body, string, len);
	response->length = len;
}

void
serve_error(struct http_response_s *response, int error, const char *detail)
{
	char errcfg[32];
	snprintf(errcfg, 32, "err.%d", error);

	response->code = error;

	if (kv_isset(config, errcfg) == TRUE)
	{
		if (serve_file(response, kv_string(config, errcfg, "error.html"), TRUE) ==
			SUCCESS)
		{
			return;
		}
	}

	serve_string(response, detail);
}

status_t
serve_file(struct http_response_s *response, const char *filename, bool noerr)
{
	FILE *file;
	int length;
	printf("trying to serve %s\n", filename);
	if (access(filename, R_OK) != SUCCESS)
	{
		if (noerr == FALSE)
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
		return FAILURE;
	}

	file = fopen(filename, "r");
	if (file == NULL)
	{
		if (noerr == FALSE)
		{
			serve_error(response, 500, "Internal server error");
		}
		return FAILURE;
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
	return SUCCESS;
}

void
serve_index(struct http_response_s *response, const char *folder)
{
	struct dirent *dp;
	DIR *dir;
	char pathbuf[512];
	char *body;
	char *tmp_name;
	char *tmp_realname;
	int total, len;
	struct stat filestat;
	struct path_s *path;

	snprintf(pathbuf, 512, "%s/%s", documentroot, folder);

	dir = opendir(pathbuf);
	if (dir == NULL)
	{
		serve_error(response, 404, "Not Found");
		return;
	}
	total = 0;
	body = calloc(1, 1024 * 16);

	len =
			sprintf(body,
					"<html><head><title>Index of %s</title><meta charset="
					"\"UTF-8\"><base href=\"%s\"></head><body><h1>Index of %s</h1><hr>"
					"<table style=\"font-family:monospace\">",
					folder,
					folder,
					folder);
	total += len;

	dp = readdir(dir);

	while (dp != NULL)
	{
		path = path_make(folder);
		path_push(path, dp->d_name);
		tmp_name = path_to_string(path, "");
		tmp_realname = path_to_string(path, documentroot);
		printf("file: %s %s\n", tmp_realname, tmp_name);
		if (stat(tmp_realname, &filestat) == SUCCESS)
		{
			len = snprintf(body + total,
						   1024 * 16 - total,
						   "<tr><td><a href=\"%s\""
						   ">%s</a></td><td>",
						   tmp_name,
						   dp->d_name);
			total += len;
			if (!S_ISDIR(filestat.st_mode))
			{
				if (sizeof(off_t) == sizeof(long long))
				{
					len = snprintf(body + total,
								   1024 * 16 - total,
								   "%lld bytes",
								   (long long) filestat.st_size);
				}
				else
				{
					len = snprintf(body + total,
								   1024 * 16 - total,
								   "%ld bytes",
								   (long) filestat.st_size);
				}
			}
			else
			{
				len = snprintf(body + total, 1024 * 16 - total, "Directory");
			}
			total += len;
		}
		path_free(path);
		free(path);
		free(tmp_name);
		free(tmp_realname);
		dp = readdir(dir);
	}

	len = snprintf(body + total, 1024 * 16 - total, "</table></body></html>");

	response->body = body;
	response->length = total;

	closedir(dir);
}

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
			path_push(request->uri->path, kv_string(config, "index", "index.html"));
			uri_path = path_to_string(request->uri->path, documentroot);
			if (stat(uri_path, &buf) != SUCCESS)
			{
				path_pop(request->uri->path);
				free(uri_path);
				uri_path = path_to_string(request->uri->path, "./");
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

	snprintf(clength, 15, "%d", response->length);
	kv_push(response->headers, "Content-Length", clength);
	kv_push(response->headers, "Server", "server.c");

	return SUCCESS;
}

int
srv_process_socket(int sockfd, struct sockaddr_storage *addr)
{
	struct http_response_s *response;
	struct http_request_s *request;
	FILE *rq_log;
	char *rq_buffer;
	char *rs_buffer;
	int response_length;
	int request_length;

	/*    rq_buffer = calloc(8192, 1);
		request_length = recv_all(sockfd, rq_buffer, 8192);*/
	request_length = recv_http(sockfd, &rq_buffer);

	if (request_length >= 0)
	{
		rq_log = fopen("requests.log", "a");
		request = http_request_from_buffer(rq_buffer, request_length);
		fprintf(rq_log, "--- REQUEST ---\n\n%s\n", rq_buffer);
		fclose(rq_log);
		request->sockfd = sockfd;
	}
	else
	{
		request = NULL;
	}

	response = calloc(1, sizeof(struct http_response_s));
	response->headers = kv_create();

	serve(request, response);

	response_length = http_response_length(response);
	rs_buffer = calloc(response_length + 2, 1);
	http_response_to_buffer(response, rs_buffer, response_length + 1);
	send_all(sockfd, rs_buffer, response_length + 1);

	if (request_length >= 0)
		free(rq_buffer);
	free(rs_buffer);

	http_response_free(response);
	if (request != NULL)
		http_request_free(request);

	return 0;
}

int
srv_listen()
{
	int client_socket;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;

	while (server_running)
	{
		client_addr_size = sizeof(client_addr);
		client_socket =
				accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_size);
		if (client_socket >= 0)
		{
			printf("client accepted (%d)\n", client_socket);
#if FORK_CLIENTS == TRUE
			if (fork() == 0)
			{
				srv_process_socket(client_socket, &client_addr);
				close(client_socket);
				close(server_socket);
				exit(0);
			}
#else
			srv_process_socket(client_socket, &client_addr);
#endif
			close(client_socket);
		}
		else
		{
			/*if (errno != EAGAIN)
				printf("accept error %d\n", errno);*/
			if (errno == EBADF)
			{
				printf("Bad socket FD\n");
				srv_cleanup();
				exit(1);
			}
		}
	}
	return 0;
}

int
srv_cleanup()
{
	printf("cleaning up\n");
	shutdown(server_socket, SHUT_RDWR);
	close(server_socket);
	return 0;
}

void
setup_document_root(void)
{
	const char *cfg_root;
	char *tmp;
	char cwd[PATH_MAX + 1];
	struct path_s *path;

	cfg_root = kv_string(config, "root", "www");
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

int
main(int argc, const char **argv)
{
	/*signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGSEGV, sighandler);*/
	signal(SIGCHLD, SIG_IGN);
	config = config_load("server.conf");

	setup_document_root();

	if (srv_setup(&server_socket) != 0)
	{
		printf("srv_setup failed\n");
		return 1;
	}
	if (srv_listen() != 0)
	{
		printf("srv_listen failed\n");
	}
	if (srv_cleanup() != 0)
	{
		printf("srv_cleanup failed\n");
	}
	return 0;
}