//
// Created by victoria on 20.06.2020.
//

#include <vhttpsl/app.h>
#include "app.h"

#include <stdio.h>
#include <stdlib.h>

vhttpsl_app_t
vhttpsl_app_create()
{
	vhttpsl_app_t app = calloc(1, sizeof(*app));

	fprintf(stderr, "TODO: vhttpsl_app_create\n");
	app->routes = calloc(4, sizeof(struct app_route));

	return app;
}

void
vhttpsl_app_destroy(vhttpsl_app_t *app)
{
	fprintf(stderr, "TODO: vhttpsl_app_destroy\n");

	free(*app);
	*app = NULL;
}

void
vhttpsl_app_route_add(vhttpsl_app_t app, const char *route, vhttpsl_callback_t callback)
{
	fprintf(stderr, "TODO: vhttpsl_app_add_route\n");

	app->routes[app->route_count].callback = callback;
	app->route_count++;
}

int
vhttpsl_app_execute(vhttpsl_app_t app, struct http_request_s *request, struct http_response_s *response)
{
	size_t i = 0;

	for (i = 0; i < app->route_count; ++i)
		app->routes[i].callback(request, response);

	return EXIT_SUCCESS;
}
