set(MINIZ_HEADERS
    "miniz.h"
)

set(MINIZ_SOURCES
    "miniz.c"
)

if(BUILD_STATIC AND MSVC)
    set_msvc_runtime_static()
endif()

add_library(miniz STATIC ${MINIZ_HEADERS} ${MINIZ_SOURCES})
