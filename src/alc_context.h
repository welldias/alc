#ifndef _ALC_ALC_CONTEXT_H_
#define _ALC_ALC_CONTEXT_H_

#include "alc_var.h"
#include "alc_rules.h"
#include "regex.h"

typedef struct _AlcContext    AlcContext;

struct _AlcContext
{
  char *name;
  char *desc;
  AlcVarList vars;
  AlcRules rhead;
  AlcRules rbody;
  AlcRules rfoot;
  unsigned int rule_index;
};

AlcContext  *alc_context_new         ();
void         alc_context_clear       (AlcContext *context);
AlcRules    *alc_context_rule_get    (AlcContext *context, const char* rule);
void         alc_context_vars_add    (AlcContext *context, AlcVar* var);
int          alc_context_name_set    (AlcContext *context, const char* name);
int          alc_context_desc_set    (AlcContext *context, const char* desc);

#endif /* _ALC_ALC_CONTEXT_H_ */
