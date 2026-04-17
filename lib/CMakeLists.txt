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

add_library(libunshield ${LIBUNSHIELD_HEADERS} ${LIBUNSHIELD_SOURCES})

set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

include(GenerateExportHeader)
generate_export_header(libunshield EXPORT_MACRO_NAME UNSHIELD_API)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(libunshield PUBLIC LIBUNSHIELD_STATIC_DEFINE)
endif()

target_include_directories(libunshield PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(libunshield PRIVATE ZLIB::ZLIB)
set_target_properties(libunshield PROPERTIES OUTPUT_NAME unshield)
set_target_properties(libunshield PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

if(USE_OUR_OWN_MD5)
    add_subdirectory(md5)
    target_link_libraries(libunshield PRIVATE $<TARGET_OBJECTS:md5>)
else()
    target_link_libraries(libunshield PRIVATE OpenSSL::Crypto)
endif()

install(TARGETS libunshield EXPORT unshieldConfig RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES libunshield.h ${CMAKE_CURRENT_BINARY_DIR}/libunshield_export.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

if(WIN32)
    install(IMPORTED_RUNTIME_ARTIFACTS ZLIB::ZLIB
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    if(NOT USE_OUR_OWN_MD5)
        install(IMPORTED_RUNTIME_ARTIFACTS OpenSSL::Crypto
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
endif()
