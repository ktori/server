#pragma once

#include "kv.h"

struct kv_list_s *
config_load(const char *filename);

struct kv_list_s *global_config;