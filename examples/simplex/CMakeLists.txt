add_executable(example_simplex example_simplex.cc)
target_link_libraries(example_simplex PRIVATE simplex)
target_compile_options(example_simplex PRIVATE ${SDLPP_WARNING_FLAGS})


add_executable(example_collision example_collision.cc)
target_link_libraries(example_collision PRIVATE simplex)
target_compile_options(example_collision PRIVATE ${SDLPP_WARNING_FLAGS})


add_executable(example_animation example_animation.cc)
target_link_libraries(example_animation PRIVATE simplex)
target_compile_options(example_animation PRIVATE ${SDLPP_WARNING_FLAGS})

add_executable(example_fire example_fire.cc)
target_link_libraries(example_fire PRIVATE simplex)
target_compile_options(example_fire PRIVATE ${SDLPP_WARNING_FLAGS})