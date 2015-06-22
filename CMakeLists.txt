cmake_minimum_required(VERSION 2.8.7)
project(unshield)

# Mimic CMP0048 which is avaliable only for cmake 3.0 and later
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 3)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

option(BUILD_STATIC "Build static version of libunshield" OFF)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(GNUInstallDirs)

check_include_files(byteswap.h HAVE_BYTESWAP_H)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdbool.h HAVE_STDBOOL_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/byteswap.h HAVE_SYS_BYTESWAP_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_function_exists(fnmatch HAVE_FNMATCH)

set(SIZE_FORMAT "zi")
check_c_source_compiles("#include <stdio.h>\nint main(int argc, char **argv) { size_t value = 0; printf(\"%${SIZE_FORMAT}\", value); return 0; }" SIZE_FORMAT_ZI)
if(NOT SIZE_FORMAT_ZI)
	set(SIZE_FORMAT "i")
	check_c_source_compiles("#include <stdio.h>\nint main(int argc, char **argv) { size_t value = 0; printf(\"%${SIZE_FORMAT}\", value); return 0; }" SIZE_FORMAT_I)
	if(NOT SIZE_FORMAT_I)
		set(SIZE_FORMAT "li")
		check_c_source_compiles("#include <stdio.h>\nint main(int argc, char **argv) { size_t value = 0; printf(\"%${SIZE_FORMAT}\", value); return 0; }" SIZE_FORMAT_LI)
		if(NOT SIZE_FORMAT_LI)
			message(FATAL_ERROR "You must be using a really weird platform!")
		endif()
	endif()
endif()

find_package(ZLIB REQUIRED)
find_package(OpenSSL)

if (${OPENSSL_FOUND})
	option(USE_OUR_OWN_MD5 "Build using own md5 implementation" OFF)
else()
	option(USE_OUR_OWN_MD5 "Build using own md5 implementation" ON)
endif()

message(STATUS "OPENSSL_FOUND: ${OPENSSL_FOUND}")
message(STATUS "USE_OUR_OWN_MD5: ${USE_OUR_OWN_MD5}")
message(STATUS "BUILD_STATIC: ${BUILD_STATIC}")

add_definitions(-DHAVE_CONFIG_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib/unshield_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/lib/unshield_config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libunshield.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libunshield.pc @ONLY)

# To force position independent code for static libs on Linux x64
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    add_definitions(-fPIC)
endif()

add_subdirectory(lib)
add_subdirectory(src)

install(FILES man/unshield.1 DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/man/man1)
install(FILES libunshield.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
