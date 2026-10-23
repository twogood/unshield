#ifndef __log_h__
#define __log_h__


#ifdef __cplusplus
extern "C"
{
#endif

void _unshield_set_log_level(int level);

void _unshield_log(int level, const char* file, int line, const char* format, ...);

#define unshield_trace(format, ...) \
	_unshield_log(UNSHIELD_LOG_LEVEL_TRACE, __func__, __LINE__, format __VA_OPT__(,) __VA_ARGS__)

#define unshield_warning(format, ...) \
	_unshield_log(UNSHIELD_LOG_LEVEL_WARNING, __func__, __LINE__, format __VA_OPT__(,) __VA_ARGS__)

#define unshield_error(format, ...) \
	_unshield_log(UNSHIELD_LOG_LEVEL_ERROR, __func__, __LINE__, format __VA_OPT__(,) __VA_ARGS__)

#ifdef __cplusplus
}
#endif


#endif

