/*
 * Created by victoria on 16.02.20.
*/

#include <errno.h>
#include <string.h>
#include <vhttpsl/http/headers.h>
#include <assert.h>
#include <vhttpsl/bits/str.h>
#include "vhttpsl/http/request.h"
#include <vhttpsl/bits/def.h>
#include "vhttpsl/bits/kv.h"
#include "vhttpsl/http/status.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

enum step
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

void
headers_read_begin(headers_read_state_t state)
{
	memset(state, 0, sizeof(*state));

	state->step = H_BEGIN;

	bytebuf_init(&state->name_buf, 32);
	bytebuf_init(&state->value_buf, 64);
}

void
headers_read_end(headers_read_state_t state)
{
	bytebuf_destroy(&state->name_buf);
	bytebuf_destroy(&state->value_buf);
}

int
headers_read(const char *buf, int size, headers_read_state_t state_ptr, kv_list_t out)
{
	bool cr_flag = FALSE;
	const char *current;
	bool crlf, consume;

	struct headers_read_state_s s = *state_ptr;

	const char *name_begin = NULL, *value_begin = NULL;

	size_t i = 0;

	while (i < size && s.step != H_ERROR && s.step != H_DONE)
	{
		current = buf + i;

		if (s.cr_flag)
		{
			s.cr_flag = FALSE;
			cr_flag = TRUE;
		}

		if (*current == '\r')
			s.cr_flag = TRUE;

		crlf = cr_flag && *current == '\n';
		consume = TRUE;

		switch (s.step)
		{
			case H_BEGIN:
				s.step = H_FIELD_NAME_BEGIN;
				if (*current == '\r')
					s.step = H_DONE;
				consume = FALSE;
				break;
			case H_FIELD_NAME_BEGIN:
				name_begin = current;
				s.name_buf.pos_write = 0;
				s.name_buf.pos_read = 0;
				s.step = H_FIELD_NAME;
				consume = FALSE;
				break;
			case H_FIELD_NAME:
				if (*current == ':')
				{
					consume = FALSE;
					if (s.name_buf.pos_read != s.name_buf.pos_write)
					{
						assert(name_begin);

						memcpy(bytebuf_write_ptr(&s.name_buf), name_begin, s.name_buf.pos_read - s.name_buf.pos_write);
						s.name_buf.pos_write = s.name_buf.pos_read;
					}
					s.step = H_FIELD_SEP;
				}
				else
				{
					if (!name_begin)
						name_begin = current;
					++s.name_buf.pos_read;
				}
				break;
			case H_FIELD_SEP:
				if (*current != ':')
					s.step = H_ERROR;
				else
					s.step = H_FIELD_SEP_OWS;
				break;
			case H_FIELD_SEP_OWS:
				if (*current != ' ' && *current != '\t')
				{
					consume = FALSE;
					s.step = H_FIELD_VALUE_BEGIN;
				}
				break;
			case H_FIELD_VALUE_BEGIN:
				value_begin = current;
				s.value_buf.pos_read = 0;
				s.value_buf.pos_write = 0;
				s.step = H_FIELD_VALUE;
				consume = FALSE;
				break;
			case H_FIELD_VALUE:
				if (crlf)
				{
					if (s.value_buf.pos_read != s.value_buf.pos_write)
					{
						assert(value_begin);

						s.value_buf.pos_read -= 1;
						bytebuf_ensure_write(&s.value_buf, s.value_buf.pos_read - s.value_buf.pos_write);
						memcpy(bytebuf_write_ptr(&s.value_buf), value_begin,
							   s.value_buf.pos_read - s.value_buf.pos_write);
						s.value_buf.pos_write = s.value_buf.pos_read;
					}

					kv_push_n(out, s.name_buf.data, s.name_buf.pos_write, s.value_buf.data, s.value_buf.pos_write);
					s.step = H_FIELD_VALUE_NEXT;
				}
				else
				{
					if (!value_begin)
						value_begin = current;
					++s.value_buf.pos_read;
				}
				break;
			case H_FIELD_VALUE_NEXT:
				s.step = H_BEGIN;
				consume = FALSE;
				break;
			default:
				break;
		}

		if (consume)
			i += 1;
	}

	if (s.name_buf.pos_write != s.name_buf.pos_read)
	{
		assert(name_begin);

		memcpy(bytebuf_write_ptr(&s.name_buf), name_begin, s.name_buf.pos_read - s.name_buf.pos_write);
		s.name_buf.pos_write = s.name_buf.pos_read;
	}

	if (s.value_buf.pos_write != s.value_buf.pos_read)
	{
		assert(value_begin);

		memcpy(bytebuf_write_ptr(&s.value_buf), value_begin,
			   s.value_buf.pos_read - s.value_buf.pos_write);
		s.value_buf.pos_write = s.value_buf.pos_read;
	}

	*state_ptr = s;

	if (state_ptr->step == H_ERROR)
		return -1;

	return i;
}

enum write_step
{
	WS_BEGIN_FIELD,
	WS_FIELD,
	WS_FIELD_SEP,
	WS_FIELD_SEP_WHITESPACE,
	WS_BEGIN_VALUE,
	WS_VALUE,
	WS_CR,
	WS_LF,
	WS_DONE
};

void
headers_write_begin(headers_write_state_t state, kv_list_t list)
{
	memset(state, 0, sizeof(*state));

	state->it = list->head;
}

void
headers_write_end(headers_write_state_t state)
{
}

int
headers_write(char *buf, int size, headers_write_state_t state_ptr)
{
	int i = 0;
	struct headers_write_state_s state = *state_ptr;
	int written;

	while (i < size && state.step != WS_DONE)
	{
		switch (state.step)
		{
			case WS_BEGIN_FIELD:
				state.field_len = strlen(state.it->key);
				state.string_index = 0;
				state.step = WS_FIELD;
				break;
			case WS_FIELD:
				written = MIN(state.field_len - state.string_index, size - i);
				memcpy(buf + i, state.it->key + state.string_index, written);
				state.string_index += written;
				i += written;
				if (state.string_index == state.field_len)
					state.step = WS_FIELD_SEP;
				break;
			case WS_FIELD_SEP:
				buf[i++] = ':';
				state.step = WS_FIELD_SEP_WHITESPACE;
				break;
			case WS_FIELD_SEP_WHITESPACE:
				buf[i++] = ' ';
				state.step = WS_BEGIN_VALUE;
				break;
			case WS_BEGIN_VALUE:
				state.value_len = strlen(state.it->value);
				state.string_index = 0;
				state.step = WS_VALUE;
				break;
			case WS_VALUE:
				written = MIN(state.value_len - state.string_index, size - i);
				memcpy(buf + i, state.it->value + state.string_index, written);
				state.string_index += written;
				i += written;
				if (state.string_index == state.value_len)
					state.step = WS_CR;
				break;
			case WS_CR:
				buf[i++] = '\r';
				state.step = WS_LF;
				break;
			case WS_LF:
				buf[i++] = '\n';
				if (state.it)
				{
					state.it = state.it->next;
					if (state.it)
						state.step = WS_BEGIN_FIELD;
					else
						state.step = WS_CR;
				}
				else
					state.step = WS_DONE;
				break;
		}

	}

	*state_ptr = state;

	return i;
}

void
headers_read_reset(headers_read_state_t state)
{
	state->step = H_BEGIN;
	state->name_buf.pos_write = 0;
	state->name_buf.pos_read = 0;
	state->value_buf.pos_write = 0;
	state->value_buf.pos_read = 0;
	state->cr_flag = 0;
}

long
headers_get_content_length(kv_list_t headers)
{
	kv_node_t node;
	long result;
	char *end_ptr;

	if (!headers)
		return -3;

	node = headers->head;

	while (node)
	{
		if (0 == stricmp("content-length", node->key))
		{
			result = strtol(node->value, &end_ptr, 10);

			if (result == 0)
			{
				if (end_ptr != node->value + strlen(node->value))
					return -1;
			}

			return result;
		}
		node = node->next;
	}

	return -2;
}
