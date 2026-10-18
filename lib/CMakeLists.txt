if(USE_OUR_OWN_MD5)
    add_subdirectory(md5)
endif()

set(LIBUNSHIELD_HEADERS
    internal.h
    libunshield.h
    log.h
    cabfile.h
    converter.h
)

set(LIBUNSHIELD_SOURCES
    component.c
    converter.c
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
    target_compile_definitions(libunshield PUBLIC UNSHIELD_DYNAMIC_LIBRARY)
endif()

target_include_directories(libunshield PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(libunshield PUBLIC ZLIB::ZLIB)
target_compile_definitions(libunshield PRIVATE UNSHIELD_EXPORT)
set_target_properties(libunshield PROPERTIES OUTPUT_NAME unshield)
set_target_properties(libunshield PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

if(USE_OUR_OWN_MD5)
    target_link_libraries(libunshield PRIVATE $<TARGET_OBJECTS:md5>)
else()
    target_link_libraries(libunshield PRIVATE OpenSSL::Crypto)
endif()

install(TARGETS libunshield EXPORT unshieldConfig RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES libunshield.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
