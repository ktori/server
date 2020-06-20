//
// Created by victoria on 20.06.2020.
//

#pragma once

#include "../http/methods.h"

typedef struct vhttpsl_app_request_s
{
	enum http_method method;
} *vhttpsl_app_request_t;
