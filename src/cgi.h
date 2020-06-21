#pragma once

#include "vhttpsl/bits/str.h"
#include "lib/http.h"
#include "vhttpsl/bits/kv.h"
#include "vhttpsl/bits/path.h"
#include "vhttpsl/http/request.h"

struct server_config_s;
struct http_request_s;
struct path_s;
struct kv_list_s;

int
cgi_prepare_environment(struct server_config_s *config,
						struct http_request_s *request,
						struct path_s *script,
						char **cmd,
						struct kv_list_s **env_list,
						struct kv_list_s **arg_list);

char *
cgi_toupper(const char *string);

bool
cgi_is_script(struct server_config_s *config, struct path_s *path);
