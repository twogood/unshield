/* $Id$ */
#ifndef __unshield_internal_h__
#define __unshield_internal_h__

#include "unshield.h"
#include "cabfile.h"

#include <synce.h>

#include <stdio.h>  /* for FILE */

typedef struct _Header Header;

struct _Header
{
  Header*   next;
  int       index;
  uint8_t*  data;
  size_t    size;

  /* shortcuts */
  CommonHeader*   common;
  CabDescriptor*  cab;
  uint32_t*       file_table;
};

struct _Unshield
{
  Header* header_list;
  char* filename_pattern;
  int major_version;
};

/*
   Helpers
 */

FILE* unshield_fopen_for_reading(Unshield* unshield, int index, const char* suffix);
long unshield_fsize(FILE* file);

/*
   Constants
 */

#define HEADER_SUFFIX   "hdr"
#define CABINET_SUFFIX  "cab"

/*
   Macros for safer development
 */

#define FREE(ptr)       { if (ptr) { free(ptr); (ptr) = NULL; } }
#define STRDUP(str)     ((str) ? strdup(str) : NULL)
#define NEW1(type)      ((type*)calloc(1, sizeof(type)))
#define FCLOSE(file)    if (file) fclose(file)
#define FSIZE(file)     (file ? unshield_fsize(file) : 0)

#endif 

