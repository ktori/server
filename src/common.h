#ifndef COMMON_H
#define COMMON_H

#include <stddef.h> /* size_t */
#include <string.h>

#ifndef NULL
#define NULL (0)
#endif

#define SUCCESS (0)
#define FAILURE (-1)
#define SKIPPED (-2)

#define FALSE (0)
#define TRUE (1)

#define TOUPPER(CH) (((CH) >= 'a' && (CH) <= 'z') ? ((CH) - 'a' + 'A') : (CH))

#define NUMFMT(type) (sizeof(type) == sizeof(long))

char
chtohex(char c);

char
hextoch(char h);

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

typedef char bool;
typedef int status_t;

#endif /* COMMON_H */