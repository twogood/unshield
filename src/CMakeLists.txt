add_executable(unshield unshield.c)
target_link_libraries(unshield libunshield)
if(WIN32 AND HAVE_ICONV)
    target_link_libraries(unshield iconv)
endif()

add_executable(unshield-deobfuscate unshield-deobfuscate.c)
target_link_libraries(unshield-deobfuscate libunshield)

if(APPLE)
    set(UNSHIELD_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(UNIX)
    set(UNSHIELD_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

if(DEFINED UNSHIELD_INSTALL_RPATH)
    set_target_properties(unshield unshield-deobfuscate PROPERTIES
        INSTALL_RPATH "${UNSHIELD_INSTALL_RPATH}")
endif()

if(BUILD_STATIC)
    if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
        target_link_options(unshield PRIVATE -static)
        target_link_options(unshield-deobfuscate PRIVATE -static)
    endif()
endif()

install(
    TARGETS unshield
    EXPORT unshieldConfig
    ${UNSHIELD_RUNTIME_DEPENDENCY_ARGS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
