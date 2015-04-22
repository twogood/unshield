add_executable(unshiled "unshield.c")
target_link_libraries(unshiled libunshield)

add_executable(unshield-deobfuscate "unshield-deobfuscate.c")
target_link_libraries(unshield-deobfuscate libunshield)

install(TARGETS unshiled RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})