/*
 * Created by victoria on 15.02.20.
*/

#include "index.h"
#include "../http.h"
#include "error.h"
#include "../path.h"
#include "../server.h"

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

int
serve_index(struct http_response_s *response, const char *folder)
{
	struct dirent *dp;
	DIR *dir;
	char pathbuf[512];
	char *body;
	char *tmp_name;
	char *tmp_realname;
	int total, len;
	struct stat filestat;
	struct path_s *path;

	snprintf(pathbuf, 512, "%s/%s", documentroot, folder);

	dir = opendir(pathbuf);
	if (dir == NULL)
	{
		serve_error(response, 404, "Not Found");
		return EXIT_FAILURE;
	}
	total = 0;
	body = calloc(1, 1024 * 16);

	len =
			sprintf(body,
					"<html><head><title>Index of %s</title><meta charset="
					"\"UTF-8\"><base href=\"%s\"></head><body><h1>Index of %s</h1><hr>"
					"<table style=\"font-family:monospace\">",
					folder,
					folder,
					folder);
	total += len;

	dp = readdir(dir);

	while (dp != NULL)
	{
		path = path_make(folder);
		path_push(path, dp->d_name);
		tmp_name = path_to_string(path, "");
		tmp_realname = path_to_string(path, documentroot);
		printf("file: %s %s\n", tmp_realname, tmp_name);
		if (stat(tmp_realname, &filestat) == SUCCESS)
		{
			len = snprintf(body + total,
						   1024 * 16 - total,
						   "<tr><td><a href=\"%s\""
						   ">%s</a></td><td>",
						   tmp_name,
						   dp->d_name);
			total += len;
			if (!S_ISDIR(filestat.st_mode))
			{
				if (sizeof(off_t) == sizeof(long long))
				{
					len = snprintf(body + total,
								   1024 * 16 - total,
								   "%lld bytes",
								   (long long) filestat.st_size);
				}
				else
				{
					len = snprintf(body + total,
								   1024 * 16 - total,
								   "%ld bytes",
								   (long) filestat.st_size);
				}
			}
			else
			{
				len = snprintf(body + total, 1024 * 16 - total, "Directory");
			}
			total += len;
		}
		path_free(path);
		free(path);
		free(tmp_name);
		free(tmp_realname);
		dp = readdir(dir);
	}

	len = snprintf(body + total, 1024 * 16 - total, "</table></body></html>");

	response->body = body;
	response->length = total;

	closedir(dir);

	return EXIT_SUCCESS;
}
