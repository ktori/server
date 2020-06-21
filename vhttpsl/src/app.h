//
// Created by victoria on 20.06.2020.
//

#pragma once

#include <vhttpsl/http/request.h>
#include <vhttpsl/http/response.h>

struct app_route
{
	vhttpsl_callback_t callback;
};

struct vhttpsl_app_s
{
	struct app_route *routes;
	size_t route_count;
};

int
vhttpsl_app_execute(vhttpsl_app_t app, struct http_request_s *request, struct http_response_s *response);