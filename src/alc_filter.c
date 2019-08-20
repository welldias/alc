#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "list.h"
#include "lib_func.h"
#include "alc_filter.h"

AlcFilter *
alc_filter_new()
{
  AlcFilter *filter;

  filter = (AlcFilter*)malloc(sizeof(AlcFilter));
  if(filter == NULL)
    return NULL;
  memset(filter, 0, sizeof(AlcFilter));
  alc_var_list_init(&filter->vars);
  return filter;
}

void
alc_filter_clear(AlcFilter *filter)
{
  if(filter == NULL)
    return;
  alc_var_list_clear(&filter->vars);
  filter->context = list_free_foreach(filter->context, alc_context_clear);
}

void
alc_filter_vars_add(AlcFilter *filter, AlcVar* var)
{
  if(filter == NULL || var == NULL)
    return;
  alc_var_list_add_var(&filter->vars, var);
}

void
alc_filter_context_add(AlcFilter *filter, AlcContext *context)
{
  if(filter == NULL)
    return;
  filter->context = list_append(filter->context, context);
}

AlcContext *
alc_filter_context_get(AlcFilter *filter, const char *context)
{
  List *l;
  AlcContext *c;
  for(l = filter->context; l; list_next(l))
  {
    c = (AlcContext*)l->data;
    if(MATCH(c->name, context))
      return c;
  }

  return NULL;
}

LogFile *
alc_filter_load_lines(AlcFilter *filter, const char *file_name)
{
  FILE *stream;
  LogFile *log_file;
  AlcContext *alc_context;
  AlcRules *alc_rules;

  if(filter == NULL || file_name == NULL)
    return NULL;
  alc_context = alc_filter_context_get(filter, "Transac");
  if(alc_context == NULL)
    return NULL;
  stream = fopen(file_name, "r");
  if(stream == NULL)
    return NULL;
  log_file = log_file_new();
  if(log_file == NULL)
  {
    fclose(stream);
    return NULL;
  }
  log_file->max_car_lines = 0;
  alc_rules = &alc_context->rhead;
  alc_rules_match_rules(alc_rules, log_file, stream);
  alc_rules = &alc_context->rbody;
  alc_rules_match_rules(alc_rules, log_file, stream);
  alc_rules = &alc_context->rfoot;
  alc_rules_match_rules(alc_rules, log_file, stream);

  log_file->line_focus = log_file->lines;
  fclose(stream);

  return log_file;
}
