# Helper function to add example executable
function(add_example_executable name path)
    add_executable(${name} ${path})
    target_link_libraries(${name} PRIVATE sdlpp)
    target_compile_options(${name} PRIVATE ${SDLPP_WARNING_FLAGS})
endfunction()

# Core examples
add_example_executable(example_error core/example_error.cc)
add_example_executable(example_log core/example_log.cc)
add_example_executable(example_timer core/example_timer.cc)
add_example_executable(example_time core/example_time.cc)
add_example_executable(example_version core/example_version.cc)
add_example_executable(example_failsafe_logger core/example_failsafe_logger.cc)

# App examples
add_example_executable(example_basic_app app/example_basic_app.cc)
add_example_executable(example_app_builder app/example_app_builder.cc)
add_example_executable(example_scene_app app/example_scene_app.cc)
add_example_executable(example_game_app app/example_game_app.cc)
add_example_executable(example_callbacks app/example_callbacks.cc)

# Video examples
add_example_executable(example_window_renderer video/example_window_renderer.cc)
add_example_executable(example_gl video/example_gl.cc)
add_example_executable(example_gpu_triangle video/example_gpu_triangle.cc)
add_example_executable(example_camera video/example_camera.cc)
add_example_executable(example_dda_comprehensive video/example_dda_comprehensive.cc)

# Special handling for OpenGL example
find_package(OpenGL REQUIRED)
target_link_libraries(example_gl PRIVATE OpenGL::GL)

# Audio examples
add_example_executable(example_audio_callback audio/example_audio_callback.cc)
add_example_executable(example_audio_tone audio/example_audio_tone.cc)
add_example_executable(example_audio_wav audio/example_audio_wav.cc)

# Events examples
add_example_executable(example_events events/example_events.cc)
add_example_executable(example_event_category events/example_event_category.cc)

# Input examples
add_example_executable(example_gamepad input/example_gamepad.cc)
add_example_executable(example_joystick input/example_joystick.cc)
add_example_executable(example_keyboard input/example_keyboard.cc)
add_example_executable(example_mouse input/example_mouse.cc)
add_example_executable(example_pen input/example_pen.cc)
add_example_executable(example_touch input/example_touch.cc)
add_example_executable(example_haptic input/example_haptic.cc)
add_example_executable(example_hidapi input/example_hidapi.cc)
add_example_executable(example_sensor input/example_sensor.cc)

# I/O examples
add_example_executable(example_filesystem io/example_filesystem.cc)
add_example_executable(example_iostream io/example_iostream.cc)
add_example_executable(example_async_io io/example_async_io.cc)
add_example_executable(example_load_file_async io/example_load_file_async.cc)
add_example_executable(example_storage io/example_storage.cc)

# System examples
add_example_executable(example_clipboard system/example_clipboard.cc)
add_example_executable(example_cpu system/example_cpu.cc)
add_example_executable(example_intrinsics system/example_intrinsics.cc)
add_example_executable(example_locale system/example_locale.cc)
add_example_executable(example_misc system/example_misc.cc)
add_example_executable(example_platform system/example_platform.cc)
add_example_executable(example_platform_enhanced system/example_platform_enhanced.cc)
add_example_executable(example_process system/example_process.cc)
add_example_executable(example_shared_object system/example_shared_object.cc)

# UI examples
add_example_executable(example_dialog ui/example_dialog.cc)
add_example_executable(example_message_box ui/example_message_box.cc)
add_example_executable(example_tray ui/example_tray.cc)

# Utility examples
add_example_executable(example_dimension_concepts utility/example_dimension_concepts.cc)
add_example_executable(example_external_geometry utility/example_external_geometry.cc)
add_example_executable(example_simple_external_geometry utility/example_simple_external_geometry.cc)
add_example_executable(example_type_safe_dimensions utility/example_type_safe_dimensions.cc)
add_example_executable(example_guid utility/example_guid.cc)

# Config examples
add_example_executable(example_hints config/example_hints.cc)
add_example_executable(example_properties config/example_properties.cc)

# Detail examples
add_example_executable(example_formatters detail/example_formatters.cc)
add_example_executable(example_string_utils detail/example_string_utils.cc)

# Tests subdirectory
add_subdirectory(tests)