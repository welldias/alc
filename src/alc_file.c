#ifdef WIN32
#include <ctype.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "alc_file.h"
#include "lib_func.h"
#include "regex.h"


typedef enum _ParseResult
{
  PARSE_ERROR = -1,
  PARSE_EOF = 0,
  PARSE_COMMENT,
  PARSE_INCLUDE,
  PARSE_OPEN_BRACE,
  PARSE_CLOSE_BRACE,
  PARSE_GET,
  PARSE_SET,
  PARSE_CONTEXT,
  PARSE_INPUT,
  PARSE_LOGICAL,
  PARSE_HEAD,
  PARSE_BODY,
  PARSE_FOOT,
  PARSE_UNKNOW,
} ParseResult;


static int     _alc_file_process_context (AlcFilter *filter, TextBuffer *text);
static int     _alc_file_process_rules   (AlcFilter *filter, AlcContext *context, const char *rule, TextBuffer *text);
static char   *_alc_file_parse           (TextBuffer *text, ParseResult* parse_result);
static int     _alc_var_parse            (AlcFilter *filter, AlcContext *context, Regex *regex, char *value);

int
alc_file_load(AlcFilter *filter, const char *file_name)
{
  char *base_name;
  char *full_name;
  unsigned int size, i;
  TextBuffer text;
  FILE *alc_file;

  if(file_name == NULL || filter == NULL)
    return -1;
  /*  */
  base_name = (char*)malloc(strlen(file_name)+5);
  strcpy(base_name, file_name);
  for(
    i = strlen(base_name)-1; 
    i > 0 
      && base_name[i] != '.' 
      && base_name[i] != '/' 
      && base_name[i] != '\\'; 
    i--);
  if(base_name[i] == '.')
    base_name[i] = '\0';
  
  /*	*/
  full_name = (char*)malloc(strlen(base_name)+5);
  sprintf(full_name, "%s.alc", base_name);
  alc_file = fopen(full_name, "rb");
  if(alc_file == NULL)
  {
    sprintf(full_name, "%s.ALC", base_name);
    alc_file = fopen(full_name, "r");
  }

  free(base_name);
  free(full_name);

  if(alc_file == NULL)
    return -1;

  fseek(alc_file, 0, SEEK_END);
  size = ftell(alc_file);
  rewind(alc_file);
  
  text.buf = (char*)malloc(size+1);
  if(text.buf == NULL)
  {
    fclose(alc_file);
    return -1;
  }
  text.ptr = NULL;
  
  if(fread(text.buf, size, 1, alc_file) != 1)
  {
    free(text.buf);
    fclose(alc_file);
    return -1;
  }
  fclose(alc_file);
  text.buf[size] = 0;
  alc_file_read(filter, &text);
  free(text.buf);
  return 0;
}

int
alc_file_read(AlcFilter *filter, TextBuffer *text)
{
  ParseResult parse_result;
  char *line;

  if(text == NULL || filter == NULL)
    return -1;

  while((line = _alc_file_parse(text, &parse_result)) != NULL)
  {
    switch(parse_result)
    {
    case PARSE_COMMENT:
      {
        break;
      }
    case PARSE_SET:
      {
        AlcVar *var;
        char name[256+1];
        char *value;
        
        value = (char*)malloc(strlen(line)+1);
        if(lib_func_format_name_value(line+3, name, value) < 0)
        {
          free(value);
          free(line);
          return -1;
        }
        var = alc_var_new();
        alc_var_set(var, name, value);
        alc_filter_vars_add(filter, var);
        free(value);
        break;
      }
    case PARSE_INCLUDE:
      {
        char alc_include[256+1];
        
        lib_func_one_space(line);
        sscanf(line, "include %s", alc_include);
        if(strlen(alc_include) == 0)
        {
          free(line);
          return -1;
        }
        if(alc_file_load(filter, alc_include) < 0)
        {
          free(line);
          return -1;
        }
        break;
      }
    case PARSE_CONTEXT:
      {
        if(_alc_file_process_context(filter, text) < 0)
        {
          free(line);
          return -1;
        }
        break;
      }
    default:
      {
      }
    }
    free(line);
  }

  return 0;
}

