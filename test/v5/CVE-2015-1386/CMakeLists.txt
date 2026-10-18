add_test(
    NAME CVE-2015-1386_new_compression
    COMMAND $<TARGET_FILE:unshield> -d extractN x ${CMAKE_CURRENT_SOURCE_DIR}/data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME CVE-2015-1386_old_compression
    COMMAND $<TARGET_FILE:unshield> -O -d extractO x ${CMAKE_CURRENT_SOURCE_DIR}/data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME CVE-2015-1386_create_tests_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME CVE-2015-1386_cleanup
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${TESTS_WORKING_DIRECTORY}
)

set_tests_properties(CVE-2015-1386_new_compression CVE-2015-1386_old_compression PROPERTIES LABELS CVE-2015-1386)

set_tests_properties(CVE-2015-1386_new_compression CVE-2015-1386_old_compression PROPERTIES WILL_FAIL True)

set_tests_properties(CVE-2015-1386_create_tests_dir PROPERTIES FIXTURES_SETUP CVE-2015-1386_Extract)
set_tests_properties(CVE-2015-1386_cleanup PROPERTIES FIXTURES_CLEANUP CVE-2015-1386_CleanUp)
set_tests_properties(CVE-2015-1386_old_compression CVE-2015-1386_new_compression PROPERTIES FIXTURES_REQUIRED "CVE-2015-1386_Extract;CVE-2015-1386_CleanUp")
