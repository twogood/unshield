/* $Id$ */
#ifdef __linux__
#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#define _POSIX_C_SOURCE 2
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h> /* getopt at: https://gist.github.com/ashelly/7776712 */
#include "../lib/libunshield.h"
#ifdef HAVE_CONFIG_H
#include "lib/unshield_config.h"
#endif
#if HAVE_FNMATCH
#include <fnmatch.h>
#endif
#ifdef HAVE_ICONV
#include <iconv.h>
#include <errno.h>
#endif

#ifndef VERSION
#define VERSION "Unknown"
#endif

#define FREE(ptr)       { if (ptr) { free(ptr); ptr = NULL; } }

#ifdef _WIN32
  #define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
  #include <direct.h>
  #ifndef PATH_MAX
    #define PATH_MAX _MAX_PATH
  #endif
#else
  #include <limits.h>
#endif

#ifndef NAME_MAX
  #define NAME_MAX FILENAME_MAX
#endif

typedef enum 
{
  OVERWRITE_ASK,
  OVERWRITE_NEVER,
  OVERWRITE_ALWAYS,
} OVERWRITE;

typedef enum
{
  ACTION_EXTRACT,
  ACTION_LIST_COMPONENTS,
  ACTION_LIST_FILE_GROUPS,
  ACTION_LIST_FILES,
  ACTION_TEST
} ACTION;

typedef enum
{
  FORMAT_NEW,
  FORMAT_OLD,
  FORMAT_RAW
} FORMAT;

#define DEFAULT_OUTPUT_DIRECTORY  "."

static const char* output_directory   = DEFAULT_OUTPUT_DIRECTORY;
static const char* file_group_name    = NULL;
static const char* component_name     = NULL;
static bool junk_paths                = false;
static bool make_lowercase            = false;
static bool raw_filename              = false;
static bool verbose                   = false;
static ACTION action                  = ACTION_EXTRACT;
static OVERWRITE overwrite            = OVERWRITE_ASK;
static int log_level                  = UNSHIELD_LOG_LEVEL_LOWEST;
static int exit_status                = 0;
static FORMAT format                  = FORMAT_NEW;
static int is_version                 = -1;
static const char* cab_file_name      = NULL;
static char* const* path_names        = NULL;
static int path_name_count            = 0;
#ifdef HAVE_ICONV
static const char* encoding           = NULL;
iconv_t encoding_descriptor           = (iconv_t)-1;
#endif

static bool make_sure_directory_exists(const char* directory)/*{{{*/
{
	struct stat dir_stat;
  const char* p = directory;
  bool success = false;
  char* current = NULL;

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
      const char* slash = strchr(p, '/');
      current = strdup(directory);
      
      if (slash)
        current[slash-directory] = '\0';

      if (stat(current, &dir_stat) < 0)
      {
        #ifdef __MINGW32__
        if (_mkdir(current) < 0)
        #else
        if (mkdir(current, 0700) < 0)
        #endif
        {
          fprintf(stderr, "Failed to create directory %s\n", directory);
          if(strlen(directory)>NAME_MAX)
            fprintf(stderr, "Directory name must be less than %i characters\n", NAME_MAX+1);
          goto exit;
        }
      }

      p = slash;
      FREE(current);
    }
  }

  success = true;

exit:
  FREE(current);
  return success;
}/*}}}*/

#ifdef HAVE_ICONV
static bool convert_encoding(char *buffer, size_t size)
{
  bool success = false;
  char *newbuf, *inbuf, *outbuf;
  size_t inbytesleft, outbytesleft, newsize;

  if (encoding_descriptor == (iconv_t)-1)
    return true;

  inbuf = buffer;
  inbytesleft = strlen(buffer);
  newbuf = outbuf = malloc(size);
  outbytesleft = size - 1;

  if (iconv(encoding_descriptor,
          &inbuf, &inbytesleft,
          &outbuf, &outbytesleft) == (size_t)-1)
  {
    fprintf(stderr, "Could not encode text to '%s' error %s\n",
        encoding, strerror(errno));
    goto exit;
  }

  newsize = (size_t)(outbuf - newbuf);
  memcpy(buffer, newbuf, newsize);
  buffer[newsize] = '\0';

  success = true;

exit:
  free(newbuf);
  return success;
}
#endif

static void show_usage(const char* name)
{
  fprintf(stderr,
      "Syntax:\n"
      "\n"
      "\t%s [-c COMPONENT] [-d DIRECTORY] [-D LEVEL] [-g GROUP] [-i VERSION] [-e ENCODING] [-GhlOrV] c|g|l|t|x CABFILE [FILENAME...]\n"
      "\n"
      "Options:\n"
      "\t-c COMPONENT  Only list/extract this component\n"
      "\t-d DIRECTORY  Extract files to DIRECTORY\n"
      "\t-D LEVEL      Set debug log level\n"
      "\t                0 - No logging (default)\n"
      "\t                1 - Errors only\n"
      "\t                2 - Errors and warnings\n"
      "\t                3 - Errors, warnings and debug messages\n"
      "\t-g GROUP      Only list/extract this file group\n"
      "\t-h            Show this help message\n"
      "\t-i VERSION    Force InstallShield version number (don't autodetect)\n"
      "\t-e ENCODING   Convert filename character encoding to local codepage from ENCODING (implicitly sets -R)\n"
      "\t-j            Junk paths (do not make directories)\n"
      "\t-L            Make file and directory names lowercase\n"
      "\t-O            Use old compression\n"
      "\t-r            Save raw data (do not decompress)\n"
      "\t-R            Don't do any conversion to file and directory names when extracting.\n"
      "\t-V            Print copyright and version information\n"
      "\n"
      "Commands:\n"
      "\tc             List components\n"         
      "\tg             List file groups\n"         
      "\tl             List files\n"
      "\tt             Test files\n"
      "\tx             Extract files\n"
      "\n"
      "Other:\n"
      "\tCABFILE       The file to list or extract contents of\n"
      "\tFILENAME...   Optionally specify names of specific files to extract"
#if HAVE_FNMATCH
      " (wildcards are supported)"
#endif
      "\n"
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
    char* const argv[])
{
	int c;

	while ((c = getopt(argc, argv, "c:d:D:g:hi:e:jLnoOrRV")) != -1)
	{
		switch (c)
    {
      case 'c':
        component_name = optarg;
        break;

      case 'd':
        output_directory = optarg;
        break;

      case 'D':
        log_level = atoi(optarg);
        break;

      case 'g':
        file_group_name = optarg;
        break;

      case 'i':
        is_version = atoi(optarg);
        break;

      case 'e':
#ifdef HAVE_ICONV
        encoding = optarg;
        raw_filename = true;
#else
        fprintf(stderr, "This version of Unshield is not built with encoding support.\n");
        return false;
#endif
        break;

      case 'j':
        junk_paths = true;
        break;

      case 'L':
        make_lowercase = true;
        break;
        
      case 'R':
        raw_filename = true;
        break;

      case 'n':
        overwrite = OVERWRITE_NEVER;
        break;

      case 'o':
        overwrite = OVERWRITE_ALWAYS;
        break;

      case 'O':
        format = FORMAT_OLD;
        break;
        
      case 'r':
        format = FORMAT_RAW;
        break;

      case 'v':
        verbose = true;
        break;

      case 'V':
        printf("Unshield version " VERSION ". Copyright (C) 2003-2013 David Eriksson.\n");
        exit(0);
        break;

      case 'h':
      default:
        show_usage(argv[0]);
        return false;
    }
	}

	unshield_set_log_level(log_level);

  if (optind == argc || !argv[optind])
  {
    fprintf(stderr, "No action provided on command line.\n\n");
    show_usage(argv[0]);
    return false;
  }

  char action_char = argv[optind++][0];
  switch (action_char)
  {
    case 'c':
      action = ACTION_LIST_COMPONENTS;
      break;

    case 'g':
      action = ACTION_LIST_FILE_GROUPS;
      break;

    case 'l':
      action = ACTION_LIST_FILES;
      break;

    case 't':
      action = ACTION_TEST;
      break;

    case 'x':
      action = ACTION_EXTRACT;
      break;

    default:
      fprintf(stderr, "Unknown action '%c' on command line.\n\n", action_char);
      show_usage(argv[0]);
      return false;
  }

  cab_file_name = argv[optind++];

  if (cab_file_name == NULL)
  {
    fprintf(stderr, "No InstallShield Cabinet File name provided on command line.\n\n");
    show_usage(argv[0]);
    return false;
  }

  path_name_count = argc - optind;
  path_names = &argv[optind];

	return true;
}

static bool extract_file(Unshield* unshield, const char* prefix, int index)
{
  bool success;
  char* dirname;
  char* filename;
  char* p;
  int directory = unshield_file_directory(unshield, index);
  long int path_max;
  char* real_output_directory;
  char* real_filename;

  #ifdef PATH_MAX
    path_max = PATH_MAX;
  #else
    path_max = pathconf(prefix, _PC_PATH_MAX);
    if (path_max <= 0)
      path_max = 4096;
  #endif

  real_output_directory = malloc(path_max);
  real_filename = malloc(path_max);
  dirname = malloc(path_max);
  filename = malloc(path_max);
  if (real_output_directory == NULL || real_filename == NULL)
  {
    fprintf(stderr,"Unable to allocate memory.");
    success=false;
    goto exit;
  }


  if(strlen(output_directory) < path_max-1)
  {
    strncpy(dirname, output_directory,path_max-1);
    if (path_max > 0)
      dirname[path_max - 1]= '\0';
    strcat(dirname, "/");
  }
  else
  {
    fprintf(stderr, "\nOutput directory exceeds maximum path length.\n");
    success = false;
    goto exit;
  }


  if (prefix && prefix[0])
  {
    if(strlen(dirname)+strlen(prefix) < path_max-1)
    {
      strcat(dirname, prefix);
      strcat(dirname, "/");
    }
    else
    {
    fprintf(stderr, "\nOutput directory exceeds maximum path length.\n");
    success = false;
    goto exit;
    }
  }

  if (!junk_paths && directory >= 0)
  {
    const char* tmp = unshield_directory_name(unshield, directory);
    if (tmp && tmp[0])
    {
      if(strlen(dirname)+strlen(tmp) < path_max-1)
      {
        strcat(dirname, tmp);
        strcat(dirname, "/");
      }
      else
      {
      fprintf(stderr, "\nOutput directory exceeds maximum path length.\n");
      success = false;
      goto exit;
      }
    }
  }

  for (p = dirname + strlen(output_directory); *p != '\0'; p++)
  {
    switch (*p)
    {
      case '\\':
        *p = '/';
        break;

      case ' ':
      case '<':
      case '>':
      case '[':
      case ']':
        *p = '_';
        break;

      default:
        if (!raw_filename)
        {  
          if (!isprint(*p))
            *p = '_';
          else if (make_lowercase)
            *p = tolower(*p);
        }
        break;;
    }
  }

#ifdef HAVE_ICONV
  if (!convert_encoding(dirname, path_max))
  {
    success = false;
    goto exit;
  }
#endif


#if 0
  if (dirname[strlen(dirname)-1] != '/')
    strcat(dirname, "/");
#endif

  make_sure_directory_exists(dirname);

  snprintf(filename, path_max, "%s%s", 
      dirname, unshield_file_name(unshield, index));

  for (p = filename + strlen(dirname); *p != '\0'; p++)
  {
    if (!raw_filename)
    {
      if (!isprint(*p))
        *p = '_';
      else if (make_lowercase)
        *p = tolower(*p);
    }
  }

#ifdef HAVE_ICONV
  if (!convert_encoding(filename + strlen(dirname),
      path_max - strlen(dirname)))
  {
    success = false;
    goto exit;
  }
#endif

#ifdef __GLIBC__
  /* use GNU extension to return non-existing files to real_output_directory */
  realpath(output_directory, real_output_directory);
  realpath(filename, real_filename);
  if (real_filename == NULL || strncmp(real_filename,
                                       real_output_directory,
                                       strlen(real_output_directory)) != 0)
  {
    fprintf(stderr, "\n\nExtraction failed.\n");
    fprintf(stderr, "Error: %s (%d).\n", strerror(errno), errno);
    fprintf(stderr, "Possible directory traversal attack for: %s\n", filename);
    fprintf(stderr, "To be placed at: %s\n\n", real_filename);
    success = false;
    goto exit;
  }
#endif

  printf("  extracting: %s\n", filename);
  switch (format)
  {
    case FORMAT_NEW:
      success = unshield_file_save(unshield, index, filename);
      break;
    case FORMAT_OLD:
      success = unshield_file_save_old(unshield, index, filename);
      break;
    case FORMAT_RAW:
      success = unshield_file_save_raw(unshield, index, filename);
      break;
  }

exit:
  if (!success)
  {
    fprintf(stderr, "Failed to extract file '%s'.%s\n",
        unshield_file_name(unshield, index),
        (log_level < 3) ? "Run unshield again with -D 3 for more information." : "");
    unlink(filename);
    exit_status = 1;
  }
  free(real_filename);
  free(real_output_directory);
  free(dirname);
  free(filename);
  return success;
}

static bool should_process_file(Unshield* unshield, int index)
{
  int i;

  if (path_name_count == 0)
    return true;

  for (i = 0; i < path_name_count; i++)
  {
#if HAVE_FNMATCH
    if (fnmatch(path_names[i], unshield_file_name(unshield, index), 0) == 0)
      return true;
#else
    if (strcmp(path_names[i], unshield_file_name(unshield, index)) == 0)
      return true;
#endif
  }

  return false;
}

static int extract_helper(Unshield* unshield, const char* prefix, int first, int last)/*{{{*/
{
  int i;
  int count = 0;
  
  for (i = first; i <= last; i++)
  {
    if (unshield_file_is_valid(unshield, i) 
        && should_process_file(unshield, i) 
        && extract_file(unshield, prefix, i))
      count++;
  }

  return count;
}/*}}}*/

static bool test_file(Unshield* unshield, int index)
{
  bool success;

  printf("  testing: %s\n", unshield_file_name(unshield, index));

    switch (format)
    {
        case FORMAT_NEW:
            success = unshield_file_save(unshield, index, NULL);
            break;
        case FORMAT_OLD:
            success = unshield_file_save_old(unshield, index, NULL);
            break;
        case FORMAT_RAW:
            success = unshield_file_save_raw(unshield, index, NULL);
            break;
    }

  if (!success)
  {
    fprintf(stderr, "Failed to extract file '%s'.%s\n", 
        unshield_file_name(unshield, index),
        (log_level < 3) ? "Run unshield again with -D 3 for more information." : "");
    exit_status = 1;
  }

  return success;
}

static int test_helper(Unshield* unshield, const char* prefix, int first, int last)/*{{{*/
{
  int i;
  int count = 0;
  
  for (i = first; i <= last; i++)
  {
    if (unshield_file_is_valid(unshield, i) && test_file(unshield, i))
      count++;
  }

  return count;
}/*}}}*/

static bool list_components(Unshield* unshield)
{
  int i;
  int count = unshield_component_count(unshield);

  if (count < 0)
    return false;
  
  for (i = 0; i < count; i++)
  {
    printf("%s\n", unshield_component_name(unshield, i));
  }

  printf("-------\n%i components\n", count);


  return true;
}

static bool list_file_groups(Unshield* unshield)
{
  int i;
  int count = unshield_file_group_count(unshield);

  if (count < 0)
    return false;
  
  for (i = 0; i < count; i++)
  {
    printf("%s\n", unshield_file_group_name(unshield, i));
  }

  printf("-------\n%i file groups\n", count);


  return true;
}

static int list_files_helper(Unshield* unshield, const char* prefix, int first, int last)/*{{{*/
{
  int i;
  int valid_count = 0;

  for (i = first; i <= last; i++)
  {
    char dirname[4096];

    if (unshield_file_is_valid(unshield, i) && should_process_file(unshield, i))
    {
      valid_count++;

      if (prefix && prefix[0])
      {
        strcpy(dirname, prefix);
        strcat(dirname, "\\");
      }
      else
        dirname[0] = '\0';

      strcat(dirname,
          unshield_directory_name(unshield, unshield_file_directory(unshield, i)));

#if 0
      for (p = dirname + strlen(output_directory); *p != '\0'; p++)
        if ('\\' == *p)
          *p = '/';
#endif

      if (dirname[strlen(dirname)-1] != '\\')
        strcat(dirname, "\\");

      printf(" %8" SIZE_FORMAT "  %s%s\n",
          unshield_file_size(unshield, i),
          dirname,
          unshield_file_name(unshield, i)); 
    }
  }

  return valid_count;
}/*}}}*/

typedef int (*ActionHelper)(Unshield* unshield, const char* prefix, int first, int last);

static bool do_action(Unshield* unshield, ActionHelper helper)
{
  int count = 0;

  if (component_name)
  {
    fprintf(stderr, "This action is not implemented for components, sorry! Patch welcome!\n");
    return false;
  }
  else if (file_group_name)
  {
    UnshieldFileGroup* file_group = unshield_file_group_find(unshield, file_group_name);
    printf("File group: %s\n", file_group_name);
    if (file_group)
      count = helper(unshield, file_group_name, file_group->first_file, file_group->last_file);
  }
  else
  {
    int i;

    for (i = 0; i < unshield_file_group_count(unshield); i++)
    {
      UnshieldFileGroup* file_group = unshield_file_group_get(unshield, i);
      if (file_group)
        count += helper(unshield, file_group->name, file_group->first_file, file_group->last_file);
    }
  }

  printf(" --------  -------\n          %i files\n", count);

  return true;
}

int main(int argc, char* const argv[])
{
  bool success = false;
  Unshield* unshield = NULL;

  setlocale(LC_ALL, "");

  if (!handle_parameters(argc, argv))
    goto exit;

  unshield = unshield_open_force_version(cab_file_name, is_version);
  if (!unshield)
  {
    fprintf(stderr, "Failed to open %s as an InstallShield Cabinet File\n", cab_file_name);
    goto exit;
  }

#ifdef HAVE_ICONV
  if (!unshield_is_unicode(unshield) && encoding != NULL)
  {
    if ((encoding_descriptor = iconv_open("", encoding)) == (iconv_t)-1)
    {
      fprintf(stderr, "Cannot use encoding '%s' error %s\n",
          encoding, strerror(errno));
      goto exit;
    }
  }
#endif

  printf("Cabinet: %s\n", cab_file_name);

  switch (action)
  {
    case ACTION_EXTRACT:
      success = do_action(unshield, extract_helper);
      break;

    case ACTION_LIST_COMPONENTS:
      success = list_components(unshield);
      break;

    case ACTION_LIST_FILE_GROUPS:
      success = list_file_groups(unshield);
      break;

    case ACTION_LIST_FILES:
      success = do_action(unshield, list_files_helper);
      break;
      
    case ACTION_TEST:
      if (strcmp(output_directory, DEFAULT_OUTPUT_DIRECTORY) != 0)
        fprintf(stderr, "Output directory (-d) option has no effect with test (t) command.\n");
      if (make_lowercase)
        fprintf(stderr, "Make lowercase (-L) option has no effect with test (t) command.\n");
      success = do_action(unshield, test_helper);
      break;
  }

exit:
  unshield_close(unshield);
#ifdef HAVE_ICONV
  if (encoding_descriptor != (iconv_t)-1)
    iconv_close(encoding_descriptor);
#endif
  if (!success)
    exit_status = 1;
  return exit_status;
}

