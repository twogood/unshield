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

#if HAVE_BYTESWAP_H
#include <byteswap.h>
#elif HAVE_SYS_BYTESWAP_H
#include <sys/byteswap.h>
#else

/* use our own functions */
#define IMPLEMENT_BSWAP_XX 1
#define bswap_16 unshield_bswap_16
#define bswap_32 unshield_bswap_32

uint16_t bswap_16(uint16_t x);
uint32_t bswap_32(uint32_t x);
#endif

#define letoh16(x)    bswap_16(x)
#define letoh32(x)    bswap_32(x)

#else
#define letoh32(x) (x)
#define letoh16(x) (x)
#endif


#endif 

