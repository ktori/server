//
// Created by victoria on 21.06.2020.
//

#include <vhttpsl/http/request_line.h>
#include <vhttpsl/http/url.h>
#include <vhttpsl/http/request.h>
#include <assert.h>

const char TEST_CASE[] = "GET /test/index.html HTTP/1.1\r\n";

int
main()
{
	struct http_request_s request = {};
	int retval;

	retval = http_parse_request_line(TEST_CASE, sizeof(TEST_CASE), &request);

	assert(retval == 0);

	assert(request.version_major == 1);
	assert(request.version_minor == 1);
	assert(request.method == HTTP_METHOD_GET);
	assert(request.uri);

	assert(0 == strcmp(request.uri->complete, "/test/index.html"));

	return 0;
}
