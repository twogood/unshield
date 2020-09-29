cmake_minimum_required(VERSION 3.6)

project(libunshield)

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

add_library(${PROJECT_NAME} STATIC ${LIBUNSHIELD_HEADES} ${LIBUNSHIELD_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ZLIB::ZLIB convert_utf)

if(USE_OUR_OWN_MD5)
    target_link_libraries(${PROJECT_NAME} PUBLIC md5)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::Crypto)
endif()

install(TARGETS libunshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES libunshield.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})