/*
 * Created by victoria on 15.02.20.
*/

#pragma once

struct http_response_s;
struct http_request_s;

int
serve_cgi(struct http_response_s *response, struct http_request_s *request);
