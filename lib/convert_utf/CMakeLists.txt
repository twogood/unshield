set(LIBCONVERT_UTF_HEADES
    "ConvertUTF.h"
)

set(LIBCONVERT_UTF_SOURCES
    "ConvertUTF.c"
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

add_library(convert_utf STATIC ${LIBCONVERT_UTF_HEADES} ${LIBCONVERT_UTF_SOURCES})
