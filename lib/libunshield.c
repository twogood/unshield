/* $Id$ */
#define _BSD_SOURCE 1
#include "internal.h"
#include "log.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
  Create filename pattern used by unshield_fopen_for_reading()
 */
static bool unshield_create_filename_pattern(Unshield* unshield, const char* filename)/*{{{*/
{
  if (unshield && filename)
  {
    char pattern[256];
    char* prefix = strdup(filename);
    char* p = strrchr(prefix, '/');
    if (!p)
      p = prefix;

    for (; *p != '\0'; p++)
    {
      if ('.' == *p || isdigit(*p))
      {
        *p = '\0';
        break;
      }
    }

    snprintf(pattern, sizeof(pattern), "%s%%i.%%s", prefix);
    free(prefix);

    FREE(unshield->filename_pattern);
    unshield->filename_pattern = strdup(pattern);
    return true;
  }
  else
    return false;
}/*}}}*/

static bool unshield_get_common_header(Header* header)
{
  uint8_t* p = header->data;
  return unshield_read_common_header(&p, &header->common);
}

static bool unshield_get_cab_descriptor(Header* header)
{  
  if (header->common.cab_descriptor_size)
  {
    uint8_t* p = header->data + header->common.cab_descriptor_offset;

    p += 0xc;
    header->cab.file_table_offset   = READ_UINT32(p); p += 4;
    p += 4;
    header->cab.file_table_size     = READ_UINT32(p); p += 4;
    header->cab.file_table_size2    = READ_UINT32(p); p += 4;
    header->cab.directory_count     = READ_UINT32(p); p += 4;
    p += 8;
    header->cab.file_count          = READ_UINT32(p); p += 4;
    header->cab.file_table_offset2  = READ_UINT32(p); p += 4;

    assert((p - (header->data + header->common.cab_descriptor_offset)) == 0x30);

    if (header->cab.file_table_size != header->cab.file_table_size2)
      unshield_warning("File table sizes do not match");
          
    unshield_trace("Cabinet descriptor: %08x %08x %08x %08x",
        header->cab.file_table_offset,
        header->cab.file_table_size,
        header->cab.file_table_size2,
        header->cab.file_table_offset2
        );

    unshield_trace("Directory count: %i", header->cab.directory_count);
    unshield_trace("File count: %i", header->cab.file_count);
    
    return true;
  }
  else
  {
    unshield_error("No CAB descriptor available!");
    return false;
  }
}

static bool unshield_get_file_table(Header* header)
{
  uint8_t* p = header->data +
        header->common.cab_descriptor_offset +
        header->cab.file_table_offset;
  int count = header->cab.directory_count + header->cab.file_count;
  int i;
  
  header->file_table = calloc(count, sizeof(uint32_t));

  for (i = 0; i < count; i++)
  {
    header->file_table[i] = READ_UINT32(p); p += 4;
  }
  
  return true;
}  

/**
  Read all header files
 */
static bool unshield_read_headers(Unshield* unshield)/*{{{*/
{
  int i;
  bool iterate = true;
  Header* previous = NULL;

  if (unshield->header_list)
  {
    unshield_warning("Already have a header list");
    return true;
  }

  for (i = 1; iterate; i++)
  {
    FILE* file = unshield_fopen_for_reading(unshield, i, HEADER_SUFFIX);
    
    if (file)
    {
      unshield_trace("Reading header from .hdr file %i.", i);
      iterate = false;
    }
    else
    {
      unshield_trace("Could not open .hdr file %i. Reading header from .cab file %i instead.", 
          i, i);
      file = unshield_fopen_for_reading(unshield, i, CABINET_SUFFIX);
    }

    if (file)
    {
      size_t bytes_read;
      Header* header = NEW1(Header);
      header->index = i;

      header->size = FSIZE(file);
      if (header->size < 4)
      {
        unshield_error("Header file %i too small", i);
        goto error;
      }

      header->data = malloc(header->size);
      if (!header->data)
      {
        unshield_error("Failed to allocate memory for header file %i", i);
        goto error;
      }

      bytes_read = fread(header->data, 1, header->size, file);
      FCLOSE(file);

      if (bytes_read != header->size)
      {
        unshield_error("Failed to read from header file %i. Expected = %i, read = %i", 
            i, header->size, bytes_read);
        goto error;
      }

      if (!unshield_get_common_header(header))
      {
        unshield_error("Failed to read common header from header file %i", i);
        goto error;
      }
      
      unshield->major_version = (letoh32(header->common.version) >> 12) & 0xf;

      if (unshield->major_version < 5)
        unshield->major_version = 5;

      unshield_trace("Version 0x%08x handled as major version %i", 
          letoh32(header->common.version),
          unshield->major_version);

      if (!unshield_get_cab_descriptor(header))
      {
        unshield_error("Failed to read CAB descriptor from header file %i", i);
        goto error;
      }

      if (!unshield_get_file_table(header))
      {
        unshield_error("Failed to read file table from header file %i", i);
        goto error;
      }

      if (previous)
        previous->next = header;
      else
        previous = unshield->header_list = header;

      continue;

error:
      if (header)
        FREE(header->data);
      FREE(header);
      iterate = false;
    }
    else
      iterate = false;
  }

  return (unshield->header_list != NULL);
}/*}}}*/

Unshield* unshield_open(const char* filename)/*{{{*/
{
  Unshield* unshield = NEW1(Unshield);
  if (!unshield)
  {
    unshield_error("Failed to allocate memory for Unshield structure");
    goto error;
  }

  if (!unshield_create_filename_pattern(unshield, filename))
  {
    unshield_error("Failed to create filename pattern");
    goto error;
  }

  if (!unshield_read_headers(unshield))
  {
    unshield_error("Failed to read header files");
    goto error;
  }

  return unshield;

error:
  unshield_close(unshield);
  return NULL;
}/*}}}*/

void unshield_close(Unshield* unshield)/*{{{*/
{
  if (unshield)
  {
    Header* header;

    for(header = unshield->header_list; header; )
    {
      Header* next = header->next;

      FREE(header->data);
      FREE(header);

      header = next;
    }

    FREE(unshield->filename_pattern);
    free(unshield);
  }
}/*}}}*/


