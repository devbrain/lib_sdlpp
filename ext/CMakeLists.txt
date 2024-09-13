include(FetchContent)

set(FETCHCONTENT_QUIET off)

if (sdlpp_ENABLE_TESTING)
    add_subdirectory(doctest)
endif()

macro(ADD_TO_FOLDER NAME FOLDER)
    if (TARGET ${NAME})
        set_target_properties(${NAME} PROPERTIES FOLDER ${FOLDER})
    endif()
endmacro()

add_subdirectory(bsw)
add_subdirectory(sdl2)
add_subdirectory(bitflags)
add_subdirectory(neutrino_math)

