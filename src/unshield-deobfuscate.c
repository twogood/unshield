#include <stdio.h>
#include <stdlib.h>

#include "../lib/libunshield.h"

int main(int argc, char** argv)
{
  unsigned seed = 0;
  FILE* input = NULL;
  FILE* output = NULL;
  size_t size;
  unsigned char buffer[16384];

  if (argc != 3)
  {
    fprintf(stderr, 
        "Syntax:\n"
        "  %s INPUT-FILE OUTPUT-FILE\n",
        argv[0]);
    exit(1);
  }

  input = fopen(argv[1], "rb");
  if (!input)
  {
    fprintf(stderr, 
        "Failed to open %s for reading\n",
        argv[1]);
    exit(2);
  }

  output = fopen(argv[2], "wb");
  if (!output)
  {
    fprintf(stderr, 
        "Failed to open %s for writing\n",
        argv[2]);
    exit(3);
  }


  while ((size = fread(buffer, 1, sizeof(buffer), input)) != 0)
  {
    unshield_deobfuscate(buffer, size, &seed);
    if (fwrite(buffer, 1, size, output) != size)
    {
      fprintf(stderr, 
          "Failed to write %lu bytes to %s\n",
          (unsigned long)size, argv[2]);
      exit(4);
    }
  }

  fclose(input);
  fclose(output);
  return 0;
}
