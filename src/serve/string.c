/*
 * Created by victoria on 15.02.20.
*/

#include "vhttpsl/bits/kv.h"
#include "../lib/http.h"
#include "vhttpsl/http/response.h"
#include "../lib/config.h"

#include <string.h>
#include <stdlib.h>

struct server_config_s;

void
serve_string(struct server_config_s *config, struct http_response_s *response, const char *string)
{
	size_t len = strlen(string);
	kv_push(response->headers, "Content-Type", "text/plain");
	response->body = malloc(len);
	strncpy(response->body, string, len);
	response->length = len;
}
