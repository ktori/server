#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "http.h"
#include "kv.h"

extern struct kv_list_s *config;
extern char *documentroot;

int
get_client_addr(int sockfd, char **addr, int *port);

int
srv_cleanup();

void
setup_document_root(void);

void
serve_index(struct http_response_s *response, const char *folder);

void
serve_error(struct http_response_s *response, int error, const char *detail);

status_t
serve_file(struct http_response_s *response, const char *filename, bool noerr);

void
serve_string(struct http_response_s *response, const char *string);

int
serve(struct http_request_s *request, struct http_response_s *response);

#endif /* SERVER_H */