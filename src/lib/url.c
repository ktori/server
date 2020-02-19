#include "url.h"

#include <assert.h>
#include <stdlib.h>

bool url_encode_table_whitelist[256];
bool url_encode_table_init = FALSE;

const char *rfc3986 =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

void
url_init(void)
{
	int i;

	memset(url_encode_table_whitelist, FALSE, sizeof(url_encode_table_whitelist));
	for (i = 0; i < strlen(rfc3986); ++i)
	{
		url_encode_table_whitelist[(int) rfc3986[i]] = TRUE;
	}
	url_encode_table_init = TRUE;
}

char *
url_encode(const char *in)
{
	int len = strlen(in);
	char *result = calloc(len * 3 + 1, 1);
	int i, j = 0;

	if (url_encode_table_init == FALSE)
	{
		url_init();
	}

	for (i = 0; i < len; ++i)
	{
		if (url_encode_table_whitelist[(int) in[i]] == FALSE)
		{
			result[j++] = '%';
			result[j++] = hextoch((in[i] & 0xF0) >> 4);
			result[j++] = hextoch(in[i] & 0x0F);
		}
		else
		{
			result[j++] = in[i];
		}
	}

	return result;
}

char *
url_decode(const char *in)
{
	int len = strlen(in);
	char *result = calloc(len + 1, 1);
	int i, j = 0;
	char byte = 0;

	for (i = 0; i < len; ++i)
	{
		if (in[i] == '%' && i < len - 2)
		{
			byte = (chtohex(in[i + 1]) << 4 | chtohex(in[i + 2]));
			i += 2;
			result[j++] = byte;
		}
		else
		{
			result[j++] = in[i];
		}
	}

	return result;
}

struct uri_s *
uri_make(const char *uri, size_t length)
{
	enum read_state
	{
		SCHEME = 0,
		AUTHORITY,
		PATH,
		QUERY,
		FRAGMENT,
		DONE
	};

	struct uri_s *result;
	enum read_state state;
	bool userinfo_read = FALSE;
	bool reading_port = FALSE;
	int i;
	int begin = 0;

	state = SCHEME;
	result = calloc(1, sizeof(struct uri_s));
	result->complete = malloc(length + 1);
	strncpy(result->complete, uri, length);
	result->complete[length] = 0;

	if (*uri == '/')
	{
		result->scheme = NULL;
		result->userinfo = NULL;
		result->host = NULL;
		result->port = NULL;
		state = PATH;
		begin = 0;
	}
	for (i = 0; i < length; ++i)
	{
		if (state == SCHEME)
		{
			if (uri[i] == ':')
			{
				result->scheme = malloc(i + 1);
				strncpy(result->scheme, uri, i);
				result->scheme[i] = '\0';
				state = AUTHORITY;
				begin = i + 3; /* :// */
				i += 2;
				continue;
			}
			else if (uri[i] == '/')
			{
				state = AUTHORITY;
				i = -1;
				begin = i;
				continue;
			}
		}
		else if (state == AUTHORITY)
		{
			if ((uri[i] == '@') && (userinfo_read == FALSE))
			{
				result->userinfo = malloc(i - begin + 1);
				strncpy(result->userinfo, uri + begin, i - begin);
				result->userinfo[i - begin] = '\0';
				begin = i + 1;
				userinfo_read = TRUE;
				continue;
			}
			if (uri[i] == ':' || uri[i] == '/' || uri[i] == '\0')
			{
				if (uri[i] == '/' && reading_port == TRUE)
				{
					result->port = malloc(i - begin + 1);
					strncpy(result->port, uri + begin, i - begin);
					result->port[i - begin] = '\0';
					begin = i;
					state = PATH;
				}
				if (uri[i] == ':')
				{
					reading_port = TRUE;
				}
				if (result->host == NULL)
				{
					result->host = malloc(i - begin + 1);
					strncpy(result->host, uri + begin, i - begin);
					result->host[i - begin] = '\0';
					begin = i + 1;
					if (reading_port == FALSE)
					{
						begin--;
						state = PATH;
					}
				}
			}
		}
		else if (state == PATH)
		{
			if ((uri[i] == '?') || (uri[i] == '#') || (uri[i] == '\0') || i == length - 1)
			{
				size_t path_size = i == length - 1 ? i - begin + 1 : i - begin;
				result->spath = malloc(path_size + 1);
				strncpy(result->spath, uri + begin, path_size);
				result->spath[path_size] = '\0';
				result->path = path_make(result->spath);
				begin = i + 1;
				if (uri[i] == '?')
				{
					state = QUERY;
				}
				else if (uri[i] == '#')
				{
					state = FRAGMENT;
				}
				else
				{
					state = DONE;
				}
			}
		}
		else if (state == QUERY)
		{
			if ((uri[i] == '#') || (uri[i] == '\0'))
			{
				result->querystring = malloc(i - begin + 1);
				strncpy(result->querystring, uri + begin, i - begin);
				result->querystring[i - begin] = '\0';
				result->query = querystring_make(result->querystring);
				begin = i + 1;
				if (uri[i] == '#')
				{
					state = FRAGMENT;
				}
				else
				{
					state = DONE;
				}
			}
		}
		else if (state == FRAGMENT)
		{
			if (uri[i] == '\0')
			{
				result->fragment = malloc(i - begin + 1);
				strncpy(result->fragment, uri + begin, i - begin);
				result->fragment[i - begin] = '\0';
			}
		}
	}

	if (result->query == NULL)
	{
		result->query = querystring_make("");
	}
	if (result->path == NULL)
	{
		result->path = path_make("");
	}

	return result;
}

void
uri_free(struct uri_s *uri)
{
	free(uri->complete);

	free(uri->scheme);
	free(uri->userinfo);
	free(uri->host);
	free(uri->port);
	free(uri->spath);
	free(uri->querystring);
	free(uri->fragment);

	if (uri->query != NULL)
		kv_free(uri->query);
	if (uri->path != NULL)
	{
		path_free(uri->path);
		free(uri->path);
	}
}

struct kv_list_s *
querystring_make(const char *qs)
{
	struct kv_list_s *result = kv_create();
	char *key = NULL, *value = NULL;
	int i, key_begin = 0, value_begin = -1;

	for (i = 0; i <= strlen(qs); ++i)
	{
		if ((qs[i] == '&') || (qs[i] == '\0'))
		{
			if (key == NULL)
			{
				assert(value_begin < 0);

				key = malloc(i - key_begin + 1);
				strncpy(key, qs + key_begin, i - key_begin);
				key[i - key_begin] = '\0';

				value = malloc(1);
				*value = '\0';
			}
			else
			{
				assert(value_begin > 0);
				assert(value == NULL);

				value = malloc(i - value_begin + 1);
				strncpy(value, qs + value_begin, i - value_begin);
				value[i - value_begin] = '\0';
			}

			kv_push(result, key, value);

			key_begin = i + 1;
			value_begin = -1;
			free(key);
			free(value);
			key = NULL;
			value = NULL;
		}
		else if ((qs[i] == '=') && (key == NULL))
		{
			key = malloc(i - key_begin + 1);
			strncpy(key, qs + key_begin, i - key_begin);
			key[i - key_begin] = '\0';
			value_begin = i + 1;
		}
	}

	return result;
}