#include "str.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
chtohex(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	return 0;
}

const char *hex_table = "0123456789ABCDEF";

int
hextoch(int h)
{
	return (h >= 0 && h < 16) ? hex_table[(int) h] : '0';
}

int
stricmp(const char *s1, const char *s2)
{
	while (*s2 != 0 && TOUPPER(*s1) == TOUPPER(*s2))
		s1++, s2++;
	return (int) (TOUPPER(*s1) - TOUPPER(*s2));
}

int
fmtlen(const char *fmt, ...)
{
	size_t size;
	int current;
	char *buffer;

	va_list list;

	size = strlen(fmt);
	if (size < 100)
	{
		size = 100;
	}
	buffer = malloc(size);

	do
	{
		va_start(list, fmt);

		current = vsnprintf(buffer, size, fmt, list);

		if (current >= size)
		{
			size *= 2;
			buffer = realloc(buffer, size);
		}

		va_end(list);
	}
	while (current >= size);

	free(buffer);
	return current;
}

char **
array_from_string(const char *string, char delim)
{
	char **result;
	char *tmp;
	int count;
	int i, j, begin;

	begin = 0;
	j = 0;
	count = 4;
	result = calloc(count, sizeof(char *));

	for (i = 0; i < strlen(string) + 1; ++i)
	{
		if (string[i] == '\0' || string[i] == delim)
		{
			tmp = malloc(i - begin + 1);
			strncpy(tmp, string + begin, i - begin);
			tmp[i - begin] = '\0';
			result[j] = tmp;
			tmp = NULL;
			/* no free */
			begin = i + 1;
			j++;
			if (j >= count)
			{
				count *= 2;
				result = realloc(result, count * sizeof(char *));
			}
		}
	}
	result[j] = NULL;

	return result;
}

char **
empty_string_array(void)
{
	return calloc(1, sizeof(char **));
}

void
string_array_free(char **array)
{
	char *iter;
	int i;

	iter = array[0];
	i = 0;
	while (iter != NULL)
	{
		free(iter);
		iter = array[++i];
	}
	free(array);
}

char *
strmake(const char *fmt, ...)
{
	size_t size;
	int current;
	char *buffer;

	va_list list;

	size = strlen(fmt);
	if (size < 100)
	{
		size = 100;
	}
	buffer = malloc(size);

	do
	{
		va_start(list, fmt);

		current = vsnprintf(buffer, size, fmt, list);

		if (current >= size)
		{
			size *= 2;
			buffer = realloc(buffer, size);
		}

		va_end(list);
	}
	while (current >= size);

	return buffer;
}

char *
substr(const char *str, int start, int length)
{
	char *buffer;

	buffer = malloc(length + 1);
	strncpy(buffer, str + start, length);
	buffer[length] = '\0';

	return buffer;
}