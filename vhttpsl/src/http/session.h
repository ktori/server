//
// Created by victoria on 21.06.2020.
//

#pragma once

#include <vhttpsl/bits/bytebuf.h>

typedef struct http_session_s
{
} *http_session_t;

int
http_session_init(http_session_t session);

void
http_session_destroy(http_session_t session);
