/* $Id */
#include "log.h"
#include <stdarg.h>
#include <stdio.h>

/* evil static data */
static int current_log_level = UNSHIELD_LOG_LEVEL_HIGHEST;

void unshield_set_log_level(int level)
{
	current_log_level = level;
}

void _unshield_log(int level, const char* file, int line, const char* format, ...)
{
	va_list ap;

	if (level > current_log_level)
		return;

	fprintf(stderr, "[%s:%i] ", file, line);
	
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	
	fprintf(stderr, "\n");
}

