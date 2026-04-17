add_executable(test_stack_memory_file test_stack_memory_file.c)
target_link_libraries(test_stack_memory_file libunshield)

if(BUILD_STATIC)
    if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
        target_link_libraries(test_stack_memory_file -static)
    endif()
endif()

add_test(
    NAME test_stack_memory_file
    COMMAND $<TARGET_FILE:test_stack_memory_file>
)

set_property(TEST test_stack_memory_file PROPERTY ENVIRONMENT_MODIFICATION PATH=path_list_append:$<TARGET_RUNTIME_DLL_DIRS:test_stack_memory_file>)

# Enable test after https://github.com/twogood/unshield/issues/226 has been fixed.
set_tests_properties(test_stack_memory_file PROPERTIES DISABLED True)
