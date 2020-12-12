/*
 * Created by victoria on 20.06.2020.
 */

#pragma once

#include <vhttpsl/app.h>
#include <vhttpsl/http/request.h>
#include <vhttpsl/http/response.h>

struct vhttpsl_app_s
{
	struct vhttpsl_callbacks_s callbacks;
};

int
vhttpsl_app_execute(vhttpsl_app_t app, struct http_request_s *request, struct http_response_s *response);

void
vhttpsl_app_destroy(vhttpsl_app_t app);
