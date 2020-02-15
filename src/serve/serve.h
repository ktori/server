/*
 * Created by victoria on 15.02.20.
*/

#pragma once

struct http_request_s;
struct http_response_s;

int
serve(struct http_request_s *request, struct http_response_s *response);
