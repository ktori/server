/*
 * Created by victoria on 15.02.20.
*/

#pragma once

#include "../lib/config.h"

struct http_response_s;
struct server_config_s;

int
serve_file(struct server_config_s *config, struct http_response_s *response, const char *filename, char noerr);
