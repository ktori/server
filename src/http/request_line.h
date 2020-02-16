/*
 * Created by victoria on 16.02.20.
*/
#pragma once

struct http_request_s;

int
request_line_read(struct http_request_s *request);
