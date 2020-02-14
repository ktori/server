#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include "kv.h"

struct kv_list_s *
config_load(const char *filename);

#endif /* CONFIG_H */