/*
 * Created by victoria on 15.02.20.
*/

#pragma once

struct http_response_s;

void
serve_string(struct http_response_s *response, const char *string);
