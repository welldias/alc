#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "regex.h"

static int _regex_list_sort(void *r1, void *r2);

Regex *
regex_new()
{
  Regex *regex;

  regex = (Regex*)malloc(sizeof(Regex));
  if(regex == NULL)
    return NULL;

  regex_init(regex);

  return regex;
}

int
regex_init(Regex *regex)
{
  if(regex == NULL)
    return -1;
  memset(regex, 0, sizeof(Regex));
  regex->index = -1;
  regex->len   = -1;
  return 0;
}

int
regex_set(Regex *regex, char* name, char* value)
{
  if(regex == NULL || name == NULL || value == NULL)
    return -1;

  regex->id = 0;
  regex->name  = strdup(name);
  regex->value = strdup(value);
  return regcomp(&regex->obj, regex->value, REG_EXTENDED);
}

void
regex_indexcmp_set(Regex *regex, unsigned int index, unsigned int len)
{
  if(regex == NULL)
    return;
  regex->index = index;
  regex->len   = len;
}

void
regex_clear(Regex *regex)
{
  if(regex == NULL)
    return;
  free(regex->name);
  free(regex->value);
  regfree(&(regex->obj));
}

char*
regex_name_get(Regex *regex)
{
  if(regex == NULL)
    return NULL;
  return regex->name;
}

char*
regex_value_get(Regex *regex)
{
  if(regex == NULL)
    return NULL;
  return regex->value;
}

RegexList *
regex_list_new()
{
  RegexList *regex_list;

  regex_list = (RegexList*)malloc(sizeof(RegexList));
  if(regex_list == NULL)
    return NULL;
  regex_list_init(regex_list);

  return regex_list;
}

void
regex_list_init(RegexList *regex_list)
{
  if(regex_list == NULL)
    return;
  memset(regex_list, 0, sizeof(RegexList));
}

int 
regex_list_load(RegexList *regex_list, char *file_name)
{
  if(regex_list == NULL)
    return -1;

  regex_list->regexs = list_sort(
    regex_list->regexs,
    list_count(regex_list->regexs),
    _regex_list_sort);

  if (list_alloc_error())
  {
    fprintf(stderr, "ERROR: Memory is low. List Sorting failed.\n");
    return -1;
  }

  return list_count(regex_list->regexs);
}

void
regex_list_clear(RegexList *regex_list)
{
  if(regex_list == NULL)
    return;
  regex_list->regexs = list_free_foreach(regex_list->regexs, regex_clear);
  memset(regex_list, 0, sizeof(RegexList));
}

int
regex_list_add_regex(RegexList *regex_list, Regex *regex)
{
  if(regex_list == NULL || regex == NULL)
    return -1;

  regex_list->regexs = list_append(regex_list->regexs, regex);
  if(list_alloc_error())
  {
    fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
    return -1;
  }

  return list_count(regex_list->regexs);
}

int
_regex_list_sort(void *r1, void *r2)
{
  if(r1 == NULL) 
    return(1);
  if(r2 == NULL) 
    return(-1);

  return(strcmp(((Regex*)r1)->name, ((Regex*)r2)->name));
}

