SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

if (NOT HAVE_UNISTD_H)
	include_directories( ../lib/unistd )
	set( INC_UniStd
    "../lib/unistd/getopt.c"
	)
endif()

add_executable(unshield "unshield.c" ${INC_UniStd} )
target_link_libraries(unshield libunshield )

add_executable(unshield-deobfuscate "unshield-deobfuscate.c")
target_link_libraries(unshield-deobfuscate libunshield)

install(TARGETS unshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})