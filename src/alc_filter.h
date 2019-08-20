#ifndef _ALC_ALC_FILTER_H_
#define _ALC_ALC_FILTER_H_

#include "alc_var.h"
#include "alc_context.h"
#include "log_file.h"

typedef struct _AlcFilter AlcFilter;

struct _AlcFilter
{
  AlcVarList vars;
  List *context;
};

AlcFilter  *alc_filter_new         ();
void        alc_filter_clear       (AlcFilter *filter);
void        alc_filter_vars_add    (AlcFilter *filter, AlcVar* var);
void        alc_filter_context_add (AlcFilter *filter, AlcContext *context);
AlcContext *alc_filter_context_get (AlcFilter *filter, const char *context);
LogFile     *alc_filter_load_lines (AlcFilter *filter, const char *file_name);

#endif /* _ALC_ALC_FILTER_H_ */
