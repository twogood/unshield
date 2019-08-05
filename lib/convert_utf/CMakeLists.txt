set(LIBCONVERT_UTF_HEADES
    "ConvertUTF.h"
)

set(LIBCONVERT_UTF_SOURCES
    "ConvertUTF.c"
)

if(BUILD_STATIC AND MSVC)
    set_msvc_runtime_static()
endif()

add_library(convert_utf STATIC ${LIBCONVERT_UTF_HEADES} ${LIBCONVERT_UTF_SOURCES})
