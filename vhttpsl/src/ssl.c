/*
 * Created by victoria on 11/30/20.
 */

#include "vhttpsl/ssl.h"

#include <openssl/ssl.h>

void
vhttpsl_init_openssl()
{
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
}

void
vhttpsl_cleanup_openssl()
{
	EVP_cleanup();
}
