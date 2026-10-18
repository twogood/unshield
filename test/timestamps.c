#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "libunshield.h"

static void put32(uint8_t* p, uint32_t value)
{
  for (int i = 0; i < 4; i++)
    p[i] = (uint8_t)(value >> (8 * i));
}

static void write_file(const char* name, const uint8_t* data, size_t size)
{
  FILE* file = fopen(name, "wb");
  assert(file);
  assert(fwrite(data, 1, size, file) == size);
  assert(fclose(file) == 0);
}

int main(int argc, char** argv)
{
  assert(argc >= 3);
  if (strcmp(argv[1], "prepare") == 0)
  {
    /* Minimal IS5 archive: one uncompressed file in one file group. */
    uint8_t header[0x400] = {0};
    uint8_t cabinet[0x45] = {0};
    uint8_t* descriptor = header + 0x2a8;
    Unshield* archive;
    char date[32];
    assert(argc == 6);
    put32(header, 0x28635349);
    put32(header + 4, 0x01005000);
    put32(header + 12, 0x10);
    put32(header + 16, sizeof(header) - 0x10);
    put32(header + 0x1c, 0x290); /* file table, relative to cabinet descriptor */
    put32(header + 0x24, 8);
    put32(header + 0x28, 8);
    put32(header + 0x2c, 1); /* directory count */
    put32(header + 0x38, 1); /* file count */
    put32(header + 0x4e, 0x350); /* file group offset list */
    put32(header + 0x2a0, 0x60); /* directory name */
    put32(header + 0x2a4, 8); /* file descriptor */
    put32(descriptor, 0x50); /* file name */
    put32(descriptor + 10, 5);
    put32(descriptor + 14, 5);
    put32(descriptor + 22, (uint32_t)strtoul(argv[2], NULL, 0));
    put32(descriptor + 26, (uint32_t)strtoul(argv[3], NULL, 0));
    put32(descriptor + 38, 0x40); /* data offset */
    memcpy(header + 0x2f0, "test.txt", 9);
    memcpy(header + 0x300, "files", 6);
    put32(header + 0x310, 0x370); /* group name */
    put32(header + 0x360, 0x370);
    put32(header + 0x364, 0x300); /* group descriptor */
    memcpy(header + 0x380, "group", 6);
    memcpy(cabinet, header, 20);
    put32(cabinet + 36, 0x40);
    put32(cabinet + 40, 5);
    put32(cabinet + 44, 5);
    put32(cabinet + 48, 0x40);
    put32(cabinet + 52, 5);
    put32(cabinet + 56, 5);
    memcpy(cabinet + 0x40, "TEST", 5);
    write_file("data1.hdr", header, sizeof(header));
    write_file("data1.cab", cabinet, sizeof(cabinet));
    archive = unshield_open("data1.cab");
    assert(archive);
    if (strcmp(argv[4], "missing") == 0)
    {
      assert(!unshield_file_iso_date(archive, 0, date, sizeof(date)));
      assert(date[0] == '\0');
      assert(unshield_file_timestamp(archive, 0) == 0);
    }
    else
    {
      assert(unshield_file_iso_date(archive, 0, date, sizeof(date)));
      assert(strcmp(date, argv[4]) == 0);
      assert(unshield_file_timestamp(archive, 0) == (time_t)strtoll(argv[5], NULL, 10));
    }
    unshield_close(archive);
  }
  else
  {
    struct stat info;
    char content[5];
    FILE* file = fopen(argv[2], "rb");
    assert(argc == 4);
    assert(file);
    assert(fread(content, 1, sizeof(content), file) == sizeof(content));
    assert(memcmp(content, "TEST", sizeof(content)) == 0);
    assert(fclose(file) == 0);
    assert(stat(argv[2], &info) == 0);
    if (strcmp(argv[3], "current") != 0)
      assert(info.st_mtime == (time_t)strtoll(argv[3], NULL, 10));
    else
      assert(difftime(time(NULL), info.st_mtime) >= -2 &&
             difftime(time(NULL), info.st_mtime) < 60);
  }
  return 0;
}
