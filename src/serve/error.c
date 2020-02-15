/*
 * Created by victoria on 15.02.20.
*/

#include "string.h"
#include "../kv.h"
#include "../http.h"
#include "file.h"
#include "../config.h"

#include <stdio.h>

void
serve_error(struct http_response_s *response, int error, const char *detail)
{
	char errcfg[32];
	snprintf(errcfg, 32, "err.%d", error);

	response->code = error;

	if (kv_isset(global_config, errcfg) == TRUE)
	{
		if (serve_file(response, kv_string(global_config, errcfg, "error.html"), TRUE) ==
			SUCCESS)
		{
			return;
		}
	}

	serve_string(response, detail);
}