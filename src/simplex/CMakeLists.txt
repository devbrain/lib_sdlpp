# =============================================================================
# simplex Library
# =============================================================================

include(GenerateExportHeader)

if (NOT TARGET sdlpp)
    message(FATAL_ERROR "simplex requires the sdlpp target to be defined first")
endif ()

if (NOT SDLPP_WITH_IMAGE OR NOT SDLPP_WITH_FONT)
    message(FATAL_ERROR "simplex requires sdlpp to be built with SDLPP_WITH_IMAGE=ON and SDLPP_WITH_FONT=ON")
endif ()

# =============================================================================
# Library Target
# =============================================================================

if (NEUTRINO_SDLPP_BUILD_SHARED)
    add_library(simplex SHARED)
else ()
    add_library(simplex STATIC)
endif ()

add_library(neutrino::simplex ALIAS simplex)
add_library(simplex::simplex ALIAS simplex)

target_sources(simplex PRIVATE
        ${PROJECT_ROOT}/include/simplex/detail/export.hh
        ${PROJECT_ROOT}/include/simplex/simplex.hh
        ${PROJECT_ROOT}/include/simplex/dp.hh
        ${PROJECT_ROOT}/include/simplex/geometry.hh
        ${PROJECT_ROOT}/include/simplex/sprite_atlas.hh
        ${PROJECT_ROOT}/include/simplex/sprite.hh
        ${PROJECT_ROOT}/include/simplex/mesh.hh
        ${PROJECT_ROOT}/include/simplex/effects.hh
        ${PROJECT_ROOT}/include/simplex/collide/shapes.hh
        ${PROJECT_ROOT}/include/simplex/collide/bridge.hh

        dp.cc
        simplex.cc
        sprite_atlas.cc
        sprite.cc
        mesh.cc
        effects.cc
)

# =============================================================================
# Target Configuration
# =============================================================================

target_compile_features(simplex PUBLIC cxx_std_20)

if (MSVC)
    target_compile_options(simplex PUBLIC /Zc:preprocessor)
endif ()

generate_export_header(simplex
        BASE_NAME SIMPLEX
        EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/simplex_export.h
)

target_include_directories(simplex
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_ROOT}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(simplex
        PUBLIC
        sdlpp
)

neutrino_target_warnings(simplex)

# =============================================================================
# Library Properties
# =============================================================================

if (NEUTRINO_SDLPP_BUILD_SHARED)
    set_target_properties(simplex PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            C_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN YES
    )
endif ()

set_target_properties(simplex PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        POSITION_INDEPENDENT_CODE ON
        FOLDER "Libraries"
)
