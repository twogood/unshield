#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <libunshield.h>

typedef struct
{
  bool opened;
  const char* name;
  uint32_t index;
  struct dirent dirent[2];
} StackMemoryDirectory_t;

typedef struct
{
  bool opened;
  const char* name;
  uint8_t *buf;
  uint32_t size;
  uint32_t current_pointer;
} StackMemoryFile_t;

typedef struct
{
  bool opened;
  const char* filename;
  StackMemoryDirectory_t directory;
  StackMemoryFile_t files[3];
} StackMemoryIoCallbacksUserData_t;

void *stack_memory_fopen(const char *filename, const char *modes, void *userdata)
{
  (void)modes;
  StackMemoryIoCallbacksUserData_t *userdata_stack_memory = (StackMemoryIoCallbacksUserData_t *) userdata;
  for (size_t i = 0; i < sizeof(userdata_stack_memory->files) / sizeof(userdata_stack_memory->files[0]); ++i)
  {
    StackMemoryFile_t* file = &userdata_stack_memory->files[i];
    if (file->name != NULL && strcmp(file->name, filename) == 0)
    {
      if (file->opened)
        return NULL;
      userdata_stack_memory->opened = true;
      return file;
    }
  }
  return NULL;
}

int stack_memory_fseek(void *file, long int offset, int whence, void *userdata)
{
  (void)userdata;
  StackMemoryFile_t *stack_file = (StackMemoryFile_t *) file;
  long int new_offset = (long int)stack_file->current_pointer;
  switch (whence)
  {
    case SEEK_SET:
      new_offset = offset;
      break;
    case SEEK_CUR:
      new_offset += offset;
      break;
    case SEEK_END:
      new_offset = (long int)stack_file->size + offset;
      break;
    default:
      return 1;
  }
  if (new_offset < 0 && new_offset > stack_file->size)
    return 1;
  stack_file->current_pointer = new_offset;
  return 0;
}

long int stack_memory_ftell(void *file, void *userdata)
{
  (void)userdata;
  StackMemoryFile_t *stack_file = (StackMemoryFile_t *) file;
  return (long int)stack_file->current_pointer;
}

size_t stack_memory_fread(void *ptr, size_t size, size_t n, void *file, void *userdata)
{
  (void)userdata;
  if (size == 0 || n == 0)
    return 0;
  uint32_t total_size = size * n;
  StackMemoryFile_t *stack_file = (StackMemoryFile_t *) file;
  if (total_size > stack_file->current_pointer + stack_file->size)
    total_size = stack_file->size - stack_file->current_pointer;
  memcpy(ptr, stack_file->buf + stack_file->current_pointer, total_size);
  stack_file->current_pointer += total_size;
  if (stack_file->current_pointer == stack_file->size)
    stack_file->current_pointer = EOF;
  return total_size;
}

size_t stack_memory_fwrite(const void *ptr, size_t size, size_t n, void *file, void *userdata)
{
  (void)userdata;
  if (size == 0 || n == 0)
    return 0;
  StackMemoryFile_t *stack_file = (StackMemoryFile_t *) file;
  memcpy(stack_file->buf + stack_file->current_pointer, ptr, size * n);
  stack_file->current_pointer += size * n;
  return size * n;
}

int stack_memory_fclose(void *file, void *userdata)
{
  (void)userdata;
  StackMemoryFile_t *stack_file = (StackMemoryFile_t *) file;
  if (!stack_file->opened)
    return EOF;
  stack_file->opened = false;
  return 0;
}

void *stack_memory_opendir(const char *name, void *userdata)
{
  StackMemoryIoCallbacksUserData_t *userdata_stack_memory = (StackMemoryIoCallbacksUserData_t *) userdata;
  if (strcmp(userdata_stack_memory->directory.name, name) != 0)
    return NULL;
  if (userdata_stack_memory->directory.opened)
    return NULL;
  userdata_stack_memory->directory.opened = true;
  userdata_stack_memory->directory.index = 0;
  return &userdata_stack_memory->directory;
}

int stack_memory_closedir(void *dir, void *userdata)
{
  StackMemoryIoCallbacksUserData_t *userdata_stack_memory = (StackMemoryIoCallbacksUserData_t *) userdata;
  if (dir != &userdata_stack_memory->directory)
    return -1;
  if (!userdata_stack_memory->directory.opened)
    return -1;
  userdata_stack_memory->directory.opened = false;
  return 0;
}

struct dirent* stack_memory_readdir(void *dir, void *userdata)
{
  StackMemoryIoCallbacksUserData_t *userdata_stack_memory = (StackMemoryIoCallbacksUserData_t *) userdata;
  if (dir != &userdata_stack_memory->directory)
    return NULL;
  if (userdata_stack_memory->directory.index >= sizeof(userdata_stack_memory->directory.dirent) / sizeof(userdata_stack_memory->directory.dirent[0]))
    return NULL;
  return &userdata_stack_memory->directory.dirent[userdata_stack_memory->directory.index++];
}

static UnshieldIoCallbacks stackMemoryFileIoCallbacks = {
  .fopen = stack_memory_fopen,
  .fseek = stack_memory_fseek,
  .ftell = stack_memory_ftell,
  .fread = stack_memory_fread,
  .fwrite = stack_memory_fwrite,
  .fclose = stack_memory_fclose,
  .opendir = stack_memory_opendir,
  .closedir = stack_memory_closedir,
  .readdir = stack_memory_readdir,
};

int test_open_and_save()
{
  uint8_t test1_hdr_data[] = {
    0x49, 0x53, 0x63, 0x28, // signature
    0x00, 0x50, 0x00, 0x01, // version
    0x00, 0x00, 0x00, 0x00, // volume_info
    0x10, 0x00, 0x00, 0x00, // cab_descriptor_offset
    0x01, 0x00, 0x00, 0x00, // cab_descriptor_size
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8
    0x76, 0x02, 0x00, 0x00, // header->cab.file_table_offset
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, // header->cab.file_table_size
    0x00, 0x00, 0x00, 0x00, // header->cab.file_table_size2
    0x00, 0x00, 0x00, 0x00, // header->cab.directory_count
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8
    0x01, 0x00, 0x00, 0x00, // header->cab.file_count
    0x00, 0x00, 0x00, 0x00, // header->cab.cab.file_table_offset2
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, // header->file_table[1]
    0x2a, 0x00, 0x00, 0x00, // fd->name_offset
    0x00, 0x00, 0x00, 0x00, // fd->directory_index
    0x00, 0x00, // fd->flags
    0x05, 0x00, 0x00, 0x00, // fd->expanded_size
    0x05, 0x00, 0x00, 0x00, // fd->compressed_size
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x14
    0x40, 0x00, 0x00, 0x00, // fd->data_offset
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // md5
    'T', 'E', 'S', 'T', 0x00, // name
  };
  uint8_t test1_cab_data[] = {
    0x49, 0x53, 0x63, 0x28, // signature
    0x00, 0x50, 0x00, 0x01, // version
    0x00, 0x00, 0x00, 0x00, // volume_info
    0x10, 0x00, 0x00, 0x00, // cab_descriptor_offset
    0x01, 0x00, 0x00, 0x00, // cab_descriptor_size
    0x00, 0x00, 0x00, 0x00, // volume_header.data_offset;
    0x00, 0x00, 0x00, 0x00, // volume_header.unknown;
    0x00, 0x00, 0x00, 0x00, // volume_header.first_file_index;
    0x00, 0x00, 0x00, 0x00, // volume_header.last_file_index;
    0x40, 0x00, 0x00, 0x00, // volume_header.first_file_offset;
    0x05, 0x00, 0x00, 0x00, // volume_header.first_file_size_expanded;
    0x05, 0x00, 0x00, 0x00, // volume_header.first_file_size_compressed;
    0x40, 0x00, 0x00, 0x00, // volume_header.last_file_offset;
    0x05, 0x00, 0x00, 0x00, // volume_header.last_file_size_expanded;
    0x05, 0x00, 0x00, 0x00, // volume_header.last_file_size_compressed;
    0x00, 0x00, 0x00, 0x00,
    'T', 'E', 'S', 'T', 0x00, // name
  };
  uint8_t test_output_txt_data[0x10] = { 0, };
  StackMemoryIoCallbacksUserData_t stack_file = {
    .opened = false,
    .filename = "test.cab",
    .directory = {
      .opened = false,
      .name = ".",
      .dirent = {
        [0] = { .d_name = "test1.hdr" },
        [1] = { .d_name = "test1.cab" },
      },
    },
    .files = {
      [0] = {
        .name = "./test1.hdr",
        .buf = test1_hdr_data,
        .size = sizeof (test1_hdr_data),
      },
      [1] = {
        .name = "./test1.cab",
        .buf = test1_cab_data,
        .size = sizeof (test1_cab_data),
      },
      [2] = {
        .name = "test_output.txt",
        .buf = test_output_txt_data,
        .size = sizeof (test_output_txt_data),
      },
    }
  };

  Unshield *unshield = unshield_open2(stack_file.filename, &stackMemoryFileIoCallbacks, &stack_file);
  if (unshield == NULL)
    return EXIT_FAILURE;

  if (!stack_file.opened)
    return EXIT_FAILURE;

  if (unshield_file_count(unshield) != 1)
  {
    unshield_close(unshield);
    return EXIT_FAILURE;
  }

  if (!unshield_file_save(unshield, 0, "test_output.txt"))
  {
    unshield_close(unshield);
    return EXIT_FAILURE;
  }

  if (strcmp((char*)test_output_txt_data, "TEST") != 0)
  {
      unshield_close(unshield);
      return EXIT_FAILURE;
  }

  unshield_close(unshield);

  return EXIT_SUCCESS;
}


int main()
{
  if (test_open_and_save() != EXIT_SUCCESS)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
