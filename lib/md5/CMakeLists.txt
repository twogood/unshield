set(LIBMD5_UTF_HEADES
	"global.h"
    "md5.h"
)

set(LIBMD5_UTF_SOURCES
    "md5c.c"
)

add_library(md5 ${LIBMD5_UTF_HEADES} ${LIBMD5_UTF_SOURCES})