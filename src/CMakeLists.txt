SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

add_executable(unshield "unshield.c")

if ("${HAVE_ICONV}" AND "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(UNSHIELD_PLATFORM_LINKAGE libiconv)
endif()

target_link_libraries(unshield libunshield ${UNSHIELD_PLATFORM_LINKAGE})

add_executable(unshield-deobfuscate "unshield-deobfuscate.c")
target_link_libraries(unshield-deobfuscate libunshield)

install(TARGETS unshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
