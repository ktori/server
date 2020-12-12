/*
 * Created by victoria on 11/30/20.
 */

#pragma once

#include <streams/stream_backend.h>

#include <openssl/ssl.h>

int
ssl_backend(struct stream_backend_s *backend, SSL_CTX *ssl_ctx, int fd, char close_on_shutdown);
