add_subdirectory(convert_utf)

if(USE_OUR_OWN_MD5)
    add_subdirectory(md5)
endif()

set(LIBUNSHIELD_HEADERS
    internal.h
    libunshield.h
    log.h
    cabfile.h
)

set(LIBUNSHIELD_SOURCES
    component.c
    directory.c
    file.c
    file_group.c
    helper.c
    libunshield.c
    log.c
)

if(BUILD_STATIC)
    add_library(libunshield STATIC ${LIBUNSHIELD_HEADERS} ${LIBUNSHIELD_SOURCES})
else()
    add_library(libunshield SHARED ${LIBUNSHIELD_HEADERS} ${LIBUNSHIELD_SOURCES})
endif()

target_include_directories(libunshield PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libunshield PUBLIC ZLIB::ZLIB convert_utf)
set_target_properties(libunshield PROPERTIES OUTPUT_NAME unshield)
set_target_properties(libunshield PROPERTIES VERSION 0.0.0 SOVERSION 0)

if(USE_OUR_OWN_MD5)
    target_link_libraries(libunshield PUBLIC md5)
else()
    target_link_libraries(libunshield PUBLIC OpenSSL::Crypto)
endif()

install(TARGETS libunshield RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES libunshield.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
