#pragma once

#include "vhttpsl/bits/str.h"
#include "lib/http.h"
#include "vhttpsl/bits/kv.h"

int
get_client_addr(int sockfd, char **addr, int *port);
