# =============================================================================
# sdlpp Library
# =============================================================================

include(GenerateExportHeader)

# =============================================================================
# Dependencies via neutrino-cmake
# =============================================================================

# SDL3
include(${NEUTRINO_CMAKE_DIR}/deps/SDL3.cmake)
neutrino_fetch_sdl3()

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
# Target Configuration
# =============================================================================

target_compile_features(sdlpp PUBLIC cxx_std_20)

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

if(NEUTRINO_SDLPP_INSTALL)
    include(NeutrinoInstall)

    # Install generated export header
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sdlpp_export.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sdlpp
    )

    neutrino_install_library(sdlpp
        NAMESPACE neutrino::
        COMPATIBILITY SameMajorVersion
    )
endif()
