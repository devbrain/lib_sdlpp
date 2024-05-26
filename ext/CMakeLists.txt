include(FetchContent)

set(FETCHCONTENT_QUIET off)
#get_filename_component(fc_base "../fetched_content" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
#set(FETCHCONTENT_BASE_DIR ${fc_base})
if (sdlpp_ENABLE_TESTING)
    add_subdirectory(doctest)
endif()
add_subdirectory(bsw)
add_subdirectory(sdl2)
add_subdirectory(sdl_ttf)
add_subdirectory(bitflags)

