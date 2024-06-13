include(FetchContent)

set(FETCHCONTENT_QUIET off)

if (sdlpp_ENABLE_TESTING)
    add_subdirectory(doctest)
endif()

add_subdirectory(bsw)
add_subdirectory(sdl2)
add_subdirectory(bitflags)
add_subdirectory(strong_type)

