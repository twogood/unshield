add_test(
    NAME avigomanager_new_compression
    COMMAND $<TARGET_FILE:unshield> -d extractN x data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME avigomanager_old_compression
    COMMAND $<TARGET_FILE:unshield> -O -d extractO x data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME avigomanager_hashes_match
    COMMAND ${CMAKE_COMMAND} -D EXPECTED_HASH_VALUES_FILE=${CMAKE_CURRENT_SOURCE_DIR}/avigomanager.md5 -P ${MD5SUM_CMAKE}
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}/extractO
)

add_test(
    NAME avigomanager_extract_test_data
    COMMAND ${CMAKE_COMMAND} -E tar x ${CMAKE_CURRENT_SOURCE_DIR}/test_data.zip
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME avigomanager_create_tests_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME avigomanager_cleanup
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${TESTS_WORKING_DIRECTORY}
)

set_tests_properties(avigomanager_new_compression avigomanager_old_compression avigomanager_hashes_match PROPERTIES LABELS avigomanager)

set_tests_properties(avigomanager_new_compression PROPERTIES WILL_FAIL True)

set_tests_properties(avigomanager_extract_test_data PROPERTIES DEPENDS avigomanager_create_tests_dir)

set_tests_properties(avigomanager_create_tests_dir avigomanager_extract_test_data PROPERTIES FIXTURES_SETUP avigomanager_Extract)
set_tests_properties(avigomanager_cleanup PROPERTIES FIXTURES_CLEANUP avigomanager_CleanUp)
set_tests_properties(avigomanager_old_compression avigomanager_new_compression PROPERTIES FIXTURES_REQUIRED "avigomanager_Extract;avigomanager_CleanUp")

set_tests_properties(avigomanager_old_compression PROPERTIES FIXTURES_SETUP avigomanager_Unshield)
set_tests_properties(avigomanager_hashes_match PROPERTIES FIXTURES_REQUIRED "avigomanager_Extract;avigomanager_Unshield;avigomanager_CleanUp")
