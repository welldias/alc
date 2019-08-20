#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "list.h"
#include "alc_var.h"

static int _alc_var_find_data(const char* name, const AlcVar *var);

AlcVar*
alc_var_new()
{
  AlcVar *var;
  var = (AlcVar*)malloc(sizeof(AlcVar));
  if(var == NULL)
    return NULL;
  memset(var, 0, sizeof(AlcVar));
  return var;
}

int
alc_var_set(AlcVar *var, const char *name, const char *value)
{
  if(var == NULL || name == NULL || value == NULL)
    return -1;
  if(var->name != NULL || var->value != NULL)
    return -2;

  var->name  = strdup(name);
  var->value = strdup(value);
  return 0;
}

void
alc_var_clear(AlcVar *var)
{
  if(var == NULL)
    return;
  free(var->name);
  free(var->value);
}

char*
alc_var_name_get(AlcVar *var)
{
  if(var == NULL)
    return NULL;
  return var->name;
}

char*
alc_var_value_get(AlcVar *var)
{
  if(var == NULL)
    return NULL;
  return var->value;
}

int
_alc_var_find_data(const char* name, const AlcVar *var)
{
  if(strcmp(name, var->name)==0)
    return 1;
  return 0;
}


AlcVarList *
alc_var_llist_new()
{
  AlcVarList *alc_var_list;
  
  alc_var_list = (AlcVarList*)malloc(sizeof(AlcVarList));
  if(alc_var_list == NULL)
    return NULL;
  alc_var_list_init(alc_var_list);
  
  return alc_var_list;
}

void 
alc_var_list_init(AlcVarList *alc_var_list)
{
  if(alc_var_list == NULL)
    return;
  memset(alc_var_list, 0, sizeof(AlcVarList));
}

void 
alc_var_list_clear(AlcVarList *alc_var_list)
{
  if(alc_var_list == NULL)
    return;
  alc_var_list->vars = list_free_foreach(alc_var_list->vars, alc_var_clear);
  memset(alc_var_list, 0, sizeof(AlcVarList));
}

int 
alc_var_list_add_var(AlcVarList *alc_var_list, AlcVar *var)
{
  if(alc_var_list == NULL || var == NULL)
    return -1;
  
  alc_var_list->vars = list_append(alc_var_list->vars, var);
  if(list_alloc_error())
  {
    fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
    return -1;
  }

  return list_count(alc_var_list->vars);
}

AlcVar *
alc_var_list_find_var(AlcVarList *var_list, const char *name)
{
  List* l;

  if(var_list == NULL || name == NULL)
    return NULL;
  l = list_find_data(var_list->vars, name, _alc_var_find_data);
  if(l == NULL)
    return NULL;
  return list_data(l);
}