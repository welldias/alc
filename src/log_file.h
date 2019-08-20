#ifndef _ALC_LOG_FILE_H_
#define _ALC_LOG_FILE_H_

#include "list.h"

typedef struct _LogFile LogFile;

struct _LogFile
{
  List *lines;
  List *line_focus;
  unsigned long max_car_lines;
};

LogFile  *log_file_new         ();
void      log_file_clear       (LogFile *log_file);
int       log_file_add_line    (LogFile *log_file, char *line);
int       log_file_count_line  (LogFile *log_file);

#endif /* _ALC_LOG_FILE_H_ */
