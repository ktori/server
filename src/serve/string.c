/*
 * Created by victoria on 15.02.20.
*/

#include "../lib/kv.h"
#include "../lib/http.h"
#include "../http/response.h"

#include <string.h>
#include <stdlib.h>

void
serve_string(struct http_response_s *response, const char *string)
{
	size_t len = strlen(string);
	kv_push(response->headers, "Content-Type", "text/plain");
	response->body = malloc(len);
	strncpy(response->body, string, len);
	response->length = len;
}