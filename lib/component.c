/* $Id$ */
#include "internal.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

int unshield_component_count(Unshield* unshield)
{
  Header* header = unshield->header_list;
  return header->component_count;
}

const char* unshield_component_name(Unshield* unshield, int index)
{
  Header* header = unshield->header_list;

  if (index >= 0 && index < header->component_count)
    return header->components[index]->name;
  else
    return NULL;
}

UnshieldComponent* unshield_component_new(Header* header, uint32_t offset)
{
  UnshieldComponent* self = NEW1(UnshieldComponent);
  uint8_t* p = unshield_header_get_buffer(header, offset);
  uint32_t file_group_table_offset;
  unsigned i;

  self->name = unshield_header_get_string(header, READ_UINT32(p)); p += 4;

  switch (header->major_version)
  {
    case 0:
    case 5:
      p += 0x6c;
      break;

    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    default:
      p += 0x6b;
      break;
  }

  self->file_group_count = READ_UINT16(p); p += 2;
  if (self->file_group_count > MAX_FILE_GROUP_COUNT)
    abort();

  self->file_group_names = NEW(const char*, self->file_group_count);

  file_group_table_offset = READ_UINT32(p); p += 4;

  p = unshield_header_get_buffer(header, file_group_table_offset);

  for (i = 0; i < self->file_group_count; i++)
  {
    self->file_group_names[i] = unshield_header_get_string(header, READ_UINT32(p)); 
    p += 4;
  }

  return self;
}

void unshield_component_destroy(UnshieldComponent* self)
{
  if (self)
  {
    FREE(self->file_group_names);
    free(self);
  }
}


