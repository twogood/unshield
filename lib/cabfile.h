/* $Id$ */
#ifndef __cabfile_h__
#define __cabfile_h__

#include <synce.h>

#define P __attribute__((packed))

#define OFFSET_COUNT 0x47
#define CAB_SIGNATURE 0x28635349

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
  uint32_t unknown3;
  uint32_t first_file_index;
  uint32_t last_file_index;
  uint32_t first_file_offset;
  uint32_t first_file_size_expanded;
  uint32_t first_file_size_compressed;
  uint32_t last_file_offset;
  uint32_t last_file_size_expanded;
  uint32_t last_file_size_compressed;
} VolumeHeader5;

typedef struct
{
  uint32_t data_offset;                     /* 14 */
  uint32_t data_offset_high;                /* 18 */
  uint32_t first_file_index;                /* 1c */
  uint32_t last_file_index;                 /* 20 */
  uint32_t first_file_offset;               /* 24 */
  uint32_t first_file_offset_high;
  uint32_t first_file_size_expanded;        /* 2c */
  uint32_t first_file_size_expanded_high;
  uint32_t first_file_size_compressed;      /* 34 */
  uint32_t first_file_size_compressed_high;
  uint32_t last_file_offset;                /* 3c */
  uint32_t last_file_offset_high;
  uint32_t last_file_size_expanded;         /* 44 */
  uint32_t last_file_size_expanded_high;
  uint32_t last_file_size_compressed;       /* 4c */
  uint32_t last_file_size_compressed_high;
} VolumeHeader6;

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
  P uint8_t md5[16];
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
  P uint16_t directory_index;       /* 3e */
  P uint8_t unknown3[0xc];          /* 40 */
  P uint32_t previous_copy;         /* 4c */
  P uint32_t next_copy;             /* 50 */
  P uint8_t link_flags;             /* 54 */
  P uint16_t volume;                /* 55 */
} FileDescriptor6;

#define FILE_SPLIT			1L
#define FILE_ENCRYPTED		2L
#define FILE_COMPRESSED		4L
#define FILE_INVALID		8L

#undef P

#endif

