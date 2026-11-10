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

    if (header->file_table && index < (int)header->cab.directory_count)
    {
      uint64_t offset = (uint64_t)header->cab.file_table_offset +
                        (uint64_t)header->file_table[index];

      if (offset <= 0xffffffffu)
      {
        const char* name = unshield_header_get_string(header, (uint32_t)offset);

        if (name)
          return name;
      }
    }
  }

  unshield_warning("Failed to get directory name %i", index);
  return NULL;
}

