#pragma once

#include <stddef.h> /* size_t */
#include <string.h>

#define TOUPPER(CH) (((CH) >= 'a' && (CH) <= 'z') ? ((CH) - 'a' + 'A') : (CH))

int
chtohex(int c);

int
hextoch(int h);

int
stricmp(const char *s1, const char *s2);

int
fmtlen(const char *fmt, ...);

char **
array_from_string(const char *string, char delim);

char **
empty_string_array(void);

void
string_array_free(char **array);

char *
strmake(const char *fmt, ...);

char *
substr(const char *str, int start, int length);

#define STREQ(A, B)                                                            \
  (((A) && (B)) &&                                                             \
   (((A) == (B)) || ((*(A) == *(B)) && (strcmp((A), (B)) == 0))))

#define STRIEQ(A, B)                                                           \
  (((A) && (B)) && (((A) == (B)) || (stricmp((A), (B)) == 0)))
