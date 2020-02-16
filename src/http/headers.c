/*
 * Created by victoria on 16.02.20.
*/

#include "request.h"
#include "../def.h"
#include "../lib/kv.h"
#include "../server/client.h"

int
headers_read(struct http_request_s *request)
{
	struct bytebuf_s *buf = &request->read_buffer;

	enum state
	{
		H_ERROR,
		H_BEGIN,
		H_FIELD_NAME_BEGIN,
		H_FIELD_NAME,
		H_FIELD_SEP,
		H_FIELD_SEP_OWS,
		H_FIELD_VALUE_BEGIN,
		H_FIELD_VALUE,
		H_FIELD_VALUE_NEXT,
		H_DONE
	};

	enum state current_state = H_BEGIN;

	size_t out_read;
	bool cr_flag = FALSE;
	bool set_cr_flag = FALSE;
	size_t name_begin = 0, name_end = 0, value_begin = 0;

	do
	{
		while (buf->pos_read == buf->pos_write)
		{
			bytebuf_ensure_write(buf, 256);
			if (client_read_some(request->client, bytebuf_write_ptr(buf), bytebuf_write_size(buf), &out_read) !=
				EXIT_SUCCESS)
				return EXIT_FAILURE;
			buf->pos_write += out_read;
		}

		const char *current = bytebuf_read_ptr(buf);

		if (set_cr_flag)
		{
			set_cr_flag = FALSE;
			cr_flag = TRUE;
		}
		if (*current == '\r')
		{
			set_cr_flag = TRUE;
		}
		bool crlf = cr_flag && *current == '\n';
		bool consume = TRUE;

		switch (current_state)
		{
			case H_BEGIN:
				current_state = H_FIELD_NAME_BEGIN;
				break;
			case H_FIELD_NAME_BEGIN:
				name_begin = buf->pos_read;
				current_state = H_FIELD_NAME;
				consume = FALSE;
				break;
			case H_FIELD_NAME:
				if (*current == ':')
				{
					consume = FALSE;
					name_end = buf->pos_read;
					current_state = H_FIELD_SEP;
				}
				break;
			case H_FIELD_SEP:
				if (*current != ':')
					current_state = H_ERROR;
				else
				{
					current_state = H_FIELD_SEP_OWS;
				}
				break;
			case H_FIELD_SEP_OWS:
				if (*current != ' ' && *current != '\t')
				{
					consume = FALSE;
					current_state = H_FIELD_VALUE_BEGIN;
				}
				break;
			case H_FIELD_VALUE_BEGIN:
				value_begin = buf->pos_read;
				current_state = H_FIELD_VALUE;
				consume = FALSE;
				break;
			case H_FIELD_VALUE:
				if (crlf)
				{
					kv_push_n(request->headers, buf->data + name_begin, name_end - name_begin, buf->data + value_begin,
							  buf->pos_read - value_begin - 2);
					current_state = H_FIELD_VALUE_NEXT;
				}
				break;
			case H_FIELD_VALUE_NEXT:
				if (*current == '\r' || crlf)
				{
					current_state = H_DONE;
				}
				else
				{
					current_state = H_FIELD_NAME_BEGIN;
					consume = FALSE;
				}
			default:
				break;
		}

		if (consume)
			request->read_buffer.pos_read += 1;
	}
	while (current_state != H_ERROR && current_state != H_DONE);

	if (current_state == H_ERROR)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
