/* $Id$ */
#include "unshield_internal.h"
#include "cabfile.h"
#include <synce_log.h>
#include <stdlib.h>
#include <zlib.h>

static FileDescriptor5* unshield_file_descriptor5(Header* header, int index)/*{{{*/
{
  return (FileDescriptor5*)(
      (uint8_t*)header->file_table + 
      header->file_table[header->cab->directory_count + index]
      );
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
    FileDescriptor5* fd5 = unshield_file_descriptor5(header, index);

    return (const char*)((uint8_t*)header->file_table + fd5->name_offset);
  }
  else
    return NULL;
}/*}}}*/

static int unshield_uncompress (Byte *dest, uLong *destLen, Byte *source, uLong sourceLen)
{
    z_stream stream;
    int err;

    stream.next_in = source;
    stream.avail_in = (uInt)sourceLen;
    /* Check for source > 64K on 16-bit machine: */
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

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
}

#define BUFFER_SIZE (64*1024+1)

bool unshield_file_save (Unshield* unshield, int index, const char* filename)
{
  bool success = false;
  Header* header = NULL;
  FileDescriptor5* fd5 = NULL;
  FILE* input = NULL;
  FILE* output = NULL;
  unsigned char* input_buffer   = (unsigned char*)malloc(BUFFER_SIZE);
  unsigned char* output_buffer  = (unsigned char*)malloc(BUFFER_SIZE);
  int bytes_left;
  
  if (!unshield || !filename)
    goto exit;
  
  /* XXX: multi-volume support... */
  header = unshield->header_list;
  fd5 = unshield_file_descriptor5(header, index);

  input = unshield_fopen_for_reading(unshield, header->index, CABINET_SUFFIX);
  if (!input)
  {
    synce_error("Failed to open input cabinet file %i", header->index);
    goto exit;
  }

  output = fopen(filename, "w");
  if (!output)
  {
    synce_error("Failed to open output file '%s'", filename);
    goto exit;
  }

  fseek(input, letoh32(fd5->data_offset), SEEK_SET);
  bytes_left = letoh32(fd5->compressed_size);

  while (bytes_left > 0)
  {
    uint16_t bytes_to_read = 0;
    uLong bytes_to_write = BUFFER_SIZE;
    int result;

    fread(&bytes_to_read, 1, sizeof(bytes_to_read), input);
    bytes_to_read = letoh16(bytes_to_read);
    fread(input_buffer, 1, bytes_to_read, input);

    /* add a null byte to make inflate happy */
    input_buffer[bytes_to_read] = 0;
    result = unshield_uncompress(output_buffer, &bytes_to_write, input_buffer, bytes_to_read+1);

    if (Z_OK != result)
    {
      synce_error("Decompression failed with code %i", result);
      goto exit;
    }

    fwrite(output_buffer, bytes_to_write, 1, output);

    bytes_left -= 2;
    bytes_left -= bytes_to_read;
  }

  success = true;
  
exit:
  FCLOSE(input);
  FCLOSE(output);
  FREE(input_buffer);
  FREE(output_buffer);
  return success;
}


