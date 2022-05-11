set(MD5SUM_CMAKE ${CMAKE_CURRENT_SOURCE_DIR}/md5sum.cmake)
set(TESTS_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_tmp_dir)

add_subdirectory(v0/avigomanager)
add_subdirectory(v0/baldurs_gate_patch_v1_1_4315_international)
add_subdirectory(v0/the-feeble-files-spanish)
add_subdirectory(v0/wireplay)
add_subdirectory(v5/CVE-2015-1386)
