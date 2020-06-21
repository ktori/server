/*
 * Created by victoria on 21.06.2020.
 */

#include <vhttpsl/http/headers.h>
#include <vhttpsl/bits/kv.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

const char TEST_CASE[] = "User-Agent: test\r\n"
						 "Referer: http://localhost:8080\r\n"
						 "Accept: text/plain\r\n\r\n";

#define BUFFER_SIZE 8

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int
main()
{
	struct headers_read_state_s read_state;
	struct headers_write_state_s write_state;
	kv_list_t header_list = kv_create();
	kv_node_t node = NULL;
	struct bytebuf_s write_buffer;
	int retval;
	int i;
	int s;

	headers_read_begin(&read_state);

	for (i = 0; i < sizeof(TEST_CASE);)
	{
		s = MIN(BUFFER_SIZE, sizeof(TEST_CASE) - i);
		retval = headers_read(TEST_CASE + i, s, &read_state, header_list);
		printf("read(%d, %d) returned: %d\n", i, s, retval);

		if (retval == 0)
			break;
		if (retval < 0)
			exit(1);

		i += retval;
	}

	/* verify parsed header */
	assert(header_list->count == 3);
	node = header_list->head;
	assert(!strcmp(node->key, "User-Agent"));
	assert(!strcmp(node->value, "test"));
	node = node->next;
	assert(!strcmp(node->key, "Referer"));
	assert(!strcmp(node->value, "http://localhost:8080"));
	node = node->next;
	assert(!strcmp(node->key, "Accept"));
	assert(!strcmp(node->value, "text/plain"));
	node = node->next;
	assert(!node);

	/* convert back to string */
	headers_write_begin(&write_state, header_list);
	bytebuf_init(&write_buffer, 64);

	do
	{
		bytebuf_ensure_write(&write_buffer, BUFFER_SIZE);
		retval = headers_write(bytebuf_write_ptr(&write_buffer), BUFFER_SIZE, &write_state);

		printf("write returned: %d\n", retval);

		if (retval < 0)
			exit(1);
		else
			write_buffer.pos_write += retval;
	}
	while (retval);

	assert(!memcmp(bytebuf_read_ptr(&write_buffer), TEST_CASE, sizeof(TEST_CASE) - 1));

	bytebuf_destroy(&write_buffer);
	headers_write_end(&write_state);
	headers_read_end(&read_state);
	kv_free(header_list);

	return 0;
}
