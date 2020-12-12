/*
 * Created by victoria on 11/30/20.
 */

#include "ssl_backend.h"

#include <errno.h>
#include <openssl/err.h>
#include <unistd.h>

typedef struct ssl_backend_data_s
{
	SSL *ssl;
	int fd;
	char close_on_shutdown;
	char is_ssl_ready;
} * ssl_backend_data_t;

static int
ssl_accept(ssl_backend_data_t self)
{
	int ret = SSL_accept(self->ssl);

	if (ret == 1)
		return EXIT_SUCCESS;

	switch (SSL_get_error(self->ssl, ret))
	{
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			errno = EAGAIN;

			return EXIT_FAILURE;
		default:
			ERR_print_errors_fp(stderr);
			errno = EIO;

			return EXIT_FAILURE;
	}
}

size_t
ssl_backend_read(ssl_backend_data_t self, char *out, size_t size)
{
	int ret = SSL_read(self->ssl, out, size);

	if (!self->is_ssl_ready && ssl_accept(self))
		return STREAM_BACKEND_IO_ERROR;

	switch (SSL_get_error(self->ssl, ret))
	{
		case SSL_ERROR_NONE:
			return ret;
		case SSL_ERROR_ZERO_RETURN:
			errno = ERANGE;
			return 0;
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			errno = EAGAIN;
			break;
		default:
			ERR_print_errors_fp(stderr);
			errno = EIO;
			break;
	}

	return STREAM_BACKEND_IO_ERROR;
}

size_t
ssl_backend_write(ssl_backend_data_t self, const char *in, size_t size)
{
	int ret;

	if (!self->is_ssl_ready && ssl_accept(self))
		return EXIT_FAILURE;

	ret = SSL_write(self->ssl, in, size);

	switch (SSL_get_error(self->ssl, ret))
	{
		case SSL_ERROR_NONE:
			return ret;
		case SSL_ERROR_ZERO_RETURN:
			errno = ERANGE;
			return 0;
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			errno = EAGAIN;
			break;
		default:
			errno = EIO;
			break;
	}

	return STREAM_BACKEND_IO_ERROR;
}

void
ssl_backend_destroy(ssl_backend_data_t self)
{
	if (self->ssl)
	{
		SSL_shutdown(self->ssl);
		SSL_free(self->ssl);
		self->ssl = NULL;
	}

	if (self->close_on_shutdown && self->fd >= 0)
	{
		close(self->fd);
		self->fd = -1;
	}
}

int
ssl_backend(struct stream_backend_s *backend, SSL_CTX *ssl_ctx, int fd, char close_on_shutdown)
{
	ssl_backend_data_t impl = calloc(1, sizeof(*impl));
	if (!impl)
		return EXIT_FAILURE;

	impl->ssl = SSL_new(ssl_ctx);

	if (!impl->ssl)
	{
		free(impl);

		return EXIT_FAILURE;
	}

	impl->fd = fd;
	impl->close_on_shutdown = close_on_shutdown;
	impl->is_ssl_ready = 0;

	if (!SSL_set_fd(impl->ssl, fd))
	{
		SSL_free(impl->ssl);
		free(impl);

		return EXIT_FAILURE;
	}

	backend->impl = impl;
	backend->def.read = (stream_backend_read_fn)ssl_backend_read;
	backend->def.write = (stream_backend_write_fn)ssl_backend_write;
	backend->def.destroy = (stream_backend_destroy_fn)ssl_backend_destroy;

	return EXIT_SUCCESS;
}
