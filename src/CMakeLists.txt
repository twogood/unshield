list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif()

add_executable(unshield unshield.c)
target_link_libraries(unshield libunshield)
if(WIN32)
    if(HAVE_ICONV)
        target_link_libraries(unshield iconv)
    endif()
    target_link_libraries(unshield ${ZLIB_LIBRARIES})
endif()

add_executable(unshield-deobfuscate unshield-deobfuscate.c)
target_link_libraries(unshield-deobfuscate libunshield)
if(WIN32)
    target_link_libraries(unshield-deobfuscate ${ZLIB_LIBRARIES})
endif()

if(BUILD_STATIC AND MSVC)
    set_msvc_runtime_static()
endif()

set(TARGETS_TO_INSTALL unshield)

if(BUILD_STATIC)
    list(APPEND TARGETS_TO_INSTALL convert_utf)
endif()

install(TARGETS ${TARGETS_TO_INSTALL} EXPORT unshieldConfig RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
