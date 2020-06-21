/*
 * Created by victoria on 16.02.20.
*/
#pragma once

struct http_request_s;

/**
 * Expects the whole line to be available in the buffer, along with CR LF
 * @param buf buffer
 * @param size size of the buffer
 * @param request request to store output data in
 * @return 0 on success
 */
int
http_parse_request_line(const char *buf, int size, struct http_request_s *request);
