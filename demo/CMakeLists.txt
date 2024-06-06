add_executable(demo_init init.cc)
target_link_libraries(demo_init PRIVATE neutrino::sdlpp thirdparty::sdl2-main)


add_library(vga_demo demo.hh demo.cc)
target_link_libraries(vga_demo PUBLIC neutrino::sdlpp)
target_include_directories(vga_demo PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
include(GenerateExportHeader)
generate_export_header(vga_demo EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/vga_demo_export.h)

add_executable(blob blob.cc)
target_link_libraries(blob PRIVATE vga_demo thirdparty::sdl2-main)

add_executable(plasma plasma.cc)
target_link_libraries(plasma PRIVATE vga_demo thirdparty::sdl2-main)

add_executable(ttf_demo ttf_demo.cc)
target_link_libraries(ttf_demo PRIVATE neutrino::sdlpp thirdparty::sdl2-main)

add_executable(sys_info sys_info.cc)
target_link_libraries(sys_info PRIVATE neutrino::sdlpp thirdparty::sdl2-main)
