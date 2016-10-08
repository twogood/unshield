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
    
    if(WIN32)
        foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")
        endforeach(flag_var)
    endif(WIN32)
else()
    set(LIBUNSHIELD_LIBRARY_TYPE SHARED)
endif()

add_library(libunshield ${LIBUNSHIELD_LIBRARY_TYPE} ${LIBUNSHIELD_HEADES} ${LIBUNSHIELD_SOURCES})

include_directories(${ZLIB_INCLUDE_DIR})
if(USE_OUR_OWN_MD5)
    target_link_libraries(libunshield ${ZLIB_LIBRARY} md5 convert_utf)
else()
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(libunshield ${ZLIB_LIBRARY} ${OPENSSL_LIBRARIES} convert_utf)
endif()

set_target_properties(libunshield PROPERTIES OUTPUT_NAME unshield)
set_target_properties(libunshield PROPERTIES SOVERSION ${PROJECT_VERSION})

install(TARGETS libunshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES libunshield.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})