/* $Id$ */
#define _BSD_SOURCE 1
#define _POSIX_C_SOURCE 2
#include <libunshield.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum 
{
  OVERWRITE_ASK,
  OVERWRITE_NEVER,
  OVERWRITE_ALWAYS,
} OVERWRITE;

typedef enum
{
  ACTION_EXTRACT,
  ACTION_LIST
} ACTION;

static const char* output_directory   = ".";
static bool junk_paths                = false;
static bool make_lowercase            = false;
static bool verbose                   = false;
static ACTION action                  = ACTION_EXTRACT;
static OVERWRITE overwrite            = OVERWRITE_ASK;

static bool make_sure_directory_exists(const char* directory)/*{{{*/
{
	struct stat dir_stat;
  const char* p = directory;

  while (p && *p)
  {
    if ('/' == *p)
      p++;
    else if (0 == strncmp(p, "./", 2))
      p+=2;
    else if (0 == strncmp(p, "../", 3))
      p+=3;
    else
    {
      char* current = strdup(directory);
      const char* slash = strchr(p, '/');
      
      if (slash)
        current[slash-directory] = '\0';

      if (stat(current, &dir_stat) < 0)
      {
        if (mkdir(current, 0700) < 0)
        {
          fprintf(stderr, "Failed to create directory %s\n", directory);
          return false;
        }
      }

      p = slash;
    }
  }

	return true;
}/*}}}*/

static void show_usage(const char* name)
{
  fprintf(stderr,
      "Syntax:\n"
      "\n"
      "\t%s [-d DIRECTORY] [-D LEVEL] [-h] [-l] CABFILE\n"
      "\n"
      "\t-d DIRECTORY  Extract files to DIRECTORY\n"
      "\t-D LEVEL      Set debug log level\n"
      "\t                0 - No logging (default)\n"
      "\t                1 - Errors only\n"
      "\t                2 - Errors and warnings\n"
      "\t                3 - Everything\n"
      "\t-h            Show this help message\n"
      "\t-j            Junk paths (do not make directories)\n"
      "\t-l            List files (default action is to extract)\n"
      "\t-L            Make file and directory names lowercase\n"
      "\tCABFILE      The file to list or extract contents of\n"
      ,
      name);

#if 0
      "\t-n            Never overwrite files\n"
      "\t-o            Overwrite files WITHOUT prompting\n"
      "\t-v            Verbose output\n"
#endif
}

static bool handle_parameters(
    int argc, 
    char** argv, 
    int* end_optind)
{
	int c;
	int log_level = UNSHIELD_LOG_LEVEL_LOWEST;

	while ((c = getopt(argc, argv, "d:D:hjlLno")) != -1)
	{
		switch (c)
		{
			case 'd':
				output_directory = optarg;
				break;
				
			case 'D':
				log_level = atoi(optarg);
				break;
       
      case 'j':
        junk_paths = true;
        break;

      case 'l':
        action = ACTION_LIST;
        break;

      case 'L':
        make_lowercase = true;
        break;

      case 'n':
        overwrite = OVERWRITE_NEVER;
        break;

      case 'o':
        overwrite = OVERWRITE_ALWAYS;
        break;

      case 'v':
        verbose = true;
        break;

			case 'h':
			default:
				show_usage(argv[0]);
				return false;
		}
	}

	unshield_set_log_level(log_level);

  *end_optind = optind;

	return true;
}

static bool extract(Unshield* unshield, int index)
{
  bool success;
  char dirname[256];
  char filename[256];
  char* p;
  int directory = unshield_file_directory(unshield, index);

  if (!junk_paths && directory >= 0)
    snprintf(dirname, sizeof(dirname), "%s/%s", 
        output_directory,
        unshield_directory_name(unshield, directory));
  else
    strcpy(dirname, output_directory);

  for (p = dirname + strlen(output_directory); *p != '\0'; p++)
  {
    if ('\\' == *p)
      *p = '/';
    else if (make_lowercase)
      *p = tolower(*p);
  }

  if (dirname[strlen(dirname)-1] != '/')
    strcat(dirname, "/");

  make_sure_directory_exists(dirname);

  snprintf(filename, sizeof(filename), "%s%s", 
      dirname, unshield_file_name(unshield, index));

  for (p = filename; *p != '\0'; p++)
  {
    if (!isprint(*p))
      *p = '_';
    else if (make_lowercase)
      *p = tolower(*p);
  }

  printf("  extracting: %s\n", filename);
  success = unshield_file_save(unshield, index, filename);

  return success;
}

static bool extract_all(Unshield* unshield)
{
  int i;
  int count = unshield_file_count(unshield);

  if (count < 0)
    return false;
  
  for (i = 0; i < count; i++)
  {
    if (unshield_file_is_valid(unshield, i))
      extract(unshield, i);
  }

  return true;
}

static bool list_files(Unshield* unshield)
{
  int i;
  int count = unshield_file_count(unshield);
  int valid_count = 0;

  if (count < 0)
    return false;
  
  for (i = 0; i < count; i++)
  {
    char dirname[256];

    if (unshield_file_is_valid(unshield, i))
    {
      valid_count++;

      strcpy(dirname,
          unshield_directory_name(unshield, unshield_file_directory(unshield, i)));

#if 0
      for (p = dirname + strlen(output_directory); *p != '\0'; p++)
        if ('\\' == *p)
          *p = '/';
#endif

      if (dirname[0])
        strcat(dirname, "\\");

      printf("%s%s\n",
          dirname,
          unshield_file_name(unshield, i)); 
    }
  }

  printf("-------\n%i files\n", valid_count);


  return true;
}

int main(int argc, char** argv)
{
  bool success = false;
  Unshield* unshield = NULL;
  int last_optind = argc+1;
  const char* cabfile;

  if (!handle_parameters(argc, argv, &last_optind))
    goto exit;

  if (last_optind >= argc)
  {
    fprintf(stderr, "Missing cabinet file on command line\n");
    show_usage(argv[0]);
    goto exit;
  }

  cabfile = argv[last_optind];

  unshield = unshield_open(cabfile);
  if (!unshield)
  {
    fprintf(stderr, "Failed to open %s as an InstallShield Cabinet File\n", cabfile);
    goto exit;
  }

  printf("Cabinet: %s\n", cabfile);

  switch (action)
  {
    case ACTION_EXTRACT:
      success = extract_all(unshield);
      break;

    case ACTION_LIST:
      success = list_files(unshield);
      break;
  }

exit:
  unshield_close(unshield);
  return success ? 0 : 1;
}

