/* $Id$ */
#include "unshield_internal.h"
#include "cabfile.h"
#include <synce_log.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static FileDescriptor5* unshield_file_descriptor5(Header* header, int index)/*{{{*/
{
  return (FileDescriptor5*)(
      (uint8_t*)header->file_table + 
      header->file_table[header->cab->directory_count + index]
      );
}/*}}}*/

static FileDescriptor6* unshield_file_descriptor6(Header* header, int index)/*{{{*/
{
  return ((FileDescriptor6*)
    ((uint8_t*)header->file_table + letoh32(header->cab->file_table_offset2)))
    + index;
}/*}}}*/

int unshield_file_count (Unshield* unshield)/*{{{*/
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    return header->cab->file_count;
  }
  else
    return -1;
}/*}}}*/

const char* unshield_file_name (Unshield* unshield, int index)/*{{{*/
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    switch (unshield->major_version)
    {
      case 5:
        {
          FileDescriptor5* fd = unshield_file_descriptor5(header, index);
          return (const char*)((uint8_t*)header->file_table + letoh32(fd->name_offset));
        }
        break;

      case 6:
        {
          FileDescriptor6* fd = unshield_file_descriptor6(header, index);
          return (const char*)((uint8_t*)header->file_table + letoh32(fd->name_offset));
        }
        break;

      default:
        synce_error("Unknown major version: %i", unshield->major_version);
        return NULL;
    }

  }
  else
    return NULL;
}/*}}}*/

static int unshield_uncompress (Byte *dest, uLong *destLen, Byte *source, uLong sourceLen)/*{{{*/
{
    z_stream stream;
    int err;

    stream.next_in = source;
    stream.avail_in = (uInt)sourceLen;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    /* make second parameter negative to disable checksum verification */
    err = inflateInit2(&stream, -MAX_WBITS);
    if (err != Z_OK) return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        return err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}/*}}}*/

#define BUFFER_SIZE (64*1024+1)

bool unshield_file_save (Unshield* unshield, int index, const char* filename)/*{{{*/
{
  bool success = false;
  Header* header = NULL;
  FILE* input = NULL;
  FILE* output = NULL;
  unsigned char* input_buffer   = (unsigned char*)malloc(BUFFER_SIZE);
  unsigned char* output_buffer  = (unsigned char*)malloc(BUFFER_SIZE);
  int bytes_left;
  uint32_t data_offset;
  uint32_t expanded_size;
  uLong total_written = 0;
  bool compressed = true;
  int volume;

  if (!unshield || !filename)
    goto exit;
  
  /* XXX: multi-volume support... */
  header = unshield->header_list;

  switch (unshield->major_version)
  {
    case 5:
      {
        FileDescriptor5* fd = unshield_file_descriptor5(header, index);
        data_offset   = letoh32(fd->data_offset);
        bytes_left    = letoh32(fd->compressed_size);
        expanded_size = letoh32(fd->expanded_size);
        volume        = header->index;
      }
      break;

    case 6:
      {
        FileDescriptor6* fd = unshield_file_descriptor6(header, index);
        uint32_t status = letoh32(fd->status);

        if (status & FILE_INVALID)
        {
          synce_error("Not a valid file");
          goto exit;
        }

        if (status & FILE_SPLIT)
        {
          synce_trace("Split file!");
        }

        if (status & FILE_COMPRESSED)
        {
          compressed = true;
          bytes_left = letoh32(fd->compressed_size);
        }
        else
        {
          compressed = false;
          bytes_left = letoh32(fd->expanded_size);
        }
        
        data_offset   = letoh32(fd->data_offset);
        expanded_size = letoh32(fd->expanded_size);
        volume        = letoh16(fd->volume);
        /*synce_trace("Compressed size: %i", bytes_left);*/
      }
      break;

    default:
      synce_error("Unknown major version: %i", unshield->major_version);
      return NULL;
  }

  input = unshield_fopen_for_reading(unshield, volume, CABINET_SUFFIX);
  if (!input)
  {
    synce_error("Failed to open input cabinet file %i", volume);
    goto exit;
  }

  output = fopen(filename, "w");
  if (!output)
  {
    synce_error("Failed to open output file '%s'", filename);
    goto exit;
  }

  if (fseek(input, data_offset, SEEK_SET))
  {
    synce_error("Failed to seek in input cabinet file %i", volume);
    goto exit;
  }

  while (bytes_left > 0)
  {
    uLong bytes_to_write = BUFFER_SIZE;
    int result;

    if (compressed)
    {
      uint16_t bytes_to_read = 0;

      if (sizeof(bytes_to_read) != fread(&bytes_to_read, 1, sizeof(bytes_to_read), input))
      {
        synce_error("Failed to read %i bytes from input cabinet file %i", 
            sizeof(bytes_to_read), volume);
        goto exit;
      }

      bytes_to_read = letoh16(bytes_to_read);
      if (bytes_to_read != fread(input_buffer, 1, bytes_to_read, input))
      {
        synce_error("Failed to read %i bytes from input cabinet file %i", 
            bytes_to_read, volume);
        goto exit;
      }

      /* add a null byte to make inflate happy */
      input_buffer[bytes_to_read] = 0;
      result = unshield_uncompress(output_buffer, &bytes_to_write, input_buffer, bytes_to_read+1);

      if (Z_OK != result)
      {
        synce_error("Decompression failed with code %i. bytes_to_read=%i, bytes_left=%i", 
            result, bytes_to_read, bytes_left);
        /*      abort();*/
        goto exit;
      }

      bytes_left -= 2;
      bytes_left -= bytes_to_read;
    }
    else
    {
      bytes_to_write = (bytes_left > BUFFER_SIZE) ? BUFFER_SIZE : bytes_left;
      if (bytes_to_write != fread(input_buffer, 1, bytes_to_write, input))
      {
        synce_error("Failed to read %i bytes from input cabinet file %i", 
            bytes_to_write, volume);
        goto exit;
      }

      bytes_left -= bytes_to_write;
    }

    if (bytes_to_write != fwrite(output_buffer, 1, bytes_to_write, output))
    {
      synce_error("Failed to write %i bytes to file '%s'", bytes_to_write, filename);
      goto exit;
    }

    total_written += bytes_to_write;
  }

  if (expanded_size != total_written)
  {
    synce_error("Expanded size expected to be %i, but was %i", expanded_size, total_written);
    goto exit;
  }

  success = true;
  
exit:
  FCLOSE(input);
  FCLOSE(output);
  FREE(input_buffer);
  FREE(output_buffer);
  return success;
}/*}}}*/

int unshield_file_directory(Unshield* unshield, int index)
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    switch (unshield->major_version)
    {
      case 5:
        {
          FileDescriptor5* fd = unshield_file_descriptor5(header, index);
          return fd->directory_index;
        }
        break;

      case 6:
        {
          FileDescriptor6* fd = unshield_file_descriptor6(header, index);
          return fd->directory_index;
        }
        break;

      default:
        synce_error("Unknown major version: %i", unshield->major_version);
        return -1;
    }

  }
  else
    return -1;
}

