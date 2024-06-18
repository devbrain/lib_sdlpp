add_executable(sdlpp_unittest
        main.cc
        test_ttf_caller.cc
        test_from_to_string.cc
)

target_link_libraries(sdlpp_unittest neutrino::sdlpp thirdparty::doctest)