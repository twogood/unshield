/* $Id$ */
#define _BSD_SOURCE 1
#include <unshield.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static bool make_sure_directory_exists(const char* directory)/*{{{*/
{
	struct stat dir_stat;

	/*
	 * Make sure that this directory exists
	 */

	if (stat(directory, &dir_stat) < 0)
	{
		if (mkdir(directory, 0700) < 0)
		{
			fprintf(stderr, "Failed to create directory %s\n", directory);
			return false;
		}
	}

	return true;
}/*}}}*/

static bool extract(Unshield* unshield, int index)
{
  bool success;
  char dirname[256];
  char filename[256];
  char* p;
  int directory = unshield_file_directory(unshield, index);

  if (directory >= 0)
    snprintf(dirname, sizeof(dirname), "/var/tmp/unshield/%s", 
      unshield_directory_name(unshield, directory));
  else
    strcpy(dirname, "/var/tmp/unshield");

  for (p = dirname; *p != '\0'; p++)
    if ('\\' == *p)
      *p = '/';

  make_sure_directory_exists(dirname);

  snprintf(filename, sizeof(filename), "%s/%s", 
      dirname, unshield_file_name(unshield, index));

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

static bool list_directories(Unshield* unshield)
{
  int i;
  int count = unshield_directory_count(unshield);

  if (count < 0)
    return false;
  
  printf("%i directories:\n", count);

  for (i = 0; i < count; i++)
  {
    printf("%s\n", unshield_directory_name(unshield, i)); 
  }

  return true;
}

static bool list_files(Unshield* unshield)
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

  list_directories(unshield);
  list_files(unshield);
  extract_all(unshield);
  
  unshield_close(unshield);

  return result;
}

