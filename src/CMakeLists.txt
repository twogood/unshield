list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif()

add_executable(unshield unshield.c)
target_link_libraries(unshield libunshield)
if(WIN32 AND HAVE_ICONV)
    target_link_libraries(unshield iconv)
endif()

add_executable(unshield-deobfuscate unshield-deobfuscate.c)
target_link_libraries(unshield-deobfuscate libunshield)

if(BUILD_STATIC)
    if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
        target_link_libraries(unshield -static)
        target_link_libraries(unshield-deobfuscate -static)
    endif()
endif()

install(TARGETS unshield EXPORT unshieldConfig RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
