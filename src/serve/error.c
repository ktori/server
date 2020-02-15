/*
 * Created by victoria on 15.02.20.
*/

#include "string.h"
#include "../lib/kv.h"
#include "../lib/http.h"
#include "file.h"
#include "../lib/config.h"

#include <stdio.h>
#include <stdlib.h>

void
serve_error(struct http_response_s *response, int error, const char *detail)
{
	char errcfg[32];
	snprintf(errcfg, 32, "err.%d", error);

	response->code = error;

	if (kv_isset(global_config, errcfg) == TRUE)
	{
		if (serve_file(response, kv_string(global_config, errcfg, "error.html"), TRUE) ==
			EXIT_SUCCESS)
		{
			return;
		}
	}

	serve_string(response, detail);
}