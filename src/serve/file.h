/*
 * Created by victoria on 15.02.20.
*/

#pragma once

struct http_response_s;

int
serve_file(struct http_response_s *response, const char *filename, char noerr);
