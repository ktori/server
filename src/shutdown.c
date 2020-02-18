/*
 * Created by victoria on 18.02.20.
*/

#include "shutdown.h"

#include <malloc.h>
#include <signal.h>

volatile int exit_flag = 0;

struct shutdown_hook_s
{
	void
	(*hook)(void *);

	void *data;
	size_t id;
	int done;

	struct shutdown_hook_s *next;
	struct shutdown_hook_s *prev;
};

static size_t next_id = 0;
static struct shutdown_hook_s *head = NULL;

static int
perform_shutdown()
{
	struct shutdown_hook_s *prev, *current = head;

	fprintf(stderr, "shutting down...\n");

	while (current != NULL && !current->done)
	{
		current->done = 1;
		current->hook(current->data);
		if (current->next == current)
			current->next = NULL;
		if (current->prev == current)
			current->prev = NULL;
		prev = current;
		current = current->next;
		if (prev->prev)
			prev->prev->next = prev->next;
		if (prev->next)
			prev->next->prev = prev->prev;
		free(prev);
	}

	return 0;
}

static void
handler(int num)
{
	exit_flag = 1;
	perform_shutdown();
}

int
graceful_shutdown_install()
{
	signal(SIGINT, handler);
	signal(SIGTERM, handler);

	return 0;
}

int
graceful_shutdown_add_hook(void (*hook_fn)(void *), void *user_data, size_t *out_id)
{
	struct shutdown_hook_s *hook = calloc(1, sizeof(struct shutdown_hook_s));

	hook->id = next_id;
	if (out_id)
		*out_id = next_id;
	next_id += 1;

	hook->hook = hook_fn;
	hook->data = user_data;

	if (head == NULL)
		head = hook;

	hook->prev = head->prev;
	hook->next = head;

	if (head->prev)
		head->prev->next = hook;
	head->prev = hook;

	return 0;
}

int
graceful_shutdown_undo_hook(size_t id)
{
	struct shutdown_hook_s *i, *found = NULL;

	if (head == NULL)
		return 1;

	i = head;
	do
	{
		if (i->id == id)
		{
			found = i;
			break;
		}
		i = i->next;
	}
	while (i != head && i != NULL);

	if (found == NULL)
		return 1;

	if (found->prev)
		found->prev->next = found->next;
	if (found->next)
		found->next->prev = found->prev;

	free(found);

	return 0;
}
