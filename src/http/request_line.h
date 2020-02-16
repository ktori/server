/*
 * Created by victoria on 16.02.20.
*/
#pragma once

enum http_status;
struct http_request_s;

int
request_line_read(struct http_request_s *request, enum http_status *out_status);
