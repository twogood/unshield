#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#include "internal.h"
#include "log.h"
#include "convert_utf/ConvertUTF.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef _WIN32
  #define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
  #include <direct.h>
  #ifndef PATH_MAX
    #define PATH_MAX _MAX_PATH
  #endif
#else
  #include <limits.h>
#endif

#define VERBOSE 0

#if defined(_MSC_VER)
  #define snprintf _snprintf
  #define vsnprintf _vsnprintf
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
#endif

long int unshield_get_path_max(Unshield* unshield)
{
#ifdef PATH_MAX
    return PATH_MAX;
#else
    long int path_max = pathconf(unshield->filename_pattern, _PC_PATH_MAX);
    if (path_max <= 0)
      path_max = 4096;
    return path_max;
#endif
}

char *unshield_get_base_directory_name(Unshield *unshield) {
    long int path_max = unshield_get_path_max(unshield);
    char *p = strrchr(unshield->filename_pattern, '/');
    char *dirname = malloc(path_max);

    if (p) {
        strncpy(dirname, unshield->filename_pattern, path_max);
        if ((unsigned int) (p - unshield->filename_pattern) > path_max) {
            dirname[path_max - 1] = 0;
        } else
            dirname[(p - unshield->filename_pattern)] = 0;
    } else
        strcpy(dirname, ".");

    return dirname;
}


static char* get_filename(Unshield* unshield, int index, const char* suffix) {
    if (unshield && unshield->filename_pattern)
    {
        long path_max = unshield_get_path_max(unshield);
        char* filename  = malloc(path_max);

        if (filename == NULL) {
            unshield_error("Unable to allocate memory.\n");
            goto exit;
        }

        if (snprintf(filename, path_max, unshield->filename_pattern, index, suffix) >= path_max) {
            unshield_error("Pathname exceeds system limits.\n");
            goto exit;
        }

    exit:
        return filename;
    }

    return NULL;
}


FILE* unshield_fopen_for_reading(Unshield* unshield, int index, const char* suffix)
{
  if (unshield && unshield->filename_pattern)
  {
    FILE* result = NULL;
    char* filename = get_filename(unshield, index, suffix);
    char* dirname = unshield_get_base_directory_name(unshield);
    const char *q;
    struct dirent *dent = NULL;
    DIR *sourcedir = NULL;
    long int path_max = unshield_get_path_max(unshield);

    q=strrchr(filename,'/');
    if (q)
      q++;
    else
      q=filename;

    sourcedir = opendir(dirname);
    /* Search for the File case independent */
    if (sourcedir)
    {
      for (dent=readdir(sourcedir);dent;dent=readdir(sourcedir))
      {
        if (!(strcasecmp(q, dent->d_name)))
        {
          /*unshield_trace("Found match %s\n",dent->d_name);*/
          break;
        }
      }
      
      if (dent == NULL)
      {
        unshield_trace("File %s not found even case insensitive\n",filename);
        goto exit;
      }
      else
        if(snprintf(filename, path_max, "%s/%s", dirname, dent->d_name)>=path_max)
        {
          unshield_error("Pathname exceeds system limits.\n");
          goto exit;
        }
    }
    else
      unshield_trace("Could not open directory %s error %s\n", dirname, strerror(errno));

#if VERBOSE
    unshield_trace("Opening file '%s'", filename);
#endif
    result = fopen(filename, "rb");

exit:
    if (sourcedir)
      closedir(sourcedir);
    free(filename);
    free(dirname);
    return result;
  }

  return NULL;
}

long unshield_fsize(FILE* file)
{
  long result;
  long previous = ftell(file);
  fseek(file, 0L, SEEK_END);
  result = ftell(file);
  fseek(file, previous, SEEK_SET);
  return result;
}

bool unshield_read_common_header(uint8_t** buffer, CommonHeader* common)
{
  uint8_t* p = *buffer;
  common->signature              = READ_UINT32(p); p += 4;

  if (CAB_SIGNATURE != common->signature)
  {
    unshield_error("Invalid file signature");

    if (MSCF_SIGNATURE == common->signature)
      unshield_warning("Found Microsoft Cabinet header. Use cabextract (https://www.cabextract.org.uk/) to unpack this file.");

    return false;
  }

  common->version                = READ_UINT32(p); p += 4;
  common->volume_info            = READ_UINT32(p); p += 4;
  common->cab_descriptor_offset  = READ_UINT32(p); p += 4;
  common->cab_descriptor_size    = READ_UINT32(p); p += 4;

#if VERBOSE
  unshield_trace("Common header: %08x %08x %08x %08x",
      common->version, 
      common->volume_info, 
      common->cab_descriptor_offset, 
      common->cab_descriptor_size);
#endif

  *buffer = p;
  return true;
}

/**
  Get pointer at cab descriptor + offset
  */
uint8_t* unshield_header_get_buffer(Header* header, uint32_t offset)
{
  if (offset)
    return 
      header->data +
      header->common.cab_descriptor_offset +
      offset;
  else
    return NULL;
}


static int unshield_strlen_utf16(const uint16_t* utf16)
{
  const uint16_t* current = utf16;
  while (*current++)
    ;
  return current - utf16;
}


static StringBuffer* unshield_add_string_buffer(Header* header)
{
  StringBuffer* result = NEW1(StringBuffer);
  result->next = header->string_buffer;
  return header->string_buffer = result;
}


static const char* unshield_utf16_to_utf8(Header* header, const uint16_t* utf16)
{
  StringBuffer* string_buffer = unshield_add_string_buffer(header); 
  int length = unshield_strlen_utf16(utf16);
  int buffer_size = 3 * length + 1;
  char* target = string_buffer->string = NEW(char, buffer_size);
  ConversionResult result = ConvertUTF16toUTF8(
      (const UTF16**)&utf16, utf16 + length + 1, 
      (UTF8**)&target, (UTF8*)(target + buffer_size), lenientConversion);
  if (result != conversionOK)
  {
    /* fail fast */
    abort();
  }
  return string_buffer->string;
}

const char* unshield_get_utf8_string(Header* header, const void* buffer)
{
  if (header->major_version >= 17 && buffer != NULL)
  {
    return unshield_utf16_to_utf8(header, (const uint16_t*)buffer);
  }
  else
  {
    return (const char*)buffer;
  }
}

/**
  Get string at cab descriptor offset + string offset
 */
const char* unshield_header_get_string(Header* header, uint32_t offset)
{
  return unshield_get_utf8_string(header, unshield_header_get_buffer(header, offset));
}


