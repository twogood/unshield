add_executable(test_stack_memory_file test_stack_memory_file.c)
target_link_libraries(test_stack_memory_file PUBLIC libunshield)

add_test(
    NAME test_stack_memory_file
    COMMAND $<TARGET_FILE:test_stack_memory_file>
)
