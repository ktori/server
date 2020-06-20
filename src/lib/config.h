#pragma once

#include "vhttpsl/bits/kv.h"

struct cgi_config_entry_s
{
	const char *extension;
	const char *command;
	int loose;
	int exec;
};

struct error_config_entry_s
{
	int code;
	const char *document;
};

struct server_config_s
{
	int port;
	const char *index;
	const char *root;
	const char *cgi_bin;
	/* error handlers */
	struct error_config_entry_s *errors;
	size_t errors_count;
	size_t errors_size;
	/* cgi config */
	struct cgi_config_entry_s *cgi;
	size_t cgi_count;
	size_t cgi_size;
	/* ssl */
	int ssl;
	const char *ssl_cert;
	const char *ssl_key;
};

const char *
config_loc();

int
config_load(struct server_config_s *config, const char *filename);

int
config_destroy(struct server_config_s *config);
