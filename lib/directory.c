/* $Id$ */
#include "internal.h"

int unshield_directory_count(Unshield* unshield)
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    return header->cab->directory_count;
  }
  else
    return -1;
}

const char* unshield_directory_name(Unshield* unshield, int index)
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    return (const char*)((uint8_t*)header->file_table +
        header->file_table[index]);
  }
  else
    return NULL;
}

