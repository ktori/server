//
// Created by victoria on 20.06.2020.
//

#pragma once

#include <vhttpsl/app.h>

typedef struct vhttpsl_server_s *vhttpsl_server_t;

vhttpsl_server_t
vhttpsl_server_create(vhttpsl_app_t app);

void
vhttpsl_server_destroy(vhttpsl_server_t *server);

void
vhttpsl_server_listen_http(vhttpsl_server_t server, int port);

void
vhttpsl_server_listen_https(vhttpsl_server_t server, int port);
