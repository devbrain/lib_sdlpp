# Determine library type based on SDLPP_BUILD_SHARED

if(SDLPP_BUILD_SHARED)
    set(SDLPP_LIBRARY_TYPE SHARED)
    message("Building SHARED Library")
else()
    set(SDLPP_LIBRARY_TYPE STATIC)
    message("Building STATIC Library")
endif()

add_library(sdlpp ${SDLPP_LIBRARY_TYPE}
        # Core subsystem
        ${PROJECT_ROOT}/include/sdlpp/core/core.hh
        ${PROJECT_ROOT}/include/sdlpp/core/error.hh
        ${PROJECT_ROOT}/include/sdlpp/core/sdl.hh
        ${PROJECT_ROOT}/include/sdlpp/core/version.hh
        ${PROJECT_ROOT}/include/sdlpp/core/log.hh
        ${PROJECT_ROOT}/include/sdlpp/core/failsafe_backend.hh
        ${PROJECT_ROOT}/include/sdlpp/core/time.hh
        ${PROJECT_ROOT}/include/sdlpp/core/timer.hh
        
        # App subsystem
        ${PROJECT_ROOT}/include/sdlpp/app/app.hh
        ${PROJECT_ROOT}/include/sdlpp/app/app_types.hh
        ${PROJECT_ROOT}/include/sdlpp/app/app_impl.hh
        ${PROJECT_ROOT}/include/sdlpp/app/app_builder.hh
        ${PROJECT_ROOT}/include/sdlpp/app/stateful_app.hh
        ${PROJECT_ROOT}/include/sdlpp/app/scene_app.hh
        ${PROJECT_ROOT}/include/sdlpp/app/game_app.hh

        # Video subsystem
        ${PROJECT_ROOT}/include/sdlpp/video/blend_mode.hh
        ${PROJECT_ROOT}/include/sdlpp/video/color.hh
        ${PROJECT_ROOT}/include/sdlpp/video/display.hh
        ${PROJECT_ROOT}/include/sdlpp/video/gl.hh
        ${PROJECT_ROOT}/include/sdlpp/video/gpu.hh
        ${PROJECT_ROOT}/include/sdlpp/video/gpu_commands.hh
        ${PROJECT_ROOT}/include/sdlpp/video/gpu_defs.hh
        ${PROJECT_ROOT}/include/sdlpp/video/gpu_resources.hh
        ${PROJECT_ROOT}/include/sdlpp/video/palette.hh
        ${PROJECT_ROOT}/include/sdlpp/video/pixels.hh
        ${PROJECT_ROOT}/include/sdlpp/video/renderer.hh
        ${PROJECT_ROOT}/include/sdlpp/video/surface.hh
        ${PROJECT_ROOT}/include/sdlpp/video/texture.hh
        ${PROJECT_ROOT}/include/sdlpp/video/window.hh
        ${PROJECT_ROOT}/include/sdlpp/video/camera.hh
        ${PROJECT_ROOT}/include/sdlpp/video/surface_renderer.hh

        # Audio subsystem
        ${PROJECT_ROOT}/include/sdlpp/audio/audio.hh

        # Events subsystem
        ${PROJECT_ROOT}/include/sdlpp/events/events.hh
        ${PROJECT_ROOT}/include/sdlpp/events/event_impl.hh
        ${PROJECT_ROOT}/include/sdlpp/events/event_types.hh
        ${PROJECT_ROOT}/include/sdlpp/events/keyboard_codes.hh
        ${PROJECT_ROOT}/include/sdlpp/events/mouse_codes.hh

        # Input subsystem
        ${PROJECT_ROOT}/include/sdlpp/input/joystick.hh
        ${PROJECT_ROOT}/include/sdlpp/input/gamepad.hh
        ${PROJECT_ROOT}/include/sdlpp/input/touch.hh
        ${PROJECT_ROOT}/include/sdlpp/input/pen.hh
        ${PROJECT_ROOT}/include/sdlpp/input/sensor.hh
        ${PROJECT_ROOT}/include/sdlpp/input/hidapi.hh
        ${PROJECT_ROOT}/include/sdlpp/input/haptic.hh
        ${PROJECT_ROOT}/include/sdlpp/input/keyboard.hh
        ${PROJECT_ROOT}/include/sdlpp/input/mouse.hh



        # Filesystem/IO subsystem
        ${PROJECT_ROOT}/include/sdlpp/io/async_io.hh
        ${PROJECT_ROOT}/include/sdlpp/io/filesystem.hh
        ${PROJECT_ROOT}/include/sdlpp/io/io_common.hh
        ${PROJECT_ROOT}/include/sdlpp/io/iostream.hh
        ${PROJECT_ROOT}/include/sdlpp/io/storage.hh

        # System subsystem
        ${PROJECT_ROOT}/include/sdlpp/system/cpu.hh
        ${PROJECT_ROOT}/include/sdlpp/system/cpu_dispatch.hh
        ${PROJECT_ROOT}/include/sdlpp/system/intrinsics.hh
        ${PROJECT_ROOT}/include/sdlpp/system/locale.hh
        ${PROJECT_ROOT}/include/sdlpp/system/misc.hh
        ${PROJECT_ROOT}/include/sdlpp/system/platform.hh
        ${PROJECT_ROOT}/include/sdlpp/system/process.hh
        ${PROJECT_ROOT}/include/sdlpp/system/shared_object.hh
        ${PROJECT_ROOT}/include/sdlpp/system/clipboard.hh
        ${PROJECT_ROOT}/include/sdlpp/system/power_state.hh

        # Config subsystem
        ${PROJECT_ROOT}/include/sdlpp/config/hints.hh
        ${PROJECT_ROOT}/include/sdlpp/config/properties.hh

        # UI subsystem
        ${PROJECT_ROOT}/include/sdlpp/ui/dialog.hh
        ${PROJECT_ROOT}/include/sdlpp/ui/message_box.hh
        ${PROJECT_ROOT}/include/sdlpp/ui/tray.hh

        # Utility headers

        ${PROJECT_ROOT}/include/sdlpp/utility/dimension.hh
        ${PROJECT_ROOT}/include/sdlpp/utility/geometry.hh
        ${PROJECT_ROOT}/include/sdlpp/utility/geometry_concepts.hh
        ${PROJECT_ROOT}/include/sdlpp/utility/guid.hh

        # Concepts headers
        ${PROJECT_ROOT}/include/sdlpp/concepts/renderer_concepts.hh


        # Detail headers
        ${PROJECT_ROOT}/include/sdlpp/detail/expected.hh
        ${PROJECT_ROOT}/include/sdlpp/detail/compiler.hh
        ${PROJECT_ROOT}/include/sdlpp/detail/type_utils.hh
        ${PROJECT_ROOT}/include/sdlpp/detail/pointer.hh

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

include(GenerateExportHeader)
generate_export_header(sdlpp
        BASE_NAME SDLPP
        EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/sdlpp_export.h
)

target_include_directories(sdlpp PUBLIC 
        $<BUILD_INTERFACE:${PROJECT_ROOT}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# SDL3 must be PUBLIC since we include SDL3 headers in our public headers
target_link_libraries(sdlpp 
    PUBLIC
        SDL3::SDL3
        thirdparty::euler
        thirdparty::expected 
        thirdparty::failsafe
)

# If euler is built as part of the project, install it with sdlpp
if(TARGET euler)
    # Install euler headers
    install(DIRECTORY ${euler_SOURCE_DIR}/include/euler
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    # Install euler target
    install(TARGETS euler
        EXPORT sdlppTargets
    )
endif()

# Apply warning flags to our library
target_compile_options(sdlpp PRIVATE ${SDLPP_WARNING_FLAGS})

# Generate export header for shared library builds
if(SDLPP_BUILD_SHARED)
    # Set visibility properties
    set_target_properties(sdlpp PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )
endif()

# Set library properties
set_target_properties(sdlpp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Install targets
if(SDLPP_INSTALL)
    include(CMakePackageConfigHelpers)
    
    # Install the library
    install(TARGETS sdlpp
        EXPORT sdlppTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sdlpp
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
    
    # Export targets
    install(EXPORT sdlppTargets
        FILE sdlppTargets.cmake
        NAMESPACE sdlpp::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdlpp
    )
    
    # Create config file
    configure_package_config_file(
        ${PROJECT_ROOT}/cmake/sdlppConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/sdlppConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdlpp
    )
    
    # Create version file
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/sdlppConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    
    # Configure and install the dependencies file
    configure_file(
        ${PROJECT_ROOT}/cmake/sdlppTargets-dependencies.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/sdlppTargets-dependencies.cmake
        @ONLY
    )
    
    # Install config files
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/sdlppConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/sdlppConfigVersion.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/sdlppTargets-dependencies.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdlpp
    )
    
    # Generate and install pkg-config file
    configure_file(
        ${PROJECT_ROOT}/cmake/sdlpp.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/sdlpp.pc
        @ONLY
    )
    
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sdlpp.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()