//
// Created by victoria on 21.06.2020.
//

#pragma once

#include <vhttpsl/bits/bytebuf.h>

typedef struct http_session_s
{
	struct bytebuf_s buf_in;
	struct bytebuf_s buf_out;
} *http_session_t;

int
http_session_init(http_session_t session);

void
http_session_destroy(http_session_t session);

int
http_session_read(http_session_t session, char *buf, size_t size);

int
http_session_write(http_session_t session, const char *buf, size_t size);