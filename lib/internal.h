/* $Id$ */
#ifndef __internal_h__
#define __internal_h__

#include "libunshield.h"
#include "unshield_config.h"

#if HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include <stdbool.h>
#include <stdio.h>  /* for FILE */

#include "cabfile.h"

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

#define FREE(ptr)       { if (ptr) { free(ptr); ptr = NULL; } }
#define STRDUP(str)     ((str) ? strdup(str) : NULL)
#define NEW1(type)      ((type*)calloc(1, sizeof(type)))
#define FCLOSE(file)    if (file) { fclose(file); file = NULL; }
#define FSIZE(file)     (file ? unshield_fsize(file) : 0)

#if WORDS_BIGENDIAN
#error "Big endian not yet supported"
#else
#define letoh32(n) (n)
#define letoh16(n) (n)
#endif


#endif 

