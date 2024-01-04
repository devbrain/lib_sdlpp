add_executable(demo_init init.cc)
target_link_libraries(demo_init PRIVATE neutrino::sdlpp thirdparty::sdl2-main)
