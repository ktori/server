/*
 * Created by victoria on 21.06.2020.
 */

#include "socket_context.h"

#include "ssl_backend.h"

#include <stdlib.h>
#include <streams/backend/fd.h>

client_context_t
client_context_create(int fd, struct vhttpsl_server_s *server, SSL_CTX *ssl_ctx)
{
	client_context_t cl = calloc(1, sizeof(*cl));

	stream_init(&cl->socket_stream);
	stream_init(&cl->http_stream);

	if (ssl_ctx)
	{
		ssl_backend(&cl->socket_stream.backend, ssl_ctx, fd, 0);
	}
	else
	{
		fd_backend(&cl->socket_stream.backend, fd, fd, 0);
	}

	http_session_init(&cl->session, server);

	stream_backend_init(&cl->http_stream.backend,
						&cl->session,
						(stream_backend_read_fn)http_session_read,
						(stream_backend_write_fn)http_session_write,
						NULL);

	stream_buffered_pipe_init(&cl->pipe_in, &cl->socket_stream, &cl->http_stream, SOC_BUF_SIZE);
	stream_buffered_pipe_init(&cl->pipe_out, &cl->http_stream, &cl->socket_stream, SOC_BUF_SIZE);

	return cl;
}

void
client_context_destroy(client_context_t context)
{

	http_session_destroy(&context->session);
	stream_buffered_pipe_destroy(&context->pipe_in);
	stream_buffered_pipe_destroy(&context->pipe_out);
	stream_backend_destroy(&context->socket_stream.backend);
	stream_backend_destroy(&context->http_stream.backend);
	stream_destroy(&context->http_stream);
	stream_destroy(&context->socket_stream);

	free(context);
}
