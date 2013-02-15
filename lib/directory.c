/* $Id$ */
#include "internal.h"
#include "log.h"

int unshield_directory_count(Unshield* unshield)
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    return header->cab.directory_count;
  }
  else
    return -1;
}

const char* unshield_directory_name(Unshield* unshield, int index)
{
  if (unshield && index >= 0)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    if (index < (int)header->cab.directory_count)
      return unshield_get_utf8_string(header, 
          header->data +
          header->common.cab_descriptor_offset +
          header->cab.file_table_offset +
          header->file_table[index]);
  }

  unshield_warning("Failed to get directory name %i", index);
  return NULL;
}

