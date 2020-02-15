#include "../common.h"
#include "config.h"
#include "kv.h"

#include <string.h>

struct kv_list_s *mimetypes;
bool mime_init_done = FALSE;

void
mimeinit(void)
{
	mimetypes = config_load("mime.conf");
	mime_init_done = TRUE;
}

const char *
mimetype(const char *filename)
{
	char *extension;

	extension = strrchr(filename, '.') + 1;
	if (extension == (char *) 1)
		return "text/plain";

	if (mime_init_done == FALSE)
	{
		mimeinit();
	}

	return kv_string(mimetypes, extension, "text/plain");
}