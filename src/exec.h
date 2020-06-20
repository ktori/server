#pragma once

#include "vhttpsl/bits/kv.h"

char **
kv_to_env(struct kv_list_s *kv);

char **
kv_to_args(struct kv_list_s *kv);

int
pexec(const char *path,
	  const char **args,
	  const char **env,
	  const char *input,
	  int input_length,
	  char **output,
	  int *output_length);