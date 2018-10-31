LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")

add_executable(unshield "unshield.c")
target_link_libraries(unshield libunshield)
if(WIN32)
    if(HAVE_ICONV)
        target_link_libraries(unshield iconv)
    endif(HAVE_ICONV)
    target_link_libraries(unshield z)
endif()

add_executable(unshield-deobfuscate "unshield-deobfuscate.c")
target_link_libraries(unshield-deobfuscate libunshield)
if(WIN32)
    target_link_libraries(unshield-deobfuscate z)
endif()

add_executable(unshield-afl "unshield-afl.c")
target_link_libraries(unshield-afl libunshield)

if(BUILD_STATIC AND MSVC)
    set_msvc_runtime_static()
endif(BUILD_STATIC AND MSVC)

install(TARGETS unshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})