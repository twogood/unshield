/* $Id$ */
#define _BSD_SOURCE 1
#include "unshield_internal.h"

FILE* unshield_fopen_for_reading(Unshield* unshield, int index, const char* suffix)
{
  if (unshield && unshield->filename_pattern)
  {
    char filename[256];
    snprintf(filename, sizeof(filename), unshield->filename_pattern, index, suffix);
    return fopen(filename, "r");
  }
  else
    return NULL;
}

long unshield_fsize(FILE* file)
{
  long result;
  long previous = ftell(file);
  fseek(file, 0L, SEEK_END);
  result = ftell(file);
  fseek(file, previous, SEEK_SET);
  return result;
}

