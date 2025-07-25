add_executable(unittest
        main.cc
        
        # Core tests
        core/test_core_enum_operators.cc
        core/test_error.cc
        core/test_log.cc
        core/test_failsafe_backend.cc
        core/test_time.cc
        core/test_timer.cc
        core/test_timer_enum_operators.cc
        core/test_version.cc
        
        # Video tests
        video/test_video_enum_operators.cc
        video/test_window.cc
        video/test_window_geometry.cc
        video/test_renderer.cc
        video/test_renderer_geometry.cc
        video/test_surface.cc
        video/test_color.cc
        video/test_display.cc
        video/test_gl.cc
        video/test_gpu.cc
        video/test_palette.cc
        video/test_pixels.cc
        video/test_camera.cc
        
        # Audio tests
        audio/test_audio.cc
        audio/test_audio_enum_operators.cc
        
        # Events tests
        events/test_events.cc
        events/test_events_enum_operators.cc
        events/test_event_category.cc
        
        # Input tests
        input/test_input_enum_operators.cc
        input/test_gamepad.cc
        input/test_haptic.cc
        input/test_hidapi.cc
        input/test_joystick.cc
        input/test_keyboard.cc
        input/test_mouse.cc
        input/test_pen.cc
        input/test_sensor.cc
        input/test_touch.cc
        
        # IO tests
        io/test_filesystem.cc
        io/test_filesystem_enum_operators.cc
        io/test_io_common.cc
        io/test_iostream.cc
        io/test_storage.cc
        
        # System tests
        system/test_system_enum_operators.cc
        system/test_clipboard.cc
        system/test_cpu.cc
        system/test_intrinsics.cc
        system/test_locale.cc
        system/test_misc.cc
        system/test_platform.cc
        system/test_platform_enhanced.cc
        system/test_process.cc
        system/test_shared_object.cc
        
        # UI tests
        ui/test_ui_enum_operators.cc
        ui/test_dialog.cc
        ui/test_message_box.cc
        ui/test_tray.cc
        
        # Config tests
        config/test_config_enum_operators.cc
        config/test_hints.cc
        config/test_hints_simple.cc
        config/test_properties.cc
        
        # Utility tests
        utility/test_dimension.cc
        utility/test_dimension_concepts.cc
        utility/test_geometry.cc
        utility/test_guid.cc
        
        # General/support tests
        test_type_safety.cc
)

target_link_libraries(unittest PRIVATE sdlpp doctest::doctest)

# Apply warning flags to test executable
target_compile_options(unittest PRIVATE ${SDLPP_WARNING_FLAGS})