set(LIBMD5_HEADERS
    global.h
    md5.h
)

set(LIBMD5_SOURCES
    md5c.c
)

add_library(md5 OBJECT ${LIBMD5_HEADERS} ${LIBMD5_SOURCES})

set_target_properties(md5 PROPERTIES
    C_VISIBILITY_PRESET hidden
)
