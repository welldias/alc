#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "alc_context.h"
#include "lib_func.h"

AlcContext *
alc_context_new()
{
  AlcContext *context;
  
  context = (AlcContext*)malloc(sizeof(AlcContext));
  if(context == NULL)
    return NULL;
  memset(context, 0, sizeof(AlcContext));
  return context;
}

void
alc_context_clear(AlcContext *context)
{
  if(context == NULL)
    return;
  alc_rules_clear(&context->rhead);
  alc_rules_clear(&context->rbody);
  alc_rules_clear(&context->rfoot);
  alc_var_list_clear(&context->vars);
  
  if(context->name != NULL)
    free(context->name);
  if(context->desc != NULL)
    free(context->desc);
  memset(context, 0, sizeof(AlcContext));
}

AlcRules *
alc_context_rule_get(AlcContext *context, const char* rule)
{
  if(context == NULL || rule == NULL)
    return NULL;
  if(MATCH(rule, "head"))
    return &context->rhead;
  else if(MATCH(rule, "body"))
    return &context->rbody;
  else if(MATCH(rule, "foot"))
    return &context->rfoot;
  else
    return NULL;
}

void
alc_context_vars_add(AlcContext *context, AlcVar* var)
{
  if(context == NULL || var == NULL)
    return;
  alc_var_list_add_var(&context->vars, var);
}

int
alc_context_name_set(AlcContext *context, const char* name)
{
  if(context == NULL || name == NULL)
    return -1;
  if(context->name != NULL)
    return -2;
  context->name  = _strdup(name);
  return 0;
}

int
alc_context_desc_set(AlcContext *context, const char* desc)
{
  if(context == NULL || desc == NULL)
    return -1;
  if(context->desc != NULL)
    return -2;
  context->desc  = _strdup(desc);
  return 0;
}
