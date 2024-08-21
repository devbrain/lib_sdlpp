add_executable(demo_init init.cc)
target_link_libraries(demo_init PRIVATE neutrino::sdlpp thirdparty::sdl2-main)
set_target_properties(demo_init PROPERTIES FOLDER "Demos")


add_library(vga_demo demo.hh demo.cc)
target_link_libraries(vga_demo PUBLIC neutrino::sdlpp)
target_include_directories(vga_demo PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
include(GenerateExportHeader)
generate_export_header(vga_demo EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/vga_demo_export.h)
set_target_properties(vga_demo PROPERTIES FOLDER "Demos")


add_executable(blob blob.cc)
target_link_libraries(blob PRIVATE vga_demo thirdparty::sdl2-main)
set_target_properties(blob PROPERTIES FOLDER "Demos")
if (MSVC)
    target_compile_options(blob PRIVATE /W4)
else()
    target_compile_options(blob PRIVATE -Wall -Wextra -pedantic -Werror)
endif()


add_executable(plasma plasma.cc)
target_link_libraries(plasma PRIVATE vga_demo thirdparty::sdl2-main)
set_target_properties(plasma PROPERTIES FOLDER "Demos")


add_executable(ttf_demo ttf_demo.cc)
target_link_libraries(ttf_demo PRIVATE neutrino::sdlpp thirdparty::sdl2-main)
set_target_properties(ttf_demo PROPERTIES FOLDER "Demos")


add_executable(sys_info sys_info.cc)
target_link_libraries(sys_info PRIVATE neutrino::sdlpp thirdparty::sdl2-main)
set_target_properties(sys_info PROPERTIES FOLDER "Demos")


