/*
 * Created by victoria on 15.02.20.
*/

#pragma once

struct http_response_s;

void
serve_error(struct http_response_s *response, int error, const char *detail);
