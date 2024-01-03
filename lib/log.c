#include "log.h"
#include <stdarg.h>
#include <stdio.h>

/* evil static data */
static int current_log_level = UNSHIELD_LOG_LEVEL_HIGHEST;
static UnshieldLogCallback* current_log_handler = NULL;
static void* current_log_handler_userdata = NULL;

void unshield_set_log_level(int level)
{
	current_log_level = level;
}

void unshield_set_log_handler(UnshieldLogCallback* handler, void* userdata)
{
	current_log_handler = handler;
	current_log_handler_userdata = userdata;
}

void unshield_default_log_handler(void* userdata, int level, const char* file, int line, const char* format, va_list ap)
{
	(void)userdata;

	if (level > current_log_level)
		return;

	fprintf(stderr, "[%s:%i] ", file, line);
	
	vfprintf(stderr, format, ap);
	
	fprintf(stderr, "\n");
}

void _unshield_log(int level, const char* file, int line, const char* format, ...)
{
	va_list ap;

	if (current_log_handler == NULL)
		unshield_set_log_handler(unshield_default_log_handler, NULL);

	va_start(ap, format);
	current_log_handler(current_log_handler_userdata, level, file, line, format, ap);
	va_end(ap);
}
