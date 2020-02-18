/*
 * Created by victoria on 15.02.20.
*/

#pragma once

struct http_response_s;
struct server_config_s;

void
serve_error(struct server_config_s *config, struct http_response_s *response, int error, const char *detail);
