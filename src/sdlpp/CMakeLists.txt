if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif ()

if (DEFINED sdlpp_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${sdlpp_SHARED_LIBS}")
endif ()

add_library(sdlpp INTERFACE)

add_library(neutrino::sdlpp ALIAS sdlpp)
target_compile_features(sdlpp INTERFACE cxx_std_17)

target_include_directories(sdlpp INTERFACE ${PROJECT_INCLUDE})
target_link_libraries(sdlpp INTERFACE thirdparty::sdl2 bsw)



