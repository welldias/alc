#include "lib_func.h"
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <ctype.h>
#endif

int
lib_func_trim(char *str)
{
  char *p1, *p2;
  int i;

  if(str == NULL)
    return -1;
  p2 = str;
  while(isspace(*p2))
    if(*p2++ == 0)
      return -1;
  p1 = strdup(p2);
  if(p1 == NULL)
    return -1;
  for(i=strlen(p1); i>0; i--)
    if(!isspace(p1[i-1]))
      break;
  p1[i] = 0;
  strcpy(str, p1);
  free(p1);
  return 0;
}

int
lib_func_one_space(char *str)
{
  char *p1, *p2;
  unsigned int i, len;
  int just_one;

  if(str == NULL)
    return -1;
  len = strlen(str);
  p1  = (char*)malloc(len+1);
  if(p1 == NULL)
    return -1;
  just_one = 0;
  p2 = p1;
  for(i=0; i<len; i++)
  {
    if(isspace(str[i]))
    {
      if(just_one)
        continue;

      just_one = 1;
      *p2++ = ' ';
    }
    else
    {
      just_one = 0;
      *p2++ = str[i];
    }
  }
  *p2 = 0;
  strcpy(str, p1);
  free(p1);

  return 0;
}

int
lib_func_format_name_value(const char *str, char *name, char *value)
{
  char *n, *v;

  if(str == NULL || name == NULL || value == NULL)
    return -1;
  n = strdup(str);
  v = strchr(n, '=');
  if(v == NULL)
  {
    free(n);
    return -1;
  }
  *v++ = 0;
  lib_func_trim(n);
  lib_func_trim(v);
  strcpy(name, n);
  strcpy(value, v);
  free(n);

  return 0;
}

char*
lib_func_line_get(char *buff, int* n)
{
  char *p;

  if((p = strchr(buff, '\r')) == NULL)
    if((p = strchr(buff, '\n')) == NULL)
      return NULL;
  *p = '\0';
  if(*(p+1) == '\n')
    *(++p) = '\0';
  *n = p - buff;

  return buff;
}

char*
lib_func_line_get2(TextBuffer *text)
{
  char *a, *b;

  if(text == NULL || text->buf == NULL)
    return NULL;
  if(text->ptr == NULL)
    text->ptr =  text->buf;
  if(strlen(text->ptr) == 0)
    return NULL;
  a = text->ptr;
  if((b = strchr(text->ptr, '\r')) == NULL)
    if((b = strchr(text->ptr, '\n')) == NULL)
      return text->ptr;
  *b++ = '\0';
  if(*b == '\n')
    *b++ = '\0';
  text->ptr = b;

  return a;
}
