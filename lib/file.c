/* $Id$ */
#include "internal.h"
#include "md5/global.h"
#include "md5/md5.h"
#include "cabfile.h"
#include "log.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>    /* for MIN(a,b) */
#include <zlib.h>

#define VERBOSE 0

#define ror8(x,n)   (((x) >> ((int)(n))) | ((x) << (8 - (int)(n))))
#define rol8(x,n)   (((x) << ((int)(n))) | ((x) >> (8 - (int)(n))))


static FileDescriptor* unshield_read_file_descriptor(Unshield* unshield, int index)
{
  /* XXX: multi-volume support... */
  Header* header = unshield->header_list;
  uint8_t* p = NULL;
  uint8_t* saved_p = NULL;
  FileDescriptor* fd = NEW1(FileDescriptor);

  switch (header->major_version)
  {
    case 0:
    case 5:
      saved_p = p = header->data +
          header->common.cab_descriptor_offset +
          header->cab.file_table_offset +
          header->file_table[header->cab.directory_count + index];

#if VERBOSE
      unshield_trace("File descriptor offset %i: %08x", index, p - header->data);
#endif
 
      fd->volume            = header->index;

      fd->name_offset       = READ_UINT32(p); p += 4;
      fd->directory_index   = READ_UINT32(p); p += 4;

      fd->flags             = READ_UINT16(p); p += 2;

      fd->expanded_size     = READ_UINT32(p); p += 4;
      fd->compressed_size   = READ_UINT32(p); p += 4;
      p += 0x14;
      fd->data_offset       = READ_UINT32(p); p += 4;
      
#if VERBOSE >= 2
      unshield_trace("Name offset:      %08x", fd->name_offset);
      unshield_trace("Directory index:  %08x", fd->directory_index);
      unshield_trace("Flags:            %04x", fd->flags);
      unshield_trace("Expanded size:    %08x", fd->expanded_size);
      unshield_trace("Compressed size:  %08x", fd->compressed_size);
      unshield_trace("Data offset:      %08x", fd->data_offset);
#endif

      if (header->major_version == 5)
      {
        memcpy(fd->md5, p, 0x10); p += 0x10;
        assert((p - saved_p) == 0x3a);
      }

      break;

    case 6:
    case 7:
      saved_p = p = header->data +
          header->common.cab_descriptor_offset +
          header->cab.file_table_offset +
          header->cab.file_table_offset2 +
          index * 0x57;
      
#if VERBOSE
      unshield_trace("File descriptor offset: %08x", p - header->data);
#endif
      fd->flags             = READ_UINT16(p); p += 2;
      fd->expanded_size     = READ_UINT32(p); p += 4;
      p += 4;
      fd->compressed_size   = READ_UINT32(p); p += 4;
      p += 4;
      fd->data_offset       = READ_UINT32(p); p += 4;
      p += 4;
      memcpy(fd->md5, p, 0x10); p += 0x10;
      p += 0x10;
      fd->name_offset       = READ_UINT32(p); p += 4;
      fd->directory_index   = READ_UINT16(p); p += 2;

      assert((p - saved_p) == 0x40);
      
      p += 0xc;
      fd->link_previous     = READ_UINT32(p); p += 4;
      fd->link_next         = READ_UINT32(p); p += 4;
      fd->link_flags        = *p; p ++;

#if VERBOSE
      if (fd->link_flags != LINK_NONE)
      {
        unshield_trace("Link: previous=%i, next=%i, flags=%i",
            fd->link_previous, fd->link_next, fd->link_flags);
      }
#endif
      
      fd->volume            = READ_UINT16(p); p += 2;

      assert((p - saved_p) == 0x57);
      break;

    default:
      unshield_error("Unknown major version: %i", header->major_version);
      abort();
  }

  if (!(fd->flags & FILE_COMPRESSED) &&
      fd->compressed_size != fd->expanded_size)
  {
    unshield_warning("File is not compressed but compressed size is %08x and expanded size is %08x",
        fd->compressed_size, fd->expanded_size);
  }

  return fd;
}

static FileDescriptor* unshield_get_file_descriptor(Unshield* unshield, int index)
{
  /* XXX: multi-volume support... */
  Header* header = unshield->header_list;

  if (index < 0 || index >= (int)header->cab.file_count)
  {
    unshield_error("Invalid index");
    return NULL;
  }

  if (!header->file_descriptors)
    header->file_descriptors = calloc(header->cab.file_count, sizeof(FileDescriptor*));

  if (!header->file_descriptors[index])
    header->file_descriptors[index] = unshield_read_file_descriptor(unshield, index);

  return header->file_descriptors[index];
}

int unshield_file_count (Unshield* unshield)/*{{{*/
{
  if (unshield)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    return header->cab.file_count;
  }
  else
    return -1;
}/*}}}*/

const char* unshield_file_name (Unshield* unshield, int index)/*{{{*/
{
  FileDescriptor* fd = unshield_get_file_descriptor(unshield, index);

  if (fd)
  {
    /* XXX: multi-volume support... */
    Header* header = unshield->header_list;

    return (const char*)(
        header->data +
        header->common.cab_descriptor_offset +
        header->cab.file_table_offset +
        fd->name_offset);
  }
    
  unshield_warning("Failed to get file descriptor %i", index);
  return NULL;
}/*}}}*/

bool unshield_file_is_valid(Unshield* unshield, int index)
{
  bool is_valid = false;
  FileDescriptor* fd;

  if (index < 0 || index >= unshield_file_count(unshield))
    goto exit;

  if (!(fd = unshield_get_file_descriptor(unshield, index)))
    goto exit;

  if (fd->flags & FILE_INVALID)
    goto exit;

  if (!fd->name_offset)
    goto exit;

  if (!fd->data_offset)
    goto exit;

  is_valid = true;
  
exit:
  return is_valid;
}


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

typedef struct 
{
  Unshield*         unshield;
  unsigned          index;
  FileDescriptor*   file_descriptor;
  int               volume;
  FILE*             volume_file;
  VolumeHeader      volume_header;
  unsigned          volume_bytes_left;
  unsigned          obfuscation_offset;
} UnshieldReader;

static bool unshield_reader_open_volume(UnshieldReader* reader, int volume)/*{{{*/
{
  bool success = false;
  unsigned data_offset = 0;
  unsigned volume_bytes_left_compressed;
  unsigned volume_bytes_left_expanded;
  CommonHeader common_header;

#if VERBOSE >= 2
  unshield_trace("Open volume %i", volume);
#endif
  
  FCLOSE(reader->volume_file);

  reader->volume_file = unshield_fopen_for_reading(reader->unshield, volume, CABINET_SUFFIX);
  if (!reader->volume_file)
  {
    unshield_error("Failed to open input cabinet file %i", volume);
    goto exit;
  }

  {
    uint8_t tmp[COMMON_HEADER_SIZE];
    uint8_t* p = tmp;

    if (COMMON_HEADER_SIZE != 
        fread(&tmp, 1, COMMON_HEADER_SIZE, reader->volume_file))
      goto exit;

    if (!unshield_read_common_header(&p, &common_header))
      goto exit;
  }
 
  memset(&reader->volume_header, 0, sizeof(VolumeHeader));

  switch (reader->unshield->header_list->major_version)
  {
    case 0:
    case 5:
      {
        uint8_t five_header[VOLUME_HEADER_SIZE_V5];
        uint8_t* p = five_header;

        if (VOLUME_HEADER_SIZE_V5 != 
            fread(&five_header, 1, VOLUME_HEADER_SIZE_V5, reader->volume_file))
          goto exit;

        reader->volume_header.data_offset                = READ_UINT32(p); p += 4;
#if VERBOSE
        if (READ_UINT32(p))
          unshield_trace("Unknown = %08x", READ_UINT32(p));
#endif
        /* unknown */                                                      p += 4;
        reader->volume_header.first_file_index           = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_index            = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_offset          = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_size_expanded   = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_size_compressed = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_offset           = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_size_expanded    = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_size_compressed  = READ_UINT32(p); p += 4;
      }
      break;

    case 6:
    case 7:
      {
        uint8_t six_header[VOLUME_HEADER_SIZE_V6];
        uint8_t* p = six_header;

        if (VOLUME_HEADER_SIZE_V6 != 
            fread(&six_header, 1, VOLUME_HEADER_SIZE_V6, reader->volume_file))
          goto exit;

        reader->volume_header.data_offset                       = READ_UINT32(p); p += 4;
        reader->volume_header.data_offset_high                  = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_index                  = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_index                   = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_offset                 = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_offset_high            = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_size_expanded          = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_size_expanded_high     = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_size_compressed        = READ_UINT32(p); p += 4;
        reader->volume_header.first_file_size_compressed_high   = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_offset                  = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_offset_high             = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_size_expanded           = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_size_expanded_high      = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_size_compressed         = READ_UINT32(p); p += 4;
        reader->volume_header.last_file_size_compressed_high    = READ_UINT32(p); p += 4;
      }
      break;

    default:
      abort();
      goto exit;
  }
  
#if VERBOSE >= 2
  unshield_trace("First file index = %i, last file index = %i",
      reader->volume_header.first_file_index, reader->volume_header.last_file_index);
  unshield_trace("First file offset = %08x, last file offset = %08x",
      reader->volume_header.first_file_offset, reader->volume_header.last_file_offset);
#endif

  /* enable support for split archives for IS5 */
  if (reader->unshield->header_list->major_version == 5)
  {
    if (reader->index < (reader->unshield->header_list->cab.file_count - 1) &&
        reader->index == reader->volume_header.last_file_index && 
        reader->volume_header.last_file_size_compressed != reader->file_descriptor->compressed_size)
    {
      unshield_trace("IS5 split file last in volume");
      reader->file_descriptor->flags |= FILE_SPLIT;
    }
    else if (reader->index > 0 &&
        reader->index == reader->volume_header.first_file_index && 
        reader->volume_header.first_file_size_compressed != reader->file_descriptor->compressed_size)
    {
      unshield_trace("IS5 split file first in volume");
      reader->file_descriptor->flags |= FILE_SPLIT;
    }
  }

  if (reader->file_descriptor->flags & FILE_SPLIT)
  {   
#if VERBOSE
    unshield_trace(/*"Total bytes left = 0x08%x, "*/"previous data offset = 0x08%x",
        /*total_bytes_left, */data_offset); 
#endif

    if (reader->index == reader->volume_header.last_file_index)
    {
      /* can be first file too... */
#if VERBOSE
      unshield_trace("Index %i is last file in cabinet file %i",
          reader->index, volume);
#endif

      data_offset                   = reader->volume_header.last_file_offset;
      volume_bytes_left_expanded    = reader->volume_header.last_file_size_expanded;
      volume_bytes_left_compressed  = reader->volume_header.last_file_size_compressed;
    }
    else if (reader->index == reader->volume_header.first_file_index)
    {
#if VERBOSE
      unshield_trace("Index %i is first file in cabinet file %i",
          reader->index, volume);
#endif

      data_offset                   = reader->volume_header.first_file_offset;
      volume_bytes_left_expanded    = reader->volume_header.first_file_size_expanded;
      volume_bytes_left_compressed  = reader->volume_header.first_file_size_compressed;
    }
    else
      abort();

#if VERBOSE
    unshield_trace("Will read 0x%08x bytes from offset 0x%08x",
        volume_bytes_left_compressed, data_offset);
#endif
  }
  else
  {
    data_offset                  = reader->file_descriptor->data_offset;
    volume_bytes_left_expanded   = reader->file_descriptor->expanded_size;
    volume_bytes_left_compressed = reader->file_descriptor->compressed_size;
  }

  if (reader->file_descriptor->flags & FILE_COMPRESSED)
    reader->volume_bytes_left = volume_bytes_left_compressed;
  else
    reader->volume_bytes_left = volume_bytes_left_expanded;

  fseek(reader->volume_file, data_offset, SEEK_SET);

  reader->volume = volume;
  success = true;

exit:
  return success;
}/*}}}*/

static void unshield_reader_deobfuscate(UnshieldReader* reader, uint8_t* buffer, size_t size)
{
  for (; size > 0; size--, buffer++, reader->obfuscation_offset++)
  {
    *buffer = ror8(*buffer ^ 0xd5, 2) - (reader->obfuscation_offset % 0x47);
  }
}

static bool unshield_reader_read(UnshieldReader* reader, void* buffer, size_t size)/*{{{*/
{
  bool success = false;
  uint8_t* p = buffer;
  size_t bytes_left = size;

  for (;;)
  {
    /* 
       Read as much as possible from this volume
     */
    size_t bytes_to_read = MIN(bytes_left, reader->volume_bytes_left);

#if VERBOSE && 0
    unshield_trace("Trying to read %i bytes from offset %08x in volume %i", 
        bytes_to_read, ftell(reader->volume_file), reader->volume);
#endif

    if (bytes_to_read != fread(p, 1, bytes_to_read, reader->volume_file))
    {
      unshield_error("Failed to read 0x%08x bytes of file %i (%s) from volume %i. Current offset = 0x%08x",
          bytes_to_read, reader->index, 
          unshield_file_name(reader->unshield, reader->index), reader->volume,
          ftell(reader->volume_file));
      goto exit;
    }

    bytes_left -= bytes_to_read;
    reader->volume_bytes_left -= bytes_to_read;

    if (!bytes_left)
      break;

    p += bytes_to_read;

    /*
       Open next volume
     */

    if (!unshield_reader_open_volume(reader, reader->volume + 1))
    {
      unshield_error("Failed to open volume %i to read %i more bytes",
          reader->volume + 1, bytes_to_read);
      goto exit;
    }
  }

  if (reader->file_descriptor->flags & FILE_OBFUSCATED)
    unshield_reader_deobfuscate(reader, buffer, size);

  success = true;

exit:
  return success;
}/*}}}*/

static UnshieldReader* unshield_reader_create(/*{{{*/
    Unshield* unshield, 
    int index,
    FileDescriptor* file_descriptor)
{
  bool success = false;
  
  UnshieldReader* reader = NEW1(UnshieldReader);
  if (!reader)
    return NULL;

  reader->unshield          = unshield;
  reader->index             = index;
  reader->file_descriptor   = file_descriptor;

  for (;;)
  {
    if (!unshield_reader_open_volume(reader, file_descriptor->volume))
    {
      unshield_error("Failed to open volume %i",
          file_descriptor->volume);
      goto exit;
    }

    /* Start with the correct volume for IS5 cabinets */
    if (reader->unshield->header_list->major_version == 5 &&
        index > (int)reader->volume_header.last_file_index)
    {
      file_descriptor->volume++;
      continue;
    }
      
    break;  
  };

  success = true;

exit:
  if (success)
    return reader;

  FREE(reader);
  return NULL;
}/*}}}*/

static void unshield_reader_destroy(UnshieldReader* reader)/*{{{*/
{
  if (reader)
  {
    FCLOSE(reader->volume_file);
    free(reader);
  }
}/*}}}*/

#define BUFFER_SIZE (64*1024+1)

/*
 * If filename is NULL, just throw away the result
 */
bool unshield_file_save (Unshield* unshield, int index, const char* filename)/*{{{*/
{
  bool success = false;
  FILE* output = NULL;
  unsigned char* input_buffer   = (unsigned char*)malloc(BUFFER_SIZE);
  unsigned char* output_buffer  = (unsigned char*)malloc(BUFFER_SIZE);
  int bytes_left;
  uLong total_written = 0;
  UnshieldReader* reader = NULL;
  FileDescriptor* file_descriptor;
	MD5_CTX md5;

	MD5Init(&md5);

  if (!unshield)
    goto exit;

  if (!(file_descriptor = unshield_get_file_descriptor(unshield, index)))
  {
    unshield_error("Failed to get file descriptor for file %i", index);
    goto exit;
  }

  if ((file_descriptor->flags & FILE_INVALID) || 0 == file_descriptor->data_offset)
  {
    /* invalid file */
    goto exit;
  }

  if (file_descriptor->link_flags & LINK_PREV)
  {
    success = unshield_file_save(unshield, file_descriptor->link_previous, filename);
    goto exit;
  }

  reader = unshield_reader_create(unshield, index, file_descriptor);
  if (!reader)
  {
    unshield_error("Failed to create data reader for file %i", index);
    goto exit;
  }

  if (unshield_fsize(reader->volume_file) == (long)file_descriptor->data_offset)
  {
    unshield_error("File %i is not inside the cabinet.", index);
    goto exit;
  }

  if (filename) 
  {
    output = fopen(filename, "w");
    if (!output)
    {
      unshield_error("Failed to open output file '%s'", filename);
      goto exit;
    }
  }

  if (file_descriptor->flags & FILE_COMPRESSED)
    bytes_left = file_descriptor->compressed_size;
  else
    bytes_left = file_descriptor->expanded_size;

  /*unshield_trace("Bytes to read: %i", bytes_left);*/

  while (bytes_left > 0)
  {
    uLong bytes_to_write = BUFFER_SIZE;
    int result;

    if (file_descriptor->flags & FILE_COMPRESSED)
    {
      uint16_t bytes_to_read = 0;

      if (!unshield_reader_read(reader, &bytes_to_read, sizeof(bytes_to_read)))
      {
#if VERBOSE
        unshield_error("Failed to read %i bytes of file %i (%s) from input cabinet file %i", 
            sizeof(bytes_to_read), index, unshield_file_name(unshield, index), file_descriptor->volume);
#endif
        goto exit;
      }

      bytes_to_read = letoh16(bytes_to_read);
      if (!unshield_reader_read(reader, input_buffer, bytes_to_read))
      {
#if VERBOSE
        unshield_error("Failed to read %i bytes of file %i (%s) from input cabinet file %i", 
            bytes_to_read, index, unshield_file_name(unshield, index), file_descriptor->volume);
#endif
        goto exit;
      }

      /* add a null byte to make inflate happy */
      input_buffer[bytes_to_read] = 0;
      result = unshield_uncompress(output_buffer, &bytes_to_write, input_buffer, bytes_to_read+1);

      if (Z_OK != result)
      {
        unshield_error("Decompression failed with code %i. bytes_to_read=%i, volume_bytes_left=%i, volume=%i", 
            result, bytes_to_read, reader->volume_bytes_left, file_descriptor->volume);
        goto exit;
      }

      bytes_left -= 2;
      bytes_left -= bytes_to_read;
    }
    else
    {
      bytes_to_write = MIN(bytes_left, BUFFER_SIZE);

      if (!unshield_reader_read(reader, output_buffer, bytes_to_write))
      {
#if VERBOSE
        unshield_error("Failed to read %i bytes from input cabinet file %i", 
            bytes_to_write, file_descriptor->volume);
#endif
        goto exit;
      }

      bytes_left -= bytes_to_write;
    }

    MD5Update(&md5, output_buffer, bytes_to_write);

    if (output)
    {
      if (bytes_to_write != fwrite(output_buffer, 1, bytes_to_write, output))
      {
        unshield_error("Failed to write %i bytes to file '%s'", bytes_to_write, filename);
        goto exit;
      }
    }

    total_written += bytes_to_write;
  }

  if (file_descriptor->expanded_size != total_written)
  {
    unshield_error("Expanded size expected to be %i, but was %i", 
        file_descriptor->expanded_size, total_written);
    goto exit;
  }

  if (unshield->header_list->major_version >= 6)
  {
    unsigned char md5result[16];
    MD5Final(md5result, &md5);

    if (0 != memcmp(md5result, file_descriptor->md5, 16))
    {
      unshield_error("MD5 checksum failure for file %i (%s)", 
          index, unshield_file_name(unshield, index));
      goto exit;
    }
  }

  success = true;
  
exit:
  unshield_reader_destroy(reader);
  FCLOSE(output);
  FREE(input_buffer);
  FREE(output_buffer);
  return success;
}/*}}}*/

int unshield_file_directory(Unshield* unshield, int index)/*{{{*/
{
  FileDescriptor* fd = unshield_get_file_descriptor(unshield, index);
  if (fd)
  {
    return fd->directory_index;
  }
  else
    return -1;
}/*}}}*/

size_t unshield_file_size(Unshield* unshield, int index)/*{{{*/
{
  FileDescriptor* fd = unshield_get_file_descriptor(unshield, index);
  if (fd)
  {
    return fd->expanded_size;
  }
  else
    return 0;
}/*}}}*/