int
_alc_file_process_context(AlcFilter *filter, TextBuffer *text)
{
  AlcContext  *context;
  ParseResult parse_result;
  int open_brace;
  char *line;

  if(text == NULL || filter == NULL)
    return -1;
  open_brace = 0;
  context = NULL;
  while((line = _alc_file_parse(text, &parse_result)) != NULL)
  {
    switch(parse_result)
    {
    case PARSE_COMMENT:
      {
        break;
      }
    case PARSE_OPEN_BRACE:
      {
        open_brace = 1;
        context = alc_context_new();
        if(context == NULL)
        {
          free(line);
          return -1;
        }
        break;
      }
    case PARSE_CLOSE_BRACE:
      {
        if(!open_brace)
        {
          free(line);
          return -3;
        }
        free(line);
        return 0;
      }
    case PARSE_INPUT:
      {
        break;
      }
    case PARSE_SET:
      {
        char name[256+1];
        char *value;
        AlcVar *var = NULL;
        
        if(!open_brace)
        {
          free(line);
          return -3;
        }
        value = (char*)malloc(strlen(line)+1);
        if(lib_func_format_name_value(line+3, name, value) < 0)
        {
          free(line);
          return -1;
        }
        if(MATCH(name, "Name"))
          alc_context_name_set(context, value);
        else if(MATCH(name, "Description"))
          alc_context_desc_set(context, value);
        else
        {
          var = alc_var_new();
          alc_var_set(var, name, value);
          alc_context_vars_add(context, var);
        }
        free(value);
        break;
      }
    case PARSE_HEAD:
    case PARSE_BODY:
    case PARSE_FOOT:
      {
        if(!open_brace)
        {
          free(line);
          return -3;
        }
        if(_alc_file_process_rules(filter, context, line, text) < 0)
        {
          free(line);
          return -1;
        }
        alc_filter_context_add(filter, context);
        break;
      }
    default:
      {
        break;
      }
    }
    free(line);
  }

  return (open_brace ? -3 : 0);
}

int
_alc_file_process_rules(AlcFilter *filter, AlcContext *context, const char *rule, TextBuffer *text)
{

 ParseResult parse_result;
  int open_brace;
  char *line;

  if(text == NULL || filter == NULL)
    return -1;
  open_brace = 0;
  while((line = _alc_file_parse(text, &parse_result)) != NULL)
  {
    switch(parse_result)
    {
    case PARSE_COMMENT:
    {
      break;
    }
    case PARSE_OPEN_BRACE:
    {
      open_brace = 1;
      break;
    }
    case PARSE_CLOSE_BRACE:
    {
      free(line);
      if(!open_brace)
        return -3;
      return 0;
    }
    case PARSE_GET:
    {
      AlcVar *var;
      Regex *regex;
      char name[256+1];
      
      if(!open_brace)
      {
        free(line);
        return -3;
      }      
      lib_func_one_space(line);
      sscanf(line, "get %s", name);
      if(strlen(name) == 0)
      {
        free(line);
        return -1;
      }
      var = alc_var_list_find_var(&filter->vars, name);
      if(var == NULL)
      {
        var = alc_var_list_find_var(&context->vars, name);
        if(var == NULL)
        {
          free(line);
          return -3;
        }
      }
      regex = regex_new();
      if(regex == NULL)
      {
        free(line);
        return -3;
      }
      if(_alc_var_parse(filter, context, regex, var->value) < 0)
      {
        free(regex);
        free(line);
        return -1;
      }
      free(regex->name);
      regex->name = strdup(name);
      if(MATCH(rule, "head"))
        alc_rules_regex_add(&context->rhead, regex);
      else if(MATCH(rule, "body"))
        alc_rules_regex_add(&context->rbody, regex);
      else if(MATCH(rule, "foot"))
        alc_rules_regex_add(&context->rfoot, regex);
      else
      {
        return -3;
      }
      break;
    }
    case PARSE_LOGICAL:
    {
      AlcRules *rules;
      if(strlen(line) <= 8)
      {
        return -3;
      }

      rules = alc_context_rule_get(context, rule);
      if(rules == NULL)
        return -3;
      if(MATCH(line+8, "OR"))
        alc_rules_logical_set(rules, RULE_LOGIC_OR);
      else if(MATCH(line+8, "NOT"))
        alc_rules_logical_set(rules, RULE_LOGIC_NOT);
      else if(MATCH(line+8, "AND"))
        alc_rules_logical_set(rules, RULE_LOGIC_AND);
      else
        return -3;
      break;
    }
    case PARSE_UNKNOW:
      {
        Regex *regex;
        
        if(!open_brace)
        {
          free(line);
          return -3;
        }
        regex = regex_new();
        regex_set(regex, "", line);
        if(MATCH(rule, "head"))
          alc_rules_regex_add(&context->rhead, regex);
        else if(MATCH(rule, "body"))
          alc_rules_regex_add(&context->rbody, regex);
        else if(MATCH(rule, "foot"))
          alc_rules_regex_add(&context->rfoot, regex);
        else
        {
          return -3;
        }
        break;
      }
     default:
       {
         break;
       }
    }
    free(line);
  }

  return (open_brace ? -3 : 0);
}

char *
_alc_file_parse(TextBuffer *text, ParseResult* parse_result)
{
  char *sentence;
  char *line;

  if(text == NULL || parse_result == NULL)
    return NULL;
  while((line = lib_func_line_get2(text)) != NULL)
  {
    if(strlen(line) == 0)
      continue;
    sentence = strdup(line);
    if(sentence == NULL)
    {
      *parse_result = PARSE_ERROR;
      return NULL;
    }
    lib_func_trim(sentence);
    /* ignorar comentarios */
    if(*sentence == '#')
    {
      *parse_result = PARSE_COMMENT;
      return sentence;
    }
    else if(MATCH(sentence, "include"))
    {
      *parse_result = PARSE_INCLUDE;
      return sentence;
    }
    else if(MATCH(sentence, "get"))
    {
      *parse_result = PARSE_GET;
      return sentence;
    }
    else if(MATCH(sentence, "set"))
    {
      *parse_result = PARSE_SET;
      return sentence;
    }
    else if(MATCH(sentence, "input"))
    {
      *parse_result = PARSE_INPUT;
      return sentence;
    }
    else if(MATCH(sentence, "logical"))
    {
      *parse_result = PARSE_LOGICAL;
      return sentence;
    }
    else if(*sentence == '{')
    {
      *parse_result = PARSE_OPEN_BRACE;
      return sentence;
    }
    else if(MATCH(sentence, "head"))
    {
      *parse_result = PARSE_HEAD;
      return sentence;
    }
    else if(MATCH(sentence, "body"))
    {
      *parse_result = PARSE_BODY;
      return sentence;
    }
    else if(MATCH(sentence, "foot"))
    {
      *parse_result = PARSE_FOOT;
      return sentence;
    }
    else if(MATCH(sentence, "context"))
    {
      *parse_result = PARSE_CONTEXT;
      return sentence;
    }
    else if(*sentence == '}')
    {
      *parse_result = PARSE_CLOSE_BRACE;
      return sentence;
    }
    else 
    {
      *parse_result = PARSE_UNKNOW;
      return sentence;
    }
  }

  *parse_result = PARSE_EOF;
  return NULL;
}

int 
_alc_var_parse(AlcFilter *filter, AlcContext *context, Regex *regex, char *value)
{
  char name[256+1];
  char *begin, *end, *aux;
  int len, idx;
  char *parsed = NULL;

  if(filter == NULL || regex == NULL || value == NULL)
    return -1;
  aux = begin = value;
  len = 1;
  idx = 0;
  while(*begin != 0)
  {
    if(*begin == '\\' && *(begin+1) == '[')
    {
      begin++;
    }
    else if(*begin == '[')
    {
      AlcVar *v;

      if((end = strchr(begin, ']')) == NULL)
        return -3;
      strncpy(name, begin+1, (end-begin)-1);
      name[(end-begin)-1] = 0;
      if(lib_func_trim(name) < 0)
      {
        return -3;
      }
      if(isdigit(*name))
      {
        char aux1[10], *aux2;
        
        strcpy(aux1, name);
        if((aux2 = strchr(aux1, ',')) == NULL)
        {
          regex_indexcmp_set(regex, atoi(aux1), -1);
        }
        else
        {
          *aux2++ = 0;
          regex_indexcmp_set(regex, atoi(aux1), atoi(aux2));
        }

        aux = begin = end;
        value = ++aux;
        continue;
      }
      
      v = alc_var_list_find_var(&filter->vars, name);
      if(v == NULL)
      {
        v = alc_var_list_find_var(&context->vars, name);
        if(v == NULL)
        {
          return -3;
        }
      }
      len += (begin-aux) + strlen(v->value);
      parsed = realloc(parsed, len);
      memcpy(parsed+idx, aux, (begin - aux));
      memcpy(parsed+(idx + begin - aux), v->value, strlen(v->value));
      parsed[len-1] = 0;
      idx = strlen(parsed);
      regex->use_regex = (char)1;
      
      aux = begin = end;
      aux++;
    }
    begin++;
  }

  if(idx > 0)
  {
    len += strlen(aux);
    parsed = realloc(parsed, len);
    memcpy(parsed+idx, aux, strlen(aux));
    parsed[len-1] = 0;
  }
  
  if(parsed == NULL)
  {
    regex_set(regex, "", value);
  }
  else
  {
    regex_set(regex, "", parsed);
    free(parsed);
  }

  return 0;
}