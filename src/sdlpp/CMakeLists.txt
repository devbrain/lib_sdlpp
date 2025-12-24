# =============================================================================
# sdlpp Library
# =============================================================================

include(GenerateExportHeader)
include(FetchContent)

# =============================================================================
# Dependencies
# =============================================================================

set(FETCHCONTENT_QUIET OFF)

# SDL3
if(NOT TARGET SDL3::SDL3)
    find_package(SDL3 QUIET)
    if(NOT SDL3_FOUND)
        message(STATUS "SDL3 not found, fetching from GitHub...")
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG main
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(SDL3)
    endif()
endif()

# euler - Line rasterization library
if(NOT TARGET euler::euler)
    find_package(euler QUIET)
    if(NOT euler_FOUND)
        message(STATUS "euler not found, fetching from GitHub...")
        set(NEUTRINO_EULER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(NEUTRINO_EULER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(NEUTRINO_EULER_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            euler
            GIT_REPOSITORY https://github.com/devbrain/euler.git
            GIT_TAG master
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(euler)
    endif()
endif()

# failsafe - Error handling library
if(NOT TARGET failsafe AND NOT TARGET thirdparty::failsafe)
    find_package(failsafe QUIET)
    if(NOT failsafe_FOUND)
        message(STATUS "failsafe not found, fetching from GitHub...")
        set(FAILSAFE_BUILD_TESTS OFF CACHE INTERNAL "")
        set(FAILSAFE_BUILD_EXAMPLES OFF CACHE INTERNAL "")
        FetchContent_Declare(
            FAILSAFE
            GIT_REPOSITORY https://github.com/devbrain/failsafe.git
            GIT_TAG master
            GIT_PROGRESS TRUE
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(FAILSAFE)
    endif()
endif()

# expected - std::expected backport
if(NOT TARGET tl::expected)
    message(STATUS "expected not found, fetching from GitHub...")
    set(EXPECTED_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        expected
        GIT_REPOSITORY https://github.com/TartanLlama/expected.git
        GIT_TAG v1.1.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(expected)

    # Create thirdparty::expected alias if it doesn't exist
    if(NOT TARGET thirdparty::expected)
        add_library(thirdparty::expected ALIAS expected)
    endif()
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
    PRIVATE
        tl::expected
)

# Apply warning flags
target_compile_options(sdlpp PRIVATE ${SDLPP_WARNING_FLAGS})

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
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install the library
    install(TARGETS sdlpp
        EXPORT sdlppTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Install headers
    install(DIRECTORY ${PROJECT_ROOT}/include/sdlpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hh"
    )

    # Install generated export header
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sdlpp_export.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sdlpp
    )
endif()
