/* $Id$ */
#ifndef __internal_h__
#define __internal_h__

#include "pendian_detect.h"
#include "libunshield.h"

#ifdef AUTOTOOLS
#include <config.h>
#else
#include "lib/unshield_config.h"
#endif

#if HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include <stdbool.h>
#include <stdio.h>  /* for FILE */

#include "cabfile.h"

typedef struct _StringBuffer StringBuffer;

struct _StringBuffer
{
  StringBuffer* next;
  char* string;
};

typedef struct _Header Header;

struct _Header
{
  Header*   next;
  int       index;
  uint8_t*  data;
  size_t    size;
  int       major_version;

  /* shortcuts */
  CommonHeader    common;
  CabDescriptor   cab;
  uint32_t*       file_table;
  FileDescriptor** file_descriptors;

  int component_count;
  UnshieldComponent** components;

  int file_group_count;
  UnshieldFileGroup** file_groups;

  StringBuffer* string_buffer;
};

struct _Unshield
{
  Header* header_list;
  char* filename_pattern;
};

/*
   Internal component functions
 */

UnshieldComponent* unshield_component_new(Header* header, uint32_t offset);
void unshield_component_destroy(UnshieldComponent* self);


/* 
   Internal file group functions
 */

UnshieldFileGroup* unshield_file_group_new(Header* header, uint32_t offset);
void unshield_file_group_destroy(UnshieldFileGroup* self);


/*
   Helpers
 */

char *unshield_get_base_directory_name(Unshield *unshield);
size_t unshield_get_path_max(Unshield* unshield);
FILE* unshield_fopen_for_reading(Unshield* unshield, int index, const char* suffix);
long long unshield_fsize(FILE* file);
bool unshield_read_common_header(uint8_t** buffer, CommonHeader* common);

const char* unshield_get_utf8_string(Header* header, const void* buffer);
const char* unshield_header_get_string(Header* header, uint32_t offset);
uint8_t* unshield_header_get_buffer(Header* header, uint32_t offset);


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
#define NEW(type, count)      ((type*)calloc(count, sizeof(type)))
#define NEW1(type)      ((type*)calloc(1, sizeof(type)))
#define FCLOSE(file)    if (file) { fclose(file); (file) = NULL; }
#define FSIZE(file)     ((file) ? unshield_fsize(file) : 0)
#define STREQ(s1,s2)    (0 == strcmp(s1,s2))

#if __BIG_ENDIAN__
/* byteswap.h */
static inline uint16_t bswap_16(uint16_t x)
{
	return ((x << 8) & 0xff00) | ((x >> 8) & 0x00ff);
}
static inline uint32_t bswap_32(uint32_t x)
{
	return	((x << 24) & 0xff000000 ) |
		((x <<  8) & 0x00ff0000 ) |
		((x >>  8) & 0x0000ff00 ) |
		((x >> 24) & 0x000000ff );
}
#define letoh16(x)    bswap_16(x)
#define letoh32(x)    bswap_32(x)

#else // not big endian
#define letoh32(x) (x)
#define letoh16(x) (x)
#endif /* __BIG_ENDIAN__ */

static inline uint16_t get_unaligned_le16(const uint8_t *p)
{
    return p[0] | p[1] << 8;
}

static inline uint32_t get_unaligned_le32(const uint8_t *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

static inline uint64_t get_unaligned_le64(const uint8_t *p)
{
    return (uint64_t)get_unaligned_le32(p + 4) << 32 | get_unaligned_le32(p);
}

#define READ_UINT16(p)   get_unaligned_le16(p)
#define READ_UINT32(p)   get_unaligned_le32(p)
#define READ_UINT64(p)   get_unaligned_le64(p)

#define READ_INT16(p)   ((int16_t)READ_UINT16(p))
#define READ_INT32(p)   ((int32_t)READ_UINT32(p))


#endif 

