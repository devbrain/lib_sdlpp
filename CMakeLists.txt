# =============================================================================
# sdlpp - Modern C++20 wrapper for SDL3
# =============================================================================

cmake_minimum_required(VERSION 3.20)

project(sdlpp
    VERSION 0.1.0
    DESCRIPTION "Modern C++20 wrapper for SDL3"
    LANGUAGES CXX
)

# =============================================================================
# Neutrino CMake Integration
# =============================================================================

include(FetchContent)

if(NOT DEFINED NEUTRINO_CMAKE_DIR)
    FetchContent_Declare(
        neutrino_cmake
        GIT_REPOSITORY https://github.com/devbrain/neutrino-cmake.git
        GIT_TAG master
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(neutrino_cmake)
    set(NEUTRINO_CMAKE_DIR "${neutrino_cmake_SOURCE_DIR}/cmake")
endif()

list(APPEND CMAKE_MODULE_PATH "${NEUTRINO_CMAKE_DIR}")
include(NeutrinoInit)

# =============================================================================
# Options
# =============================================================================

neutrino_define_library_options(sdlpp)

neutrino_is_top_level(_sdlpp_is_top_level)
if(_sdlpp_is_top_level)
    option(NEUTRINO_SDLPP_BUILD_EXAMPLES "Build example applications" ON)
else()
    option(NEUTRINO_SDLPP_BUILD_EXAMPLES "Build example applications" OFF)
endif()

# =============================================================================
# Optional Integrations
# =============================================================================

option(SDLPP_WITH_IMAGE "Enable onyx_image integration for image loading" ON)
option(SDLPP_WITH_FONT  "Enable onyx_font integration for text rendering" ON)

# =============================================================================
# C++ Standard
# =============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Symbol visibility (hide all by default, like Windows)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# =============================================================================
# Output directories
# =============================================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# =============================================================================
# Main Library
# =============================================================================

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src/sdlpp)

# =============================================================================
# Unit Tests
# =============================================================================

if(NEUTRINO_SDLPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# =============================================================================
# Examples
# =============================================================================

if(NEUTRINO_SDLPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# =============================================================================
# Summary
# =============================================================================

neutrino_is_top_level(_sdlpp_is_top_level)
if(_sdlpp_is_top_level)
    neutrino_print_options(sdlpp)
    message(STATUS "")
    message(STATUS "sdlpp ${PROJECT_VERSION} Configuration:")
    message(STATUS "  C++ Standard:    20")
    if(NEUTRINO_SDLPP_BUILD_SHARED)
        message(STATUS "  Library type:    SHARED")
    else()
        message(STATUS "  Library type:    STATIC")
    endif()
    message(STATUS "  Build tests:     ${NEUTRINO_SDLPP_BUILD_TESTS}")
    message(STATUS "  Build examples:  ${NEUTRINO_SDLPP_BUILD_EXAMPLES}")
    message(STATUS "  Build docs:      ${NEUTRINO_SDLPP_BUILD_DOCS}")
    message(STATUS "  Image support:   ${SDLPP_WITH_IMAGE}")
    message(STATUS "  Font support:    ${SDLPP_WITH_FONT}")
    message(STATUS "")
endif()
