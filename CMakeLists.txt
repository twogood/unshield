cmake_minimum_required(VERSION 2.8.7)
project(unshield C)

# Mimic CMP0048 which is avaliable only for cmake 3.0 and later
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 4)
set(PROJECT_VERSION_PATCH 3)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

option(BUILD_STATIC "Build static version of libunshield" OFF)

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(GNUInstallDirs)

check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdbool.h HAVE_STDBOOL_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(fnmatch.h HAVE_FNMATCH_H)

set(ICONVTEST
    "#include <stddef.h>\n#include <iconv.h>\nint main() {\nchar *a, *b;\nsize_t i, j;\niconv_t ic;\nic = iconv_open(\"to\", \"from\");\niconv(ic, &a, &i, &b, &j);\niconv_close(ic); }"
)
check_c_source_compiles("${ICONVTEST}" HAVE_ICONV)
if (NOT HAVE_ICONV)
   check_symbol_exists(iconv iconv.h HAVE_ICONV)
   if (HAVE_ICONV)
      #assume libiconv
      set(ICONV_LIBRARIES "-liconv")
   endif()
endif()

message(STATUS "BUILD_STATIC: ${BUILD_STATIC}")

add_definitions(-DHAVE_CONFIG_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib/unshield_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/lib/unshield_config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libunshield.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libunshield.pc @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/lib)

# Function to change MSVC runtime linkage to static
function(set_msvc_runtime_static)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endfunction()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32_msvc)
endif ()

add_subdirectory(lib)
add_subdirectory(src)

install(FILES man/unshield.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libunshield.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
