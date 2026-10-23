#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#include "internal.h"
#include "log.h"

static void unshield_free_string_buffers(Header* header);
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#ifdef _WIN32
  #define unshield_native_fseek _fseeki64
  #define unshield_native_ftell _ftelli64
#else
  #define unshield_native_fseek fseek
  #define unshield_native_ftell ftell
#endif


void *unshield_default_fopen(const char *filename, const char *modes, void *userdata)
{
  (void)userdata;
  return fopen(filename, modes);
}

int unshield_default_fseek(void *file, long int offset, int whence, void *userdata)
{
  (void)userdata;
  return unshield_native_fseek(file, offset, whence);
}

long int unshield_default_ftell(void *file, void *userdata)
{
  (void)userdata;
  return unshield_native_ftell(file);
}

size_t unshield_default_fread(void *ptr, size_t size, size_t n, void *file, void *userdata)
{
  (void)userdata;
  return fread(ptr, size, n, file);
}

size_t unshield_default_fwrite(const void *ptr, size_t size, size_t n, void *file, void *userdata)
{
  (void)userdata;
  return fwrite(ptr, size, n, file);
}

int unshield_default_fclose(void *file, void *userdata)
{
  (void)userdata;
  return fclose(file);
}

void *unshield_default_opendir(const char *name, void *userdata)
{
  (void)userdata;
  return opendir(name);
}

int unshield_default_closedir(void *dir, void *userdata)
{
  (void)userdata;
  return closedir(dir);
}

struct dirent* unshield_default_readdir(void *dir, void *userdata)
{
  (void)userdata;
  return readdir(dir);
}

static UnshieldIoCallbacks unshield_default_io_callbacks = {
  .fopen = unshield_default_fopen,
  .fseek = unshield_default_fseek,
  .ftell = unshield_default_ftell,
  .fread = unshield_default_fread,
  .fwrite = unshield_default_fwrite,
  .fclose = unshield_default_fclose,
  .opendir = unshield_default_opendir,
  .closedir = unshield_default_closedir,
  .readdir = unshield_default_readdir,
};

/**
  Create filename pattern used by unshield_fopen_for_reading()
 */
static bool unshield_create_filename_pattern(Unshield* unshield, const char* filename)/*{{{*/
{
  /* 
     TODO
     Correct his function so that it handles filenames with more than one dot ('.')! 
   */
  
  if (unshield && filename)
  {
    char pattern[256];
    char* prefix = strdup(filename);
    char* p = unshield_get_last_path_separator(prefix);
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
    /* On-disk descriptor layout: 0x3e bytes of fields/gaps followed by
       MAX_FILE_GROUP_COUNT + MAX_COMPONENT_COUNT 32-bit offsets, so the
       parser reads 0x3e + 4*(71+71) = 0x276 bytes.  Do not derive this from
       offsetof(CabDescriptor): the in-memory struct does not model the
       on-disk gaps (file_table_offset sits at 0x0c, etc.). */
    const size_t descriptor_size =
      0x3e + sizeof(header->cab.file_group_offsets) +
      sizeof(header->cab.component_offsets);
    uint8_t* p = unshield_header_get_buffer_checked(header, 0, descriptor_size);
    int i;

    if (!p)
    {
      unshield_error("CAB descriptor (offset %lu, %lu bytes) is out of bounds "
          "for header of %lu bytes",
          (unsigned long)header->common.cab_descriptor_offset,
          (unsigned long)descriptor_size,
          (unsigned long)header->size);
      return false;
    }

    p += 0xc;
    header->cab.file_table_offset   = READ_UINT32(p); p += 4;
    p += 4;
    header->cab.file_table_size     = READ_UINT32(p); p += 4;
    header->cab.file_table_size2    = READ_UINT32(p); p += 4;
    header->cab.directory_count     = READ_UINT32(p); p += 4;
    p += 8;
    header->cab.file_count          = READ_UINT32(p); p += 4;
    header->cab.file_table_offset2  = READ_UINT32(p); p += 4;

    UNSHIELD_ASSERT((p - (header->data + header->common.cab_descriptor_offset)) == 0x30);

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

    p += 0xe;

    for (i = 0; i < MAX_FILE_GROUP_COUNT; i++)
    {
      header->cab.file_group_offsets[i] = READ_UINT32(p); p += 4;
    }

    for (i = 0; i < MAX_COMPONENT_COUNT; i++)
    {
      header->cab.component_offsets[i] = READ_UINT32(p); p += 4;
    }

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
  uint64_t start = (uint64_t)header->common.cab_descriptor_offset +
                   (uint64_t)header->cab.file_table_offset;
  uint64_t count = (uint64_t)header->cab.directory_count +
                   (uint64_t)header->cab.file_count;
  uint64_t bytes = count * (uint64_t)sizeof(uint32_t);
  uint8_t* p;
  uint64_t i;

  if (count == 0)
  {
    unshield_error("File table is empty");
    return false;
  }

  if (start > header->size || bytes > (uint64_t)header->size - start)
  {
    unshield_error("File table (offset %lu, %lu bytes) is out of bounds "
        "for header of %lu bytes",
        (unsigned long)start, (unsigned long)bytes,
        (unsigned long)header->size);
    return false;
  }

  p = header->data + start;

  header->file_table = calloc(count, sizeof(uint32_t));
  if (!header->file_table)
  {
    unshield_error("Failed to allocate file table");
    return false;
  }

  for (i = 0; i < count; i++)
  {
    header->file_table[i] = READ_UINT32(p); p += 4;
  }
  
  return true;
}  

static bool unshield_header_get_components(Header* header)/*{{{*/
{
  int count = 0;
  int i;
	int available = 16;

	header->components = malloc(available * sizeof(UnshieldComponent*));
	if (!header->components)
	  return false;

  for (i = 0; i < MAX_COMPONENT_COUNT; i++)
  {
    if (header->cab.component_offsets[i])
    {
      OffsetList list;
      uint64_t guard = (uint64_t)header->size + 1;

      list.next_offset = header->cab.component_offsets[i];

      while (list.next_offset && guard--)
      {
        uint8_t* p = unshield_header_get_buffer_checked(header, list.next_offset, 12);
        UnshieldComponent* component;

        if (!p)
        {
          unshield_error("Component list offset %08x is out of bounds "
              "for header of %lu bytes",
              list.next_offset, (unsigned long)header->size);
          return false;
        }

        list.name_offset       = READ_UINT32(p); p += 4;
        list.descriptor_offset = READ_UINT32(p); p += 4;
        list.next_offset       = READ_UINT32(p); p += 4;

				if (count == available)
				{
					UnshieldComponent** resized;
					available <<= 1;
					resized = realloc(header->components, available * sizeof(UnshieldComponent*));
					if (!resized)
					{
					  unshield_error("Failed to allocate component table");
					  return false;
					}
					header->components = resized;
				}

        component = unshield_component_new(header, list.descriptor_offset);
        if (!component)
        {
          unshield_error("Failed to read component descriptor at offset %08x",
              list.descriptor_offset);
          return false;
        }

        header->components[count++] = component;
        header->component_count = count;
      }

      if (list.next_offset)
      {
        unshield_error("Component list is too long");
        return false;
      }
    }
  }

  header->component_count = count;

  return true;
}  /*}}}*/

static bool unshield_header_get_file_groups(Header* header)/*{{{*/
{
  int count = 0;
  int i;
	int available = 16;

	header->file_groups = malloc(available * sizeof(UnshieldFileGroup*));
	if (!header->file_groups)
	  return false;

  for (i = 0; i < MAX_FILE_GROUP_COUNT; i++)
  {
    if (header->cab.file_group_offsets[i])
    {
      OffsetList list;
      uint64_t guard = (uint64_t)header->size + 1;

      list.next_offset = header->cab.file_group_offsets[i];

      while (list.next_offset && guard--)
      {
        uint8_t* p = unshield_header_get_buffer_checked(header, list.next_offset, 12);
        UnshieldFileGroup* file_group;

        if (!p)
        {
          unshield_error("File group list offset %08x is out of bounds "
              "for header of %lu bytes",
              list.next_offset, (unsigned long)header->size);
          return false;
        }

        list.name_offset       = READ_UINT32(p); p += 4;
				list.descriptor_offset = READ_UINT32(p); p += 4;
				list.next_offset       = READ_UINT32(p); p += 4;

				if (count == available)
				{
					UnshieldFileGroup** resized;
					available <<= 1;
					resized = realloc(header->file_groups, available * sizeof(UnshieldFileGroup*));
					if (!resized)
					{
					  unshield_error("Failed to allocate file group table");
					  return false;
					}
					header->file_groups = resized;
				}

				file_group = unshield_file_group_new(header, list.descriptor_offset);
				if (!file_group)
				{
				  unshield_error("Failed to read file group descriptor at offset %08x",
				      list.descriptor_offset);
				  return false;
				}

				header->file_groups[count++] = file_group;
				header->file_group_count = count;
			}
    }
  }

  header->file_group_count = count;

  return true;
}  /*}}}*/

/**
  Read all header files
 */
static bool unshield_read_headers(Unshield* unshield, int version)/*{{{*/
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

      header->size = FSIZE(unshield, file);
      if (header->size < 4)
      {
        unshield_error("Header file %i too small", i);
        FCLOSE(unshield, file);
        goto error;
      }

      header->data = malloc(header->size);
      if (!header->data)
      {
        unshield_error("Failed to allocate memory for header file %i", i);
        FCLOSE(unshield, file);
        goto error;
      }

      bytes_read = unshield_fread(unshield, header->data, 1, header->size, file);
      FCLOSE(unshield, file);

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

      if (version != -1)
      {
        header->major_version = version;
      }
      else if (header->common.version >> 24 == 1)
      {
        header->major_version = (header->common.version >> 12) & 0xf;
      }
      else if (header->common.version >> 24 == 2
               || header->common.version >> 24 == 4)
      {
        header->major_version = (header->common.version & 0xffff);
        if (header->major_version != 0)
          header->major_version = header->major_version / 100;
      }

#if 0
      if (header->major_version < 5)
        header->major_version = 5;
#endif

      unshield_trace("Version 0x%08x handled as major version %i", 
          header->common.version,
          header->major_version);

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

      if (!unshield_header_get_components(header))
      {
        unshield_error("Failed to read components from header file %i", i);
        goto error;
      }

      if (!unshield_header_get_file_groups(header))
      {
        unshield_error("Failed to read file groups from header file %i", i);
        goto error;
      }

      if (previous)
        previous->next = header;
      else
        previous = unshield->header_list = header;

      continue;

error:
      if (header)
      {
        int n;

        for (n = 0; n < header->component_count; n++)
          unshield_component_destroy(header->components[n]);
        FREE(header->components);

        for (n = 0; n < header->file_group_count; n++)
          unshield_file_group_destroy(header->file_groups[n]);
        FREE(header->file_groups);

        FREE(header->file_table);

        unshield_free_string_buffers(header);
        FREE(header->data);
      }
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
  return unshield_open_force_version(filename, -1);
}/*}}}*/

Unshield* unshield_open_force_version(const char* filename, int version)/*{{{*/
{
  return unshield_open2_force_version(filename, version, NULL, NULL);
}/*}}}*/

Unshield* unshield_open2(const char* filename, const UnshieldIoCallbacks* callbacks, void* userdata)/*{{{*/
{
  return unshield_open2_force_version(filename, -1, callbacks, userdata);
}/*}}}*/

Unshield* unshield_open2_force_version(const char* filename, int version, const UnshieldIoCallbacks* callbacks, void* userdata)/*{{{*/
{
  Unshield* unshield = NEW1(Unshield);
  if (!unshield)
  {
    unshield_error("Failed to allocate memory for Unshield structure");
    goto error;
  }

  unshield->io_callbacks = callbacks == NULL ? &unshield_default_io_callbacks : callbacks;
  unshield->io_userdata = userdata;

  if (!unshield_create_filename_pattern(unshield, filename))
  {
    unshield_error("Failed to create filename pattern");
    goto error;
  }

  if (!unshield_read_headers(unshield, version))
  {
    unshield_error("Failed to read header files");
    goto error;
  }

  return unshield;

error:
  unshield_close(unshield);
  return NULL;
}/*}}}*/


static void unshield_free_string_buffers(Header* header)
{
  StringBuffer* current = header->string_buffer;
  header->string_buffer = NULL;

  while (current != NULL)
  {
    StringBuffer* next = current->next;
    FREE(current->string);
    FREE(current);
    current = next;
  }
}

void unshield_close(Unshield* unshield)/*{{{*/
{
  if (unshield)
  {
    Header* header;

    for(header = unshield->header_list; header; )
    {
      Header* next = header->next;
      int i;

      unshield_free_string_buffers(header);

			if (header->components)
			{
				for (i = 0; i < header->component_count; i++)
					unshield_component_destroy(header->components[i]);
				free(header->components);
			}

			if (header->file_groups)
			{
				for (i = 0; i < header->file_group_count; i++)
					unshield_file_group_destroy(header->file_groups[i]);
				free(header->file_groups);
			}

      if (header->file_descriptors)
      {
        for (i = 0; i < (int)header->cab.file_count; i++)
          FREE(header->file_descriptors[i]);
        free(header->file_descriptors);
      }

      FREE(header->file_table);
      
      FREE(header->data);
      FREE(header);

      header = next;
    }

    FREE(unshield->filename_pattern);
    free(unshield);
  }
}/*}}}*/

bool unshield_is_unicode(Unshield* unshield)
{
  if (unshield)
  {
    Header* header = unshield->header_list;

    return header->major_version >= 17;
  }
  else
    return false;
}
