/* $Id$ */
#ifndef __cabfile_h__
#define __cabfile_h__

#include "internal.h"

#define OFFSET_COUNT 0x47
#define CAB_SIGNATURE 0x28635349

#define MSCF_SIGNATURE 0x4643534d

#define COMMON_HEADER_SIZE      20
#define VOLUME_HEADER_SIZE_V5   40
#define VOLUME_HEADER_SIZE_V6   64

#define MAX_FILE_GROUP_COUNT    71
#define MAX_COMPONENT_COUNT     71

typedef struct
{
  uint32_t signature;               /* 00 */
  uint32_t version;
  uint32_t volume_info;
  uint32_t cab_descriptor_offset;
  uint32_t cab_descriptor_size;     /* 10 */
} CommonHeader;


typedef struct
{
  uint32_t data_offset;
  uint32_t data_offset_high;
  uint32_t first_file_index;
  uint32_t last_file_index;
  uint32_t first_file_offset;
  uint32_t first_file_offset_high;
  uint32_t first_file_size_expanded;
  uint32_t first_file_size_expanded_high;
  uint32_t first_file_size_compressed;
  uint32_t first_file_size_compressed_high;
  uint32_t last_file_offset;
  uint32_t last_file_offset_high;
  uint32_t last_file_size_expanded;
  uint32_t last_file_size_expanded_high;
  uint32_t last_file_size_compressed;
  uint32_t last_file_size_compressed_high;
} VolumeHeader;


typedef struct
{
  uint32_t file_table_offset;             /* c */
  uint32_t file_table_size;               /* 14 */
  uint32_t file_table_size2;              /* 18 */
  uint32_t directory_count;               /* 1c */
  uint32_t file_count;                    /* 28 */
  uint32_t file_table_offset2;  /* 2c */

  uint32_t file_group_offsets[MAX_FILE_GROUP_COUNT];  /* 0x3e  */
  uint32_t component_offsets [MAX_COMPONENT_COUNT];   /* 0x15a */
} CabDescriptor;

#define FILE_SPLIT			  1U
#define FILE_OBFUSCATED   2U
#define FILE_COMPRESSED		4U
#define FILE_INVALID		  8U

#define LINK_NONE	0
#define LINK_PREV	1
#define LINK_NEXT	2
#define LINK_BOTH	3

typedef struct
{
  uint32_t name_offset;
  uint32_t directory_index;
  uint16_t flags;
  uint64_t expanded_size;
  uint64_t compressed_size;
  uint64_t data_offset;
  uint8_t md5[16];
  uint16_t volume;
  uint32_t link_previous;
  uint32_t link_next;
  uint8_t link_flags;
} FileDescriptor;

typedef struct
{
  uint32_t name_offset;
  uint32_t descriptor_offset;
  uint32_t next_offset;
} OffsetList;

#endif

