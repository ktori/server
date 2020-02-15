#pragma once

#include "common.h"
#include "kv.h"
#include "path.h"

void
url_init(void);

char *
url_encode(const char *in);

char *
url_decode(const char *in);

struct uri_s
{
	char *complete;

	char *scheme;
	char *userinfo;
	char *host;
	char *port;
	char *spath;
	char *querystring;
	char *fragment;

	struct kv_list_s *query;
	struct path_s *path;
};

struct uri_s *
uri_make(const char *uri);

void
uri_free(struct uri_s *uri);

struct kv_list_s *
querystring_make(const char *qs);