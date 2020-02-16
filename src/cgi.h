#pragma once

#include "lib/str.h"
#include "lib/http.h"
#include "lib/kv.h"
#include "lib/path.h"
#include "http/request.h"

int
cgi_prepare_environment(struct http_request_s *request,
						struct path_s *script,
						char **cmd,
						struct kv_list_s **env_list,
						struct kv_list_s **arg_list);

char *
cgi_toupper(const char *string);

bool
cgi_is_script(struct path_s *path);