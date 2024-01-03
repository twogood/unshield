#ifndef __log_h__
#define __log_h__

#include "internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

void _unshield_log(int level, const char* file, int line, const char* format, ...);

#define unshield_trace(format, ...) \
	_unshield_log(UNSHIELD_LOG_LEVEL_TRACE,__FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define unshield_warning(format, ...) \
	_unshield_log(UNSHIELD_LOG_LEVEL_WARNING,__FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define unshield_error(format, ...) \
	_unshield_log(UNSHIELD_LOG_LEVEL_ERROR,__FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif


#endif

