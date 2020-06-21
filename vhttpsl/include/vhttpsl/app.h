//
// Created by victoria on 20.06.2020.
//

#pragma once

#include "app/request.h"
#include "app/response.h"

typedef struct vhttpsl_app_s *vhttpsl_app_t;

typedef struct http_response_s *http_response_t;
typedef struct http_request_s *http_request_t;

typedef void(*vhttpsl_callback_t)(http_request_t request, http_response_t response);

vhttpsl_app_t
vhttpsl_app_create();

void
vhttpsl_app_destroy(vhttpsl_app_t *app);

void
vhttpsl_app_route_add(vhttpsl_app_t app, const char *route, vhttpsl_callback_t callback);

void
vhttpsl_app_process_request(vhttpsl_app_t app, vhttpsl_app_request_t request, vhttpsl_response_t response);