/*
 * Created by victoria on 15.02.20.
*/

#pragma once

#include "../server/server.h"

struct http_request_s;
struct http_response_s;
struct server_s;

int
serve(struct server_s *server, struct http_request_s *request, struct http_response_s *response);
