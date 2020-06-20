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
}