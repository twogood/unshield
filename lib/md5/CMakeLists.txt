set(LIBMD5_UTF_HEADES
	"global.h"
    "md5.h"
)

set(LIBMD5_UTF_SOURCES
    "md5c.c"
)

if(BUILD_STATIC AND WIN32)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
endif(BUILD_STATIC AND WIN32)

add_library(md5 STATIC ${LIBMD5_UTF_HEADES} ${LIBMD5_UTF_SOURCES})
