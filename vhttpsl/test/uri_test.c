/*
 * Created by victoria on 21.06.2020.
 */

#include <vhttpsl/http/url.h>
#include <assert.h>

const char TEST_URI_ABSOLUTE[] = "https://localhost/example/1.html";
const char TEST_URI_RELATIVE[] = "/var/log/mail.log?t=1234";
const char TEST_URI_FULL[] = "https://user:password@example.com:8080/static/style.css?t=123&ac=ABC&ex=%23%20Encoded%20%25#frag";

void
test_uri_absolute();

void
test_uri_relative();

void
test_uri_full();

int
main()
{
	test_uri_absolute();
	test_uri_relative();
	test_uri_full();

	return 0;
}

void
test_uri_absolute()
{
	struct uri_s *uri = uri_make(TEST_URI_ABSOLUTE, sizeof(TEST_URI_ABSOLUTE));

	assert(0 == strcmp(uri->scheme, "https"));
	assert(0 == strcmp(uri->host, "localhost"));

	assert(1);
}

void
test_uri_relative()
{
	struct uri_s *uri = uri_make(TEST_URI_RELATIVE, sizeof(TEST_URI_RELATIVE));

	assert(1);
}

void
test_uri_full()
{
	struct uri_s *uri = uri_make(TEST_URI_FULL, sizeof(TEST_URI_FULL));

	assert(1);
}
