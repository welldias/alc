#ifndef _ALC_REGEX_H_
#define _ALC_REGEX_H_

#include <regex.h>
#include "list.h"

typedef struct _Regex Regex;
typedef struct _RegexList RegexList;

struct _Regex
{
  unsigned int id;
  char* name;
  char* value;
  char use_regex;
  int index;
  int len;
  regex_t obj;
};

struct _RegexList
{
  unsigned int index;
  List *regexs;
};

Regex     *regex_new            ();
int        regex_init           (Regex *regex);
int        regex_set            (Regex *regex, char* name, char* value);
void       regex_indexcmp_set   (Regex *regex, unsigned int index, unsigned int len);
void       regex_clear          (Regex *regex);
char*      regex_name_get       (Regex *regex);
char*      regex_value_get      (Regex *regex);

RegexList *regex_list_new       ();
int        regex_list_load      (RegexList *regex_list, char *file_name);
void       regex_list_init      (RegexList *regex_list);
void       regex_list_clear     (RegexList *regex_list);
int        regex_list_add_regex (RegexList *regex_list, Regex *regex);

#endif /* _ALC_REGEX_H_ */
