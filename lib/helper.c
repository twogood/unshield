/* $Id$ */
#define _BSD_SOURCE 1
#include "internal.h"
#include "log.h"
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

FILE* unshield_fopen_for_reading(Unshield* unshield, int index, const char* suffix)
{
  if (unshield && unshield->filename_pattern)
  {
    FILE* result = NULL;
    char filename[256];
    char dirname[256];
    char * p = strrchr(unshield->filename_pattern, '/');
    const char *q;
    struct dirent *dent = NULL;
    DIR *sourcedir;
    snprintf(filename, sizeof(filename), unshield->filename_pattern, index, suffix);
    q=strrchr(filename,'/');
    if (q)
      q++;
    else
      q=filename;

    if (p)
    {
      strncpy( dirname, unshield->filename_pattern,sizeof(dirname));
      if ((unsigned int)(p-unshield->filename_pattern) > sizeof(dirname))
      {
        unshield_trace("WARN: size\n");
        dirname[sizeof(dirname)-1]=0;
      }
      else
        dirname[(p-unshield->filename_pattern)] = 0;
    }
    else
      strcpy(dirname,".");

    sourcedir = opendir(dirname);
    /* Search for the File case independent */
    if (sourcedir)
    {
      for (dent=readdir(sourcedir);dent;dent=readdir(sourcedir))
      {
        if (!(strcasecmp(q, dent->d_name)))
        {
          unshield_trace("Found match %s\n",dent->d_name);
          break;
        }
      }
      
      if (dent == NULL)
      {
        unshield_trace("File %s not found even case insensitive\n",filename);
        goto exit;
      }
      else
        snprintf(filename, sizeof(filename), "%s/%s", dirname, dent->d_name);
    }
    else
      unshield_trace("Could not open directory %s error %s\n", dirname, strerror(errno));

    unshield_trace("Opening file '%s'", filename);
    result = fopen(filename, "r");

exit:
    if (sourcedir)
      closedir(sourcedir);

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
      unshield_warning("Found Microsoft Cabinet header. Use cabextract (http://www.kyz.uklinux.net/cabextract.php) to unpack this file.");

    return false;
  }

  common->version                = READ_UINT32(p); p += 4;
  common->volume_info            = READ_UINT32(p); p += 4;
  common->cab_descriptor_offset  = READ_UINT32(p); p += 4;
  common->cab_descriptor_size    = READ_UINT32(p); p += 4;

  unshield_trace("Common header: %08x %08x %08x %08x",
      common->version, 
      common->volume_info, 
      common->cab_descriptor_offset, 
      common->cab_descriptor_size);

  *buffer = p;
  return true;
}


