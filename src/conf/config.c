/*
 * Created by victoria on 19.02.20.
*/

#include "config.h"

#include <stdlib.h>
#include <conf/config.h>

const char *
get_config_path()
{
	const char *env = getenv(CONFIG_ENV);

	if (env != NULL)
		return env;

	return CONFIG_DEFAULT;
}
