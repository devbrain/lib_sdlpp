# =============================================================================
# sdlpp Library
# =============================================================================

include(GenerateExportHeader)

# =============================================================================
# Dependencies via neutrino-cmake
# =============================================================================

# SDL3
include(${NEUTRINO_CMAKE_DIR}/deps/SDL3.cmake)
if(NEUTRINO_SDLPP_BUILD_SHARED)
    set(_sdlpp_had_pic FALSE)
    if(DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
        set(_sdlpp_had_pic TRUE)
        set(_sdlpp_saved_pic "${CMAKE_POSITION_INDEPENDENT_CODE}")
    endif()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    neutrino_fetch_sdl3()
    if(_sdlpp_had_pic)
        set(CMAKE_POSITION_INDEPENDENT_CODE "${_sdlpp_saved_pic}")
        unset(_sdlpp_saved_pic)
    else()
        unset(CMAKE_POSITION_INDEPENDENT_CODE)
    endif()
    unset(_sdlpp_had_pic)
else()
    neutrino_fetch_sdl3()
endif()

# euler - Line rasterization library
include(${NEUTRINO_CMAKE_DIR}/deps/euler.cmake)
neutrino_fetch_euler()

# failsafe - Error handling library
include(${NEUTRINO_CMAKE_DIR}/deps/failsafe.cmake)
neutrino_fetch_failsafe()

# expected - std::expected backport
include(${NEUTRINO_CMAKE_DIR}/deps/expected.cmake)
neutrino_fetch_expected()

# =============================================================================
# Optional Dependencies
# =============================================================================

if(SDLPP_WITH_IMAGE)
    include(${NEUTRINO_CMAKE_DIR}/deps/onyx_image.cmake)
    neutrino_fetch_onyx_image()
endif()

if(SDLPP_WITH_FONT)
    include(${NEUTRINO_CMAKE_DIR}/deps/onyx_font.cmake)
    neutrino_fetch_onyx_font()
endif()

# =============================================================================
# Library Target
# =============================================================================

if(NEUTRINO_SDLPP_BUILD_SHARED)
    add_library(sdlpp SHARED)
    message(STATUS "Building SHARED Library")
else()
    add_library(sdlpp STATIC)
    message(STATUS "Building STATIC Library")
endif()

# Create neutrino:: alias
add_library(neutrino::sdlpp ALIAS sdlpp)
# Legacy alias for compatibility
add_library(sdlpp::sdlpp ALIAS sdlpp)

target_sources(sdlpp PRIVATE
    # Source files
    video/window.cc
    events/events.cc
    audio/audio.cc
    config/hints.cc
    system/power_state.cc
    core/time.cc
    core/log.cc
    core/failsafe_backend.cc
    events/keyboard_codes.cc
    events/mouse_codes.cc
    io/async_io.cc
    io/filesystem.cc
    io/io_common.cc
    io/iostream.cc
    input/gamepad.cc
    input/haptic.cc
    input/hidapi.cc
    input/joystick.cc
    input/mouse.cc
    input/pen.cc
    input/sensor.cc
    input/touch.cc
    system/platform.cc
    system/process.cc
    ui/dialog.cc
    ui/message_box.cc
    ui/tray.cc
    video/blend_mode.cc
    video/camera.cc
    video/display.cc
    video/gl.cc
    video/gpu.cc
    video/pixels.cc
    video/renderer.cc
    video/renderer_dda.cc
    video/surface.cc
    video/surface_renderer.cc
)

# =============================================================================
# Optional Module Sources
# =============================================================================

if(SDLPP_WITH_IMAGE)
    target_sources(sdlpp PRIVATE
        image/sdl_surface_adapter.cc
        image/image.cc
    )
endif()

if(SDLPP_WITH_FONT)
    target_sources(sdlpp PRIVATE
        font/sdl_raster_target.cc
        font/font.cc
        font/font_cache.cc
    )
endif()

# =============================================================================
# Target Configuration
# =============================================================================

target_compile_features(sdlpp PUBLIC cxx_std_20)

# MSVC requires conformant preprocessor for __VA_OPT__ support (C++20 feature)
if(MSVC)
    target_compile_options(sdlpp PUBLIC /Zc:preprocessor)
endif()

generate_export_header(sdlpp
    BASE_NAME SDLPP
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/sdlpp_export.h
)

target_include_directories(sdlpp
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_ROOT}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# SDL3 must be PUBLIC since we include SDL3 headers in our public headers
target_link_libraries(sdlpp
    PUBLIC
        SDL3::SDL3
        euler::euler
        failsafe
        tl::expected
)

# =============================================================================
# Optional Module Dependencies
# =============================================================================

if(SDLPP_WITH_IMAGE)
    target_link_libraries(sdlpp PUBLIC neutrino::onyx_image)
    target_compile_definitions(sdlpp PUBLIC SDLPP_HAS_IMAGE=1)
endif()

if(SDLPP_WITH_FONT)
    target_link_libraries(sdlpp PUBLIC neutrino::onyx_font)
    target_compile_definitions(sdlpp PUBLIC SDLPP_HAS_FONT=1)
endif()

# Apply warning flags
neutrino_target_warnings(sdlpp)

# =============================================================================
# Library Properties
# =============================================================================

if(NEUTRINO_SDLPP_BUILD_SHARED)
    set_target_properties(sdlpp PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )
endif()

set_target_properties(sdlpp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
    FOLDER "Libraries"
)

# =============================================================================
# Installation
# =============================================================================

# Note: Full CMake export/install is not supported when sdlpp is built with
# FetchContent dependencies (SDL3, euler, failsafe). Those targets cannot be
# exported. For installation, build sdlpp against system-installed dependencies.
