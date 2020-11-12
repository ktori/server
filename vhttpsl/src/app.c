/*
 * Created by victoria on 20.06.2020.
 */

#include <vhttpsl/app.h>
#include "app.h"

#include <stdio.h>
#include <stdlib.h>

vhttpsl_app_t
vhttpsl_app_create(struct vhttpsl_callbacks_s callbacks)
{
	vhttpsl_app_t app = calloc(1, sizeof(*app));

	fprintf(stderr, "TODO: vhttpsl_app_create\n");
	app->callbacks = callbacks;

	return app;
}

void
vhttpsl_app_destroy(vhttpsl_app_t *app)
{
	fprintf(stderr, "TODO: vhttpsl_app_destroy\n");

	if ((*app)->callbacks.destroy)
		(*app)->callbacks.destroy(*app, (*app)->callbacks.user_data);

	free(*app);
	*app = NULL;
}

int
vhttpsl_app_execute(vhttpsl_app_t app, struct http_request_s *request, struct http_response_s *response)
{
	if (app->callbacks.http)
		app->callbacks.http(app, app->callbacks.user_data, request, response);

	return EXIT_SUCCESS;
}
