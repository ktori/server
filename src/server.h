#pragma once

#include "lib/str.h"
#include "lib/http.h"
#include "lib/kv.h"

extern char *documentroot;

int
get_client_addr(int sockfd, char **addr, int *port);

void
setup_document_root(void);