/*
 * Created by victoria on 21.06.2020.
 */

#pragma once

#include <openssl/ssl.h>
#include <vhttpsl/bits/bytebuf.h>
#include <streams/buffered_pipe.h>

#include "http/session.h"

#define SOC_BUF_SIZE 256

struct vhttpsl_server_s;

typedef struct client_context_s
{
	struct http_session_s session;

	struct stream_s socket_stream;
	struct stream_s http_stream;
	struct stream_buffered_pipe_s pipe_in;
	struct stream_buffered_pipe_s pipe_out;
} *client_context_t;

client_context_t
client_context_create(int fd, struct vhttpsl_server_s *server, SSL_CTX *ssl_ctx);

void
client_context_destroy(client_context_t context);
