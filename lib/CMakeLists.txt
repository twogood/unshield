cmake_minimum_required(VERSION 3.6)

project(libunshield C)

add_subdirectory(convert_utf)
add_subdirectory(zlib)

set(LIBUNSHIELD_HEADES
    "md5/md5.h"
    "internal.h"
    "libunshield.h"
    "log.h"
    "cabfile.h"
)

set(LIBUNSHIELD_SOURCES
    "md5/md5.c"
    "component.c"
    "directory.c"
    "file.c"
    "file_group.c"
    "helper.c"
    "libunshield.c"
    "log.c"
)

if(BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC ${LIBUNSHIELD_HEADES} ${LIBUNSHIELD_SOURCES})
else()
    add_library(${PROJECT_NAME} SHARED ${LIBUNSHIELD_HEADES} ${LIBUNSHIELD_SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC convert_utf)
target_link_libraries(${PROJECT_NAME} PUBLIC zlib)

install(TARGETS libunshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES libunshield.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})