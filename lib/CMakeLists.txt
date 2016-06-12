add_subdirectory(convert_utf)

	
if(USE_OUR_OWN_MD5)
    add_subdirectory(md5)
endif()

set(LIBUNSHIELD_HEADES
    "internal.h"
    "libunshield.h"
    "log.h"
    "cabfile.h"
)

set(LIBUNSHIELD_SOURCES
    "bswap.c"
    "component.c"
    "directory.c"
    "file.c"
    "file_group.c"
    "helper.c"
    "libunshield.c"
    "log.c"
)

if(BUILD_STATIC)
    set(LIBUNSHIELD_LIBRARY_TYPE STATIC)
else()
    set(LIBUNSHIELD_LIBRARY_TYPE SHARED)
endif()

if (NOT HAVE_UNISTD_H)
	include_directories( unistd )
endif()

add_library(libunshield ${LIBUNSHIELD_LIBRARY_TYPE} ${LIBUNSHIELD_HEADES} ${LIBUNSHIELD_SOURCES} )

if(USE_OUR_OWN_MD5)
    target_link_libraries(libunshield ${ZLIB_LIBRARY} convert_utf md5)
else()
    target_link_libraries(libunshield ${ZLIB_LIBRARY} convert_utf ${OPENSSL_LIBRARIES})
endif()

set_target_properties(libunshield PROPERTIES OUTPUT_NAME unshield)
set_target_properties(libunshield PROPERTIES SOVERSION ${PROJECT_VERSION})

install(TARGETS libunshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} )
install(FILES libunshield.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )