/*
 * Created by victoria on 20.06.2020.
 */

#pragma once

#include "app/request.h"
#include "app/response.h"

typedef struct vhttpsl_app_s *vhttpsl_app_t;

struct http_request_s;
struct http_response_s;

typedef void(*vhttpsl_http_callback_t)(vhttpsl_app_t app, void *user_data, struct http_request_s *request, struct http_response_s *response);
typedef void(*vhttpsl_destroy_callback_t)(vhttpsl_app_t app, void *user_data);

struct vhttpsl_callbacks_s {
	vhttpsl_http_callback_t http;
	vhttpsl_destroy_callback_t destroy;

	void *user_data;
};

vhttpsl_app_t
vhttpsl_app_create(struct vhttpsl_callbacks_s callbacks);

void
vhttpsl_app_destroy(vhttpsl_app_t *app);
