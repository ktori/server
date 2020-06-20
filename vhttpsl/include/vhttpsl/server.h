//
// Created by victoria on 20.06.2020.
//

#pragma once

typedef struct vhttpsl_server_s *vhttpsl_server_t;

typedef void(*vhttpsl_callback_t)(void);

vhttpsl_server_t
vhttpsl_server_create();

void
vhttpsl_server_destroy(vhttpsl_server_t server);

void
vhttpsl_server_add_route(vhttpsl_server_t server, const char *route, vhttpsl_callback_t callback);

void
vhttpsl_server_listen_http(vhttpsl_server_t server, int port);

void
vhttpsl_server_listen_https(vhttpsl_server_t server, int port);
