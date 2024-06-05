add_executable(sdlpp_unittest
        main.cc
        test_ttf_caller.cc
)

target_link_libraries(sdlpp_unittest neutrino::sdlpp thirdparty::doctest)