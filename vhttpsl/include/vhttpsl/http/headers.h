/*
 * Created by victoria on 16.02.20.
*/
#pragma once

#include <vhttpsl/bits/bytebuf.h>
#include <vhttpsl/bits/kv.h>

typedef struct headers_read_state_s
{
	/* enum */ int step;
	int cr_flag;
	struct bytebuf_s name_buf;
	struct bytebuf_s value_buf;

} *headers_read_state_t;

typedef struct headers_write_state_s
{
	kv_node_t it;
	int step;
	int string_index;
	size_t field_len;
	size_t value_len;
} *headers_write_state_t;

void
headers_read_begin(headers_read_state_t state);

void
headers_read_reset(headers_read_state_t state);

void
headers_read_end(headers_read_state_t state);

/**
 * Read the HTTP headers from the buffer
 * Buffer is expected to begin with at the position of the first header field
 * @param state
 * @return
 * 	0 when reading is finished (CR LF CR LF reached),
 * 	-1 if there was an error
 * 	n number of bytes read otherwise
 */
int
headers_read(const char *buf, int size, headers_read_state_t state_ptr, kv_list_t out);

void
headers_write_begin(headers_write_state_t state, kv_list_t in);

void
headers_write_reset(headers_write_state_t state);

void
headers_write_end(headers_write_state_t state);

int
headers_write(char *buf, int size, headers_write_state_t state_ptr);

/* helpers */

long
headers_get_content_length(kv_list_t headers);