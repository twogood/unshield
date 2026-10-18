#include <libunshield.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* Keep checks active in Release builds too. */
#define CHECK(condition) do { if (!(condition)) { \
  fprintf(stderr, "Line %d: %s\n", __LINE__, #condition); \
  exit(EXIT_FAILURE); } } while (0)

static void put32(uint8_t* p, uint32_t value)
{
  for (unsigned i = 0; i < 4; ++i)
    p[i] = (uint8_t)(value >> (8 * i));
}

static void write_file(const char* name, const void* data, size_t size)
{
  FILE* file = fopen(name, "wb");
  CHECK(file != NULL);
  CHECK(fwrite(data, 1, size, file) == size);
  CHECK(fclose(file) == 0);
}

static void common_header(uint8_t* p)
{
  put32(p, 0x28635349);
  put32(p + 4, 0x01005000); /* InstallShield 5 */
  put32(p + 12, 20);
  put32(p + 16, 0x276);
}

static void write_volume(const char* name, const uint8_t* data, size_t size,
    unsigned first, unsigned last)
{
  uint8_t cab[256] = {0};
  CHECK(size <= sizeof(cab) - 64);
  common_header(cab);
  put32(cab + 20, 64);
  put32(cab + 28, first);
  put32(cab + 32, last);
  put32(cab + 36, 64);
  put32(cab + 40, 5);
  put32(cab + 44, (uint32_t)size);
  put32(cab + 48, 64);
  put32(cab + 52, 5);
  put32(cab + 56, (uint32_t)size);
  memcpy(cab + 64, data, size);
  write_file(name, cab, 64 + size);
}

static size_t compressed_chunk(uint8_t* data)
{
  z_stream stream = {0};
  CHECK(deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
        -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) == Z_OK);
  stream.next_in = (Bytef*)"hello";
  stream.avail_in = 5;
  stream.next_out = data + 2;
  stream.avail_out = 100;
  CHECK(deflate(&stream, Z_FINISH) == Z_STREAM_END);
  size_t size = stream.total_out;
  CHECK(deflateEnd(&stream) == Z_OK);
  data[0] = (uint8_t)size;
  data[1] = (uint8_t)(size >> 8);
  return size + 2;
}

static void check_extract(bool expected)
{
  Unshield* unshield = unshield_open("data1.cab");
  CHECK(unshield != NULL);
  CHECK(unshield_file_count(unshield) == 2);
  CHECK(unshield_file_save(unshield, 1, "output") == expected);
  unshield_close(unshield);
  if (expected)
  {
    char output[11] = {0};
    FILE* file = fopen("output", "rb");
    CHECK(file != NULL);
    CHECK(fread(output, 1, sizeof(output), file) == 10);
    CHECK(memcmp(output, "hellohello", 10) == 0);
    CHECK(fclose(file) == 0);
  }
}

int main(void)
{
  uint8_t data[128] = {0};
  size_t chunk_size = compressed_chunk(data);
  memcpy(data + chunk_size, data, chunk_size);
  size_t size = chunk_size * 2;

  /* One directory, a dummy file and a split file; all offsets are relative
     to the cabinet descriptor or file table, as in an IS5 header. */
  uint8_t hdr[832] = {0};
  common_header(hdr);
  uint8_t* descriptor = hdr + 20;
  put32(descriptor + 12, 0x276);
  put32(descriptor + 20, 144);
  put32(descriptor + 24, 144);
  put32(descriptor + 28, 1);
  put32(descriptor + 40, 2);
  uint8_t* table = descriptor + 0x276;
  put32(table, 12); /* empty directory name */
  put32(table + 4, 13);
  put32(table + 8, 71);
  uint8_t* file = table + 71;
  put32(file, 129); /* file name */
  file[8] = 5; /* FILE_SPLIT | FILE_COMPRESSED */
  put32(file + 10, 10);
  put32(file + 14, (uint32_t)size);
  put32(file + 38, 64);
  memcpy(table + 129, "hello.txt", 10);
  write_file("data1.hdr", hdr, sizeof(hdr));

  /* The next read starts after the length field, after a complete chunk,
     or partway through a chunk (the pre-existing transition path). */
  size_t boundaries[] = {2, chunk_size, 3};
  for (unsigned i = 0; i < sizeof(boundaries) / sizeof(boundaries[0]); ++i)
  {
    size_t split = boundaries[i];
    write_volume("data1.cab", data, split, 1, 1);
    write_volume("data2.cab", data + split, size - split, 1, 1);
    check_extract(true);

    /* The next volume cannot supply this split file: fail, never spin. */
    write_volume("data2.cab", data + split, size - split, 0, 2);
    check_extract(false);
    CHECK(remove("data2.cab") == 0);
    check_extract(false);
  }

  /* IS5 can start the file in a later cabinet than its descriptor names. */
  write_volume("data1.cab", data, 2, 0, 0);
  write_volume("data2.cab", data, 2, 1, 1);
  write_volume("data3.cab", data + 2, size - 2, 1, 1);
  check_extract(true);
  CHECK(remove("data3.cab") == 0);
  CHECK(remove("data2.cab") == 0);

  write_volume("data1.cab", data, 2, 0, 2);
  check_extract(false);
  CHECK(remove("data1.hdr") == 0);
  CHECK(remove("data1.cab") == 0);
  CHECK(remove("output") == 0);
  return EXIT_SUCCESS;
}
