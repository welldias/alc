#ifndef _ALC_LIB_FUNC_H_
#define _ALC_LIB_FUNC_H_

#include <stdio.h>
#include <string.h>

#define MATCH(a, b) (!strncmp((a), (b), strlen(b)))

typedef struct _TextBuffer TextBuffer;

struct _TextBuffer
{
  char *buf;
  char *ptr;
  unsigned int size;
};

int       lib_func_trim(char *str);
int       lib_func_one_space(char *str);
int       lib_func_format_name_value(const char *str, char *name, char *value);
char     *lib_func_line_get(char *buff, int* n);
char     *lib_func_line_get2(TextBuffer *text);
FILE     *lib_func_open_file(const char *file_name, const char* mode);

#endif /* _ALC_LIB_FUNC_H_ */
