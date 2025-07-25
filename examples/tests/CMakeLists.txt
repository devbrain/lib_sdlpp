# Helper function to add test executable
function(add_test_executable name)
    add_executable(${name} ${name}.cc)
    target_link_libraries(${name} PRIVATE sdlpp)
    target_compile_options(${name} PRIVATE ${SDLPP_WARNING_FLAGS})
endfunction()

# Simple test programs
add_test_executable(test_properties_simple)
add_test_executable(test_version_check)
add_test_executable(test_gl_simple)
add_test_executable(test_timer_simple)

# Additional test programs moved from examples root
add_test_executable(test_expected)
add_test_executable(test_async_io_simple)