SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

add_executable(unshield "unshield.c")
target_link_libraries(unshield libunshield)

add_executable(unshield-deobfuscate "unshield-deobfuscate.c")
target_link_libraries(unshield-deobfuscate libunshield)

if(BUILD_STATIC AND WIN32 AND MSVC)
    set_msvc_runtime_static()
endif(BUILD_STATIC AND WIN32 AND MSVC)

install(TARGETS unshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})