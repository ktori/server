#pragma once

#include "common.h"
#include "http.h"
#include "kv.h"

extern char *documentroot;

int
get_client_addr(int sockfd, char **addr, int *port);

int
srv_cleanup();

void
setup_document_root(void);
