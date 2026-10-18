#include "../lib/internal.h"
#include <stdlib.h>
#undef NDEBUG
#include <assert.h>

int main(void)
{
  const uint64_t sizes[] = {0, INT64_MAX, (uint64_t)INT64_MAX + 1};
  for (int field = 0; field < 2; ++field)
  {
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i)
    {
      uint8_t data[0x57] = {UNSHIELD_FILE_COMPRESSED};
      Header header = {.data = data, .major_version = 6,
                       .cab = {.file_count = 1, .directory_count = 1}};
      Unshield unshield = {.header_list = &header};
      for (int byte = 0; byte < 8; ++byte)
        data[2 + field * 8 + byte] = sizes[i] >> (byte * 8);

      (void)unshield_file_size(&unshield, 0);
      FileDescriptor* fd = header.file_descriptors[0];
      assert((fd != NULL) == (sizes[i] <= INT64_MAX));
      if (fd)
        assert((field == 0 ? fd->expanded_size : fd->compressed_size) == (int64_t)sizes[i]);
      free(fd);
      free(header.file_descriptors);
    }
  }
  return 0;
}
