add_test(
    NAME the-feeble-files_new_compression
    COMMAND $<TARGET_FILE:unshield> -d extractN x data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME the-feeble-files_old_compression
    COMMAND $<TARGET_FILE:unshield> -O -d extractO x data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME the-feeble-files_hashes_match
    COMMAND ${CMAKE_COMMAND} -D EXPECTED_HASH_VALUES_FILE=${CMAKE_CURRENT_SOURCE_DIR}/the-feeble-files-spanish.md5 -P ${MD5SUM_CMAKE}
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}/extractO
)

add_test(
    NAME the-feeble-files_extract_test_data
    COMMAND ${CMAKE_COMMAND} -E tar x ${CMAKE_CURRENT_SOURCE_DIR}/test_data.zip
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME the-feeble-files_create_tests_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME the-feeble-files_cleanup
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${TESTS_WORKING_DIRECTORY}
)

set_tests_properties(the-feeble-files_new_compression the-feeble-files_old_compression the-feeble-files_hashes_match PROPERTIES LABELS the-feeble-files)

set_tests_properties(the-feeble-files_new_compression PROPERTIES WILL_FAIL True)

set_tests_properties(the-feeble-files_extract_test_data PROPERTIES DEPENDS the-feeble-files_create_tests_dir)

set_tests_properties(the-feeble-files_create_tests_dir the-feeble-files_extract_test_data PROPERTIES FIXTURES_SETUP the-feeble-files_Extract)
set_tests_properties(the-feeble-files_cleanup PROPERTIES FIXTURES_CLEANUP the-feeble-files_CleanUp)
set_tests_properties(the-feeble-files_old_compression the-feeble-files_new_compression PROPERTIES FIXTURES_REQUIRED "the-feeble-files_Extract;the-feeble-files_CleanUp")

set_tests_properties(the-feeble-files_old_compression PROPERTIES FIXTURES_SETUP the-feeble-files_Unshield)
set_tests_properties(the-feeble-files_hashes_match PROPERTIES FIXTURES_REQUIRED "the-feeble-files_Extract;the-feeble-files_Unshield;the-feeble-files_CleanUp")
