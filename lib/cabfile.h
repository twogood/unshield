/* $Id$ */
#ifndef __cabfile_h__
#define __cabfile_h__

#include <synce.h>

#define P __attribute__((packed))

#define OFFSET_COUNT 0x47
#define CAB_SIGNATURE 0x28635349

typedef struct
{
  uint32_t signature;
  uint32_t version;
  uint32_t volume_info;
  uint32_t cab_descriptor_offset;
  uint32_t cab_descriptor_size;
} CommonHeader;


typedef struct
{
  uint32_t data_offset;
  uint32_t unknown3;
  uint32_t first_file_index;
  uint32_t last_file_index;
  uint32_t first_file_offset;
  uint32_t first_file_size_expanded;
  uint32_t first_file_size_here;
  uint32_t last_file_offset;
  uint32_t last_file_size_expanded;
  uint32_t last_file_size_here;
} FiveHeader;

typedef struct
{
  uint64_t data_offset;
  uint32_t unknown3;
  uint32_t first_file_index;
  uint32_t last_file_index;
  uint64_t first_file_offset;
  uint64_t first_file_size_expanded;
  uint64_t first_file_size_here;
  uint64_t last_file_offset;
  uint64_t last_file_size_expanded;
  uint64_t last_file_size_here;
} SixHeader;

typedef struct
{
  P uint8_t stuff1[0xc];                    /* 0 */
  P uint32_t file_table_offset;             /* c */
  P uint32_t unknown1;                      /* 10 */
  P uint32_t file_table_size;               /* 14 */
  P uint32_t file_table_size2;              /* 18 */
  P uint32_t directory_count;               /* 1c */
  P uint32_t unknown3;                      /* 20 */
  P uint32_t unknown4;                      /* 24 */
  P uint32_t file_count;                    /* 28 */
  P uint32_t file_table_offset2;  /* 2c */
  P uint8_t  stuff2[0xe];                        /* 30 */
  P uint32_t file_group_offsets[OFFSET_COUNT];    /* 3e */
  P uint32_t component_offsets[OFFSET_COUNT];     /* 15a */
  /* 276 */
} CabDescriptor;

typedef struct
{
  uint32_t name_offset;
  uint32_t descriptor_offset;
  uint32_t next_offset;
} Entry;

typedef struct
{
  P uint32_t name_offset;
  P uint32_t directory_index;
  P uint16_t status;
  P uint32_t expanded_size;
  P uint32_t compressed_size;
  P uint32_t attributes;
  P uint32_t date;
  P uint32_t time;
  P uint32_t unknown[2];
  P uint32_t data_offset;
} FileDescriptor5;

typedef struct
{
  P uint16_t status;                /* 0 */
  P uint32_t expanded_size;         /* 2 */
  P uint32_t expanded_size_high;    /* 6 */
  P uint32_t compressed_size;       /* a */
  P uint32_t compressed_size_high;  /* e */
  P uint32_t data_offset;           /* 12 */
  P uint32_t data_offset_high;      /* 16 */
  P uint8_t md5[16];                /* 1a */
  P uint32_t version_ms;            /* 2a */
  P uint32_t version_ls;            /* 2e */
  P uint32_t unknown1;              /* 32 */
  P uint32_t unknown2;              /* 36 */
  P uint32_t name_offset;           /* 3a */
  P uint8_t unknown3[0x19];         /* 3e */
} FileDescriptor6;

#undef P

#endif

