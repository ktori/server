//
// Created by victoria on 21.06.2020.
//

#include "session.h"

#include <stdlib.h>
#include <memory.h>

int
http_session_init(http_session_t session)
{
	memset(session, 0, sizeof(*session));

	return EXIT_SUCCESS;
}

void
http_session_destroy(http_session_t session)
{

}

int
http_session_read(http_session_t session, char *buf, size_t size)
{
	return 0;
}

int
http_session_write(http_session_t session, const char *buf, size_t size)
{
	return 0;
}
