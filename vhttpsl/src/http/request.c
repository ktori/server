/*
 * Created by victoria on 16.02.20.
*/

#include "vhttpsl/http/url.h"
#include "vhttpsl/http/request.h"
#include "vhttpsl/bits/kv.h"

#include <string.h>
#include <stdlib.h>

void
http_request_free(struct http_request_s *request)
{
	if (request->uri != NULL)
	{
		uri_free(request->uri);
		free(request->uri);
	}
	kv_free(request->headers);
}

