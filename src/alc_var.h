#ifndef _ALC_ALC_VAR_H_
#define _ALC_ALC_VAR_H_

#include "list.h"

typedef struct _AlcVar     AlcVar;
typedef struct _AlcVarList AlcVarList;

struct _AlcVar
{
   char* name;
   char* value;
};

struct _AlcVarList
{
  List *vars;
};

AlcVar     *alc_var_new            ();
int         alc_var_set            (AlcVar *var, const char *name, const char *value);
void        alc_var_clear          (AlcVar *var);
char       *alc_var_name_get       (AlcVar *var);
char       *alc_var_value_get      (AlcVar *var);

AlcVarList *alc_var_list_new      ();
void        alc_var_list_init     (AlcVarList *var_list);
void        alc_var_list_clear    (AlcVarList *var_list);
int         alc_var_list_add_var  (AlcVarList *var_list, AlcVar *var);
AlcVar     *alc_var_list_find_var (AlcVarList *var_list, const char *name);

#endif /* _ALC_ALC_CONTEXT_H_ */
