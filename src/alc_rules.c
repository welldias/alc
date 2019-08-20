#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "alc_rules.h"

#define LINE_LEN 1024*10

static int   _alc_rules_match_rule(Regex *regex, char* line);
static char* _alc_rules_line_get(FILE *stream);

AlcRules *
alc_rules_new()
{
  AlcRules *rules;
  
  rules = (AlcRules*)malloc(sizeof(AlcRules));
  if(rules == NULL)
    return NULL;
  memset(rules, 0, sizeof(AlcRules));
  return rules;
}

void
alc_rules_clear(AlcRules *alc_rules)
{
  if(alc_rules == NULL)
    return;

  regex_list_clear(&alc_rules->rules);
  memset(alc_rules, 0, sizeof(AlcRules));
}

int
alc_rules_regex_add(AlcRules *alc_rules, Regex* regex)
{
  if(alc_rules == NULL || regex == NULL)
    return -1;
  return regex_list_add_regex(&alc_rules->rules, regex);
}

void  
alc_rules_logical_set(AlcRules *alc_rules, RuleLogic logical)
{
  if(alc_rules == NULL)
    return;
  alc_rules->logic = logical;
}

void  
alc_rules_type_set(AlcRules *alc_rules, RuleType type)
{
  if(alc_rules == NULL)
    return;
  alc_rules->type = type;
}

int
alc_rules_match_rules(AlcRules *alc_rules, LogFile *log_file, FILE *stream)
{
  Regex *regex;
  List *l;
  char *line;
  int count;

  count = 0;
  if(alc_rules == NULL || log_file == NULL || stream == NULL)
    return 0;
  if(alc_rules->rules.regexs == NULL 
    || list_count(alc_rules->rules.regexs) == 0)
    return 0;
  switch(alc_rules->logic)
  {
  case RULE_LOGIC_NOT:
    l=alc_rules->rules.regexs;
    regex = (Regex*)list_data(l);
    while(!feof(stream))
    {
      line = _alc_rules_line_get(stream);
      if(line == NULL)
        return 0;
      if(_alc_rules_match_rule(regex, line) != 0)
      {
        log_file->max_car_lines = max(log_file->max_car_lines, strlen(line));
        log_file_add_line(log_file, line);
        count++;
        l=list_next(l);
        if(l==NULL)
          return count;
        regex = (Regex*)list_data(l);
      }
      free(line);
    }
    return count;
  case RULE_LOGIC_OR:
    while(!feof(stream))
    {
      line = _alc_rules_line_get(stream);
      if(line == NULL)
        return 0;
      for(l=alc_rules->rules.regexs; l; l=list_next(l))
      {
        regex = (Regex*)list_data(l);
        if(_alc_rules_match_rule(regex, line) == 0)
        {
          log_file->max_car_lines = max(log_file->max_car_lines, strlen(line));
          log_file_add_line(log_file, line);
          count++;
          return count;
        }
      }
      free(line);
    }
    return count;
  case RULE_LOGIC_AND:
  default:
    l=alc_rules->rules.regexs;
    regex = (Regex*)list_data(l);
    while(!feof(stream))
    {
      line = _alc_rules_line_get(stream);
      if(line == NULL)
        return 0;
      if(_alc_rules_match_rule(regex, line) == 0)
      {
        log_file->max_car_lines = max(log_file->max_car_lines, strlen(line));
        log_file_add_line(log_file, line);
        count++;
        l=list_next(l);
        if(l==NULL)
          return count;
        regex = (Regex*)list_data(l);
      }
      free(line);
    }
    return count;
  }
  return count;
}

int 
_alc_rules_match_rule(Regex *regex, char* line)
{
  regmatch_t pmatch;
  int result;

  if(regex == NULL || line == NULL)
    return -1;
  if(regex->index >= 0)
  {
    if(regex->index >= (int)strlen(line))
      return -1;
    
    if(regex->use_regex)
    {
      if(regex->len > 0)
      {
        char *aux;
        
        result = -1;
        aux = (char*)malloc(regex->len)+1;
        if(aux != NULL)
        {
          strncpy(aux, line+regex->index, regex->len);
          result = regexec(&(regex->obj), aux, 1, &pmatch, 0);
          free(aux);
        }
        return result;
      }
      else
      {
        return regexec(&(regex->obj), line+regex->index, 1, &pmatch, 0);
      }
    }
    else
    {
      if(regex->len > 0)
        result = strncmp(line+regex->index, regex->value, regex->len);
      else
        result = strcmp(line+regex->index, regex->value);
      return result;
    }
  }
  result = regexec(&(regex->obj), line, 1, &pmatch, 0);
  return result;
}

char *
 _alc_rules_line_get(FILE *stream)
{
    char buf[LINE_LEN];
    char *aux;

    fgets(buf, LINE_LEN, stream);
    if((aux = strchr(buf, '\n')) != NULL)
    {
      *aux = '\0';
      if(*(aux-1) == '\r')
        *(aux-1) = '\0';
    }
    return strdup(buf);
}