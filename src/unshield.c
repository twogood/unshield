/* $Id$ */
#define _BSD_SOURCE 1
#include <unshield.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static bool extract(Unshield* unshield, int index)
{
  bool success;
  char filename[256];
  char* p;

  snprintf(filename, sizeof(filename), "/var/tmp/unshield/%s", 
      unshield_file_name(unshield, index));

  for (p = filename; *p != '\0'; p++)
    if (!isprint(*p))
      *p = '_';

  printf("Writing %s...", filename);
  success = unshield_file_save(unshield, index, filename);
  printf("%s.\n", success ? "success" : "failure");

  return success;
}

static bool extract_all(Unshield* unshield)
{
  int i;
  int count = unshield_file_count(unshield);

  if (count < 0)
    return false;
  
  printf("%i files:\n", count);

  for (i = 0; i < count; i++)
  {
    extract(unshield, i);
  }

  return true;
}

static bool list(Unshield* unshield)
{
  int i;
  int count = unshield_file_count(unshield);

  if (count < 0)
    return false;
  
  printf("%i files:\n", count);

  for (i = 0; i < count; i++)
  {
    printf("%s\n", unshield_file_name(unshield, i)); 
  }

  return true;
}

int main(int argc, char** argv)
{
  int result = 1;
  Unshield* unshield = NULL;

  unshield = unshield_open(argv[1]);

  list(unshield);
  extract_all(unshield);
  
  unshield_close(unshield);

  return result;
}

