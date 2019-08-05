set(LIBMD5_UTF_HEADES
	"global.h"
    "md5.h"
)

set(LIBMD5_UTF_SOURCES
    "md5c.c"
)

if(BUILD_STATIC AND MSVC)
    set_msvc_runtime_static()
endif()

add_library(md5 STATIC ${LIBMD5_UTF_HEADES} ${LIBMD5_UTF_SOURCES})
