add_test(
    NAME wireplay_new_compression
    COMMAND $<TARGET_FILE:unshield> -d extractN x dpcb1197-Wireplay/DOS/data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME wireplay_old_compression
    COMMAND $<TARGET_FILE:unshield> -O -d extractO x dpcb1197-Wireplay/DOS/data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME wireplay_hashes_match
    COMMAND ${CMAKE_COMMAND} -D EXPECTED_HASH_VALUES_FILE=${CMAKE_CURRENT_SOURCE_DIR}/wireplay.md5 -P ${MD5SUM_CMAKE}
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}/extractO
)

add_test(
    NAME wireplay_extract_test_data
    COMMAND ${CMAKE_COMMAND} -E tar x ${CMAKE_CURRENT_SOURCE_DIR}/test_data.zip
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME wireplay_create_tests_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME wireplay_cleanup
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${TESTS_WORKING_DIRECTORY}
)

set_tests_properties(wireplay_new_compression wireplay_old_compression wireplay_hashes_match PROPERTIES LABELS wireplay)

set_tests_properties(wireplay_new_compression PROPERTIES WILL_FAIL True)

set_tests_properties(wireplay_extract_test_data PROPERTIES DEPENDS wireplay_create_tests_dir)

set_tests_properties(wireplay_create_tests_dir wireplay_extract_test_data PROPERTIES FIXTURES_SETUP wireplay_Extract)
set_tests_properties(wireplay_cleanup PROPERTIES FIXTURES_CLEANUP wireplay_CleanUp)
set_tests_properties(wireplay_old_compression wireplay_new_compression PROPERTIES FIXTURES_REQUIRED "wireplay_Extract;wireplay_CleanUp")

set_tests_properties(wireplay_old_compression PROPERTIES FIXTURES_SETUP wireplay_Unshield)
set_tests_properties(wireplay_hashes_match PROPERTIES FIXTURES_REQUIRED "wireplay_Extract;wireplay_Unshield;wireplay_CleanUp")
