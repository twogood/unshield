# Regression tests for malformed InstallShield headers.  Before the header
# offset validation was added these inputs crashed the unshield binary with
# an out-of-bounds read (SIGSEGV); now they must be rejected cleanly.  A
# signal-terminated run fails the test even with WILL_FAIL, so these tests
# only pass once the parser fails gracefully.  The fixtures must be named
# data1.cab (unshield derives "<prefix>N.cab" volume names from the argument).
add_test(
    NAME security_cab_descriptor_oob
    COMMAND $<TARGET_FILE:unshield> l data1.cab
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/cab_descriptor_oob
)
set_tests_properties(security_cab_descriptor_oob PROPERTIES WILL_FAIL True)

add_test(
    NAME security_component_list_oob
    COMMAND $<TARGET_FILE:unshield> l data1.cab
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/component_list_oob
)
set_tests_properties(security_component_list_oob PROPERTIES WILL_FAIL True)
