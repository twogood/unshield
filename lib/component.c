/* $Id$ */
#include "internal.h"
#include "log.h"
#include <string.h>

typedef bool (*ComponentCallback)
  (Header* header, uint32_t name_offset, uint32_t descriptor_offset, void* cookie);


static bool unshield_foreach_component(
    Header* header, ComponentCallback callback, void* cookie)
{
  int i;
  for (i = 0; i < MAX_COMPONENT_COUNT; i++)
  {
    if (header->cab.component_offsets[i])
    {
      OffsetList list;

      /*unshield_trace("Component offset %08x", header->cab.component_offsets[i]);*/
      list.next_offset = header->cab.component_offsets[i];

      while (list.next_offset)
      {
        uint8_t* p =
          header->data + 
          header->common.cab_descriptor_offset +
          list.next_offset;

        list.name_offset       = READ_UINT32(p); p += 4;
        list.descriptor_offset = READ_UINT32(p); p += 4;
        list.next_offset       = READ_UINT32(p); p += 4;

        if (!callback(header, list.name_offset, list.descriptor_offset, cookie))
          return true;
      }
    }
  }

  return true;
}

static bool unshield_component_counter_callback(
    Header* header, uint32_t name_offset, uint32_t descriptor_offset, void* cookie)
{
  int* counter = (int*)cookie;
  (*counter)++;
  return true;
}


int unshield_component_count(Unshield* unshield)
{
  Header* header = unshield->header_list;
  int counter = 0;
  
  unshield_foreach_component(header, unshield_component_counter_callback, &counter);

  return counter;
}

typedef struct
{
  int current_index;
  int wanted_index;
  bool component_found;
  uint32_t name_offset;
  uint32_t descriptor_offset;
} FindComponentData;

static bool unshield_find_component_callback(
    Header* header, uint32_t name_offset, uint32_t descriptor_offset, void* cookie)
{
  FindComponentData* data = (FindComponentData*)cookie;

  if (data->wanted_index == data->current_index)
  {
    data->component_found = true;
    data->name_offset = name_offset;
    data->descriptor_offset = descriptor_offset;
    return false;
  }
  
  data->current_index++;
  return true;
}

/* XXX: Index-based access is very slow */
const char* unshield_component_name(Unshield* unshield, int index)
{
  Header* header = unshield->header_list;
  FindComponentData data;
  const char* name = NULL;

  memset(&data, 0, sizeof(FindComponentData));
  data.wanted_index = index;

  unshield_foreach_component(header, unshield_find_component_callback, &data);

  /*unshield_trace("data.name_offset = %08x", data.name_offset);*/

  if (data.component_found && data.name_offset)
  {
    /*unshield_trace("cab_descriptor_offset = %08x", 
        header->common.cab_descriptor_offset);*/

    name = (const char*)(
        header->data +
        header->common.cab_descriptor_offset +
        data.name_offset);
  }

  /*unshield_trace("component name %i = '%s'", index, name);*/

  return name;
}

