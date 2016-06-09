cmake_minimum_required(VERSION 2.8.7)
project(unshield)

# Mimic CMP0048 which is available only for cmake 3.0 and later
set(PROJECT_VERSION_MAJOR 2)
set(PROJECT_VERSION_MINOR 3)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

option(BUILD_STATIC "Build static version of libunshield" ON)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceRuns)
include(GNUInstallDirs)

check_include_files( byteswap.h HAVE_BYTESWAP_H)
check_include_files( dlfcn.h HAVE_DLFCN_H)
check_include_files( inttypes.h HAVE_INTTYPES_H)
check_include_files( memory.h HAVE_MEMORY_H)
check_include_files( stdbool.h HAVE_STDBOOL_H)
check_include_files( stdint.h HAVE_STDINT_H)
check_include_files( stdlib.h HAVE_STDLIB_H)
check_include_files( strings.h HAVE_STRINGS_H)
check_include_files( string.h HAVE_STRING_H)
check_include_files( sys/byteswap.h HAVE_SYS_BYTESWAP_H)
check_include_files( sys/stat.h HAVE_SYS_STAT_H)
check_include_files( sys/types.h HAVE_SYS_TYPES_H)
check_include_files( unistd.h HAVE_UNISTD_H)
check_function_exists( fnmatch HAVE_FNMATCH)
check_function_exists( iconv HAVE_ICONV)

###############################
# Check what format string for printf  works to output some size_t type
set( SourcePre " #include <stdio.h> \n int main() {  char buf[2]; return  1-sprintf( buf,\"%")
set( SourcePost "\",  (size_t) 0);} ")
	
set(SIZE_FORMAT "zi")
CHECK_C_SOURCE_RUNS( "${SourcePre}${SIZE_FORMAT}${SourcePost}" SIZE_FORMAT_ZI)
if(NOT SIZE_FORMAT_ZI)
	set(SIZE_FORMAT "i")
	CHECK_C_SOURCE_RUNS( "${SourcePre}${SIZE_FORMAT}${SourcePost}" SIZE_FORMAT_I)
	if(NOT SIZE_FORMAT_I)
		set(SIZE_FORMAT "li")
		CHECK_C_SOURCE_RUNS( "${SourcePre}${SIZE_FORMAT}${SourcePost}" SIZE_FORMAT_LI)
		if(NOT SIZE_FORMAT_LI)
			message(FATAL_ERROR "You must be using a really weird platform!")
		endif()
	endif()
endif()
message(STATUS "SIZE_FORMAT: \
	${SIZE_FORMAT}")

###############################
	
find_package(ZLIB REQUIRED) #http://gnuwin32.sourceforge.net/packages/zlib.htm
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
include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/lib)

include_directories( ${ZLIB_INCLUDE_DIR} )


# To force position independent code for static libs on Linux x64
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    add_definitions(-fPIC)
endif()

add_subdirectory(lib)
add_subdirectory(src)

install(FILES man/unshield.1 DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/man/man1)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libunshield.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig) 
