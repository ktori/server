/*
 * Created by victoria on 15.02.20.
*/

#include "string.h"
#include "vhttpsl/bits/kv.h"
#include "../lib/http.h"
#include "file.h"
#include "../lib/config.h"
#include "../http/response.h"

#include <stdio.h>
#include <stdlib.h>

void
serve_error(struct server_config_s *config, struct http_response_s *response, int error, const char *detail)
{
	size_t i;
	struct error_config_entry_s *error_config = NULL;

	response->status = error;

	for (i = 0; i < config->errors_count; ++i)
		if (config->errors[i].code == error)
			error_config = config->errors + i;

	if (error_config != NULL)
	{
		if (serve_file(config, response, error_config->document, TRUE) ==
			EXIT_SUCCESS)
		{
			return;
		}
	}

	serve_string(config, response, detail);
}
