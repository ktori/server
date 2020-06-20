/*
 * Created by victoria on 18.02.20.
*/

#pragma once

int
graceful_shutdown_install();

int
graceful_shutdown_add_hook(void (*hook)(void *, int), void *user_data, int *out_id);

int
graceful_shutdown_undo_hook(int id);
