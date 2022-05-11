add_test(
    NAME baldurs_gate_new_compression
    COMMAND $<TARGET_FILE:unshield> -d extractN x data1.cab
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME baldurs_gate_old_compression
    # Enable test after https://github.com/twogood/unshield/issues/156 has been fixed.
    # COMMAND $<TARGET_FILE:unshield> -O -d extractO x data1.cab
    COMMAND ${CMAKE_COMMAND} -E false
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME baldurs_gate_hashes_match
    COMMAND ${CMAKE_COMMAND} -D EXPECTED_HASH_VALUES_FILE=${CMAKE_CURRENT_SOURCE_DIR}/baldurs_gate_patch_v1_1_4315_international.md5 -P ${MD5SUM_CMAKE}
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}/extractN
)

add_test(
    NAME baldurs_gate_extract_test_data
    COMMAND ${CMAKE_COMMAND} -E tar x ${CMAKE_CURRENT_SOURCE_DIR}/test_data.zip
    WORKING_DIRECTORY ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME baldurs_gate_create_tests_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_WORKING_DIRECTORY}
)

add_test(
    NAME baldurs_gate_cleanup
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${TESTS_WORKING_DIRECTORY}
)

set_tests_properties(baldurs_gate_new_compression baldurs_gate_old_compression baldurs_gate_hashes_match PROPERTIES LABELS baldurs_gate)

set_tests_properties(baldurs_gate_old_compression PROPERTIES WILL_FAIL True)

set_tests_properties(baldurs_gate_extract_test_data PROPERTIES DEPENDS baldurs_gate_create_tests_dir)
set_tests_properties(baldurs_gate_create_tests_dir baldurs_gate_extract_test_data PROPERTIES FIXTURES_SETUP baldurs_gate_Extract)
set_tests_properties(baldurs_gate_cleanup PROPERTIES FIXTURES_CLEANUP baldurs_gate_CleanUp)
set_tests_properties(baldurs_gate_old_compression baldurs_gate_new_compression PROPERTIES FIXTURES_REQUIRED "baldurs_gate_Extract;baldurs_gate_CleanUp")

set_tests_properties(baldurs_gate_new_compression PROPERTIES FIXTURES_SETUP baldurs_gate_Unshield)
set_tests_properties(baldurs_gate_hashes_match PROPERTIES FIXTURES_REQUIRED "baldurs_gate_Extract;baldurs_gate_Unshield;baldurs_gate_CleanUp")
