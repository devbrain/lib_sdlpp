include(FetchContent)

set(FETCHCONTENT_QUIET off)
#get_filename_component(fc_base "../fetched_content" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
#set(FETCHCONTENT_BASE_DIR ${fc_base})

add_subdirectory(doctest)
add_subdirectory(bsw)
add_subdirectory(sdl2)
add_subdirectory(bitflags)

