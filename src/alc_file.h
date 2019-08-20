#ifndef _ALC_ALC_FILE_H_
#define _ALC_ALC_FILE_H_

#include "lib_func.h"
#include "alc_filter.h"

int alc_file_load           (AlcFilter *filter, const char *file_name);
int alc_file_read           (AlcFilter *filter, TextBuffer *text);

#endif /* _ALC_ALC_FILE_H_ */
