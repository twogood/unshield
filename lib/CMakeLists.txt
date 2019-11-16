LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
# Enable CMP0042
if (APPLE)
	set(CMAKE_MACOSX_RPATH ON)
endif ()
ENDIF()

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
    
    if(MSVC)
        set_msvc_runtime_static()
    endif()
else()
    set(LIBUNSHIELD_LIBRARY_TYPE SHARED)
endif()

add_library(libunshield ${LIBUNSHIELD_LIBRARY_TYPE} ${LIBUNSHIELD_HEADES} ${LIBUNSHIELD_SOURCES})

include_directories(${ZLIB_INCLUDE_DIR})
if(USE_OUR_OWN_MD5)
    target_link_libraries(libunshield ZLIB::ZLIB md5 convert_utf)
else()
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(libunshield ZLIB::ZLIB ${OPENSSL_LIBRARIES} convert_utf)
endif()

set_target_properties(libunshield PROPERTIES OUTPUT_NAME unshield)
set_target_properties(libunshield PROPERTIES VERSION 0.0.0 SOVERSION 0)

install(TARGETS libunshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES libunshield.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})