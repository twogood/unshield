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
  /* The component record spans 0x76 bytes for major versions 0 and 5, and
     0x75 for every other version (the switch below skips 0x6c vs 0x6b). */
  const size_t record_size =
    (header->major_version == 0 || header->major_version == 5) ? 0x76 : 0x75;
  UnshieldComponent* self = NEW1(UnshieldComponent);
  uint8_t* p = unshield_header_get_buffer_checked(header, offset, record_size);
  uint32_t file_group_table_offset;
  unsigned i;

  if (!self)
    return NULL;

  if (!p)
  {
    unshield_error("Component descriptor (offset %08x) is out of bounds "
        "for header of %lu bytes", offset, (unsigned long)header->size);
    unshield_component_destroy(self);
    return NULL;
  }

  self->name = unshield_header_get_string(header, READ_UINT32(p)); p += 4;
  if (!self->name)
  {
    unshield_error("Component name (offset %08x) is out of bounds", offset);
    unshield_component_destroy(self);
    return NULL;
  }

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
  {
    unshield_error("File group count %u exceeds maximum %u",
        self->file_group_count, MAX_FILE_GROUP_COUNT);
    unshield_component_destroy(self);
    return NULL;
  }

  self->file_group_names = NEW(const char*, self->file_group_count);
  if (self->file_group_count && !self->file_group_names)
  {
    unshield_component_destroy(self);
    return NULL;
  }

  file_group_table_offset = READ_UINT32(p); p += 4;

  p = unshield_header_get_buffer_checked(header, file_group_table_offset,
      (size_t)self->file_group_count * 4);
  if (self->file_group_count && !p)
  {
    unshield_error("File group table (offset %08x) is out of bounds",
        file_group_table_offset);
    unshield_component_destroy(self);
    return NULL;
  }

  for (i = 0; i < self->file_group_count; i++)
  {
    self->file_group_names[i] = unshield_header_get_string(header, READ_UINT32(p)); 
    p += 4;
    if (!self->file_group_names[i])
    {
      unshield_error("File group name (component offset %08x) is out of bounds", offset);
      unshield_component_destroy(self);
      return NULL;
    }
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


