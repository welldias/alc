#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log_file.h"
#include "lib_func.h"

static void _log_file_free_each(void *line);

LogFile *
log_file_new()
{
  LogFile * log_file;

  log_file = (LogFile*)malloc(sizeof(LogFile));
  if(log_file == NULL)
    return NULL;
  memset(log_file, 0, sizeof(LogFile));
  return log_file;
}

void
log_file_clear(LogFile *log_file)
{
  if(log_file == NULL)
    return;
  log_file->lines = list_free_foreach(log_file->lines, _log_file_free_each);
  memset(log_file, 0, sizeof(LogFile));
}

int
log_file_add_line(LogFile *log_file, char *line)
{
  List *list = NULL;

  if(log_file == NULL || line == NULL)
    return -1;
  log_file->lines = list_append(log_file->lines, line);
  if(list_alloc_error())
  {
    fprintf(stderr, "ERROR: Memory is low. List allocation failed.\n");
    return -1;
  }
  log_file->max_car_lines = max(strlen(line), log_file->max_car_lines);
  return list_count(log_file->lines);
}

int 
log_file_count_line(LogFile *log_file)
{
  if(log_file == NULL || log_file->lines == NULL)
    return -1;
  return list_count(log_file->lines);
}

void _log_file_free_each(void *line)
{
  if(line != NULL)
    free(line);
}