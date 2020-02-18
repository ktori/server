/*
 * Created by victoria on 18.02.20.
*/

#pragma once

#include <stddef.h>

extern volatile int exit_flag;

int
graceful_shutdown_install();

int
graceful_shutdown_add_hook(void (*hook)(void *), void *user_data, size_t *out_id);

int
graceful_shutdown_undo_hook(size_t id);
