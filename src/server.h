#pragma once

#include "lib/str.h"
#include "lib/http.h"
#include "lib/kv.h"

int
get_client_addr(int sockfd, char **addr, int *port);
