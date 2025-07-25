#include <doctest/doctest.h>
#include <sdlpp/events/events.hh>
#include <string>

TEST_SUITE("event_category") {
    using namespace sdlpp;

    TEST_CASE("event categorization") {
        SUBCASE("application events") {
            CHECK(get_event_category(event_type::quit) == event_category::application);
            CHECK(get_event_category(event_type::terminating) == event_category::application);
            CHECK(get_event_category(event_type::low_memory) == event_category::application);
            CHECK(get_event_category(event_type::will_enter_background) == event_category::application);
            CHECK(get_event_category(event_type::did_enter_background) == event_category::application);
            CHECK(get_event_category(event_type::will_enter_foreground) == event_category::application);
            CHECK(get_event_category(event_type::did_enter_foreground) == event_category::application);
            CHECK(get_event_category(event_type::locale_changed) == event_category::application);
            CHECK(get_event_category(event_type::system_theme_changed) == event_category::application);
        }

        SUBCASE("window events") {
            CHECK(get_event_category(event_type::window_shown) == event_category::window);
            CHECK(get_event_category(event_type::window_hidden) == event_category::window);
            CHECK(get_event_category(event_type::window_exposed) == event_category::window);
            CHECK(get_event_category(event_type::window_moved) == event_category::window);
            CHECK(get_event_category(event_type::window_resized) == event_category::window);
            CHECK(get_event_category(event_type::window_minimized) == event_category::window);
            CHECK(get_event_category(event_type::window_maximized) == event_category::window);
            CHECK(get_event_category(event_type::window_restored) == event_category::window);
            CHECK(get_event_category(event_type::window_focus_gained) == event_category::window);
            CHECK(get_event_category(event_type::window_focus_lost) == event_category::window);
            CHECK(get_event_category(event_type::window_close_requested) == event_category::window);
        }

        SUBCASE("keyboard events") {
            CHECK(get_event_category(event_type::key_down) == event_category::keyboard);
            CHECK(get_event_category(event_type::key_up) == event_category::keyboard);
            CHECK(get_event_category(event_type::text_editing) == event_category::keyboard);
            CHECK(get_event_category(event_type::text_input) == event_category::keyboard);
            CHECK(get_event_category(event_type::keymap_changed) == event_category::keyboard);
            CHECK(get_event_category(event_type::keyboard_added) == event_category::keyboard);
            CHECK(get_event_category(event_type::keyboard_removed) == event_category::keyboard);
            CHECK(get_event_category(event_type::text_editing_candidates) == event_category::keyboard);
        }

        SUBCASE("mouse events") {
            CHECK(get_event_category(event_type::mouse_motion) == event_category::mouse);
            CHECK(get_event_category(event_type::mouse_button_down) == event_category::mouse);
            CHECK(get_event_category(event_type::mouse_button_up) == event_category::mouse);
            CHECK(get_event_category(event_type::mouse_wheel) == event_category::mouse);
            CHECK(get_event_category(event_type::mouse_added) == event_category::mouse);
            CHECK(get_event_category(event_type::mouse_removed) == event_category::mouse);
        }

        SUBCASE("joystick events") {
            CHECK(get_event_category(event_type::joystick_axis_motion) == event_category::joystick);
            CHECK(get_event_category(event_type::joystick_ball_motion) == event_category::joystick);
            CHECK(get_event_category(event_type::joystick_hat_motion) == event_category::joystick);
            CHECK(get_event_category(event_type::joystick_button_down) == event_category::joystick);
            CHECK(get_event_category(event_type::joystick_button_up) == event_category::joystick);
            CHECK(get_event_category(event_type::joystick_added) == event_category::joystick);
            CHECK(get_event_category(event_type::joystick_removed) == event_category::joystick);
            CHECK(get_event_category(event_type::joystick_battery_updated) == event_category::joystick);
            CHECK(get_event_category(event_type::joystick_update_complete) == event_category::joystick);
        }

        SUBCASE("gamepad events") {
            CHECK(get_event_category(event_type::gamepad_axis_motion) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_button_down) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_button_up) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_added) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_removed) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_remapped) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_touchpad_down) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_touchpad_motion) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_touchpad_up) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_sensor_update) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_update_complete) == event_category::gamepad);
            CHECK(get_event_category(event_type::gamepad_steam_handle_updated) == event_category::gamepad);
        }

        SUBCASE("touch events") {
            CHECK(get_event_category(event_type::finger_down) == event_category::touch);
            CHECK(get_event_category(event_type::finger_up) == event_category::touch);
            CHECK(get_event_category(event_type::finger_motion) == event_category::touch);
        }

        SUBCASE("pen events") {
            CHECK(get_event_category(event_type::pen_proximity_in) == event_category::pen);
            CHECK(get_event_category(event_type::pen_proximity_out) == event_category::pen);
            CHECK(get_event_category(event_type::pen_down) == event_category::pen);
            CHECK(get_event_category(event_type::pen_up) == event_category::pen);
            CHECK(get_event_category(event_type::pen_button_down) == event_category::pen);
            CHECK(get_event_category(event_type::pen_button_up) == event_category::pen);
            CHECK(get_event_category(event_type::pen_motion) == event_category::pen);
            CHECK(get_event_category(event_type::pen_axis) == event_category::pen);
        }

        SUBCASE("clipboard events") {
            CHECK(get_event_category(event_type::clipboard_update) == event_category::clipboard);
        }

        SUBCASE("drop events") {
            CHECK(get_event_category(event_type::drop_file) == event_category::drop);
            CHECK(get_event_category(event_type::drop_text) == event_category::drop);
            CHECK(get_event_category(event_type::drop_begin) == event_category::drop);
            CHECK(get_event_category(event_type::drop_complete) == event_category::drop);
            CHECK(get_event_category(event_type::drop_position) == event_category::drop);
        }

        SUBCASE("audio events") {
            CHECK(get_event_category(event_type::audio_device_added) == event_category::audio);
            CHECK(get_event_category(event_type::audio_device_removed) == event_category::audio);
            CHECK(get_event_category(event_type::audio_device_format_changed) == event_category::audio);
        }

        SUBCASE("sensor events") {
            CHECK(get_event_category(event_type::sensor_update) == event_category::sensor);
        }

        SUBCASE("camera events") {
            CHECK(get_event_category(event_type::camera_device_added) == event_category::camera);
            CHECK(get_event_category(event_type::camera_device_removed) == event_category::camera);
            CHECK(get_event_category(event_type::camera_device_approved) == event_category::camera);
            CHECK(get_event_category(event_type::camera_device_denied) == event_category::camera);
        }

        SUBCASE("display events") {
            CHECK(get_event_category(event_type::display_orientation) == event_category::display);
            CHECK(get_event_category(event_type::display_added) == event_category::display);
            CHECK(get_event_category(event_type::display_removed) == event_category::display);
            CHECK(get_event_category(event_type::display_moved) == event_category::display);
            CHECK(get_event_category(event_type::display_desktop_mode_changed) == event_category::display);
            CHECK(get_event_category(event_type::display_current_mode_changed) == event_category::display);
            CHECK(get_event_category(event_type::display_content_scale_changed) == event_category::display);
        }

        SUBCASE("render events") {
            CHECK(get_event_category(event_type::render_targets_reset) == event_category::render);
            CHECK(get_event_category(event_type::render_device_reset) == event_category::render);
            CHECK(get_event_category(event_type::render_device_lost) == event_category::render);
        }

        SUBCASE("user events") {
            CHECK(get_event_category(event_type::user) == event_category::user);
        }
    }

    TEST_CASE("event category utilities") {
        SUBCASE("is_event_in_category") {
            CHECK(is_event_in_category(event_type::quit, event_category::application));
            CHECK(is_event_in_category(event_type::key_down, event_category::keyboard));
            CHECK(is_event_in_category(event_type::mouse_motion, event_category::mouse));
            
            CHECK_FALSE(is_event_in_category(event_type::quit, event_category::window));
            CHECK_FALSE(is_event_in_category(event_type::key_down, event_category::mouse));
        }

        SUBCASE("is_input_category") {
            CHECK(is_input_category(event_category::keyboard));
            CHECK(is_input_category(event_category::mouse));
            CHECK(is_input_category(event_category::joystick));
            CHECK(is_input_category(event_category::gamepad));
            CHECK(is_input_category(event_category::touch));
            CHECK(is_input_category(event_category::pen));
            
            CHECK_FALSE(is_input_category(event_category::application));
            CHECK_FALSE(is_input_category(event_category::window));
            CHECK_FALSE(is_input_category(event_category::audio));
        }

        SUBCASE("is_input_event") {
            CHECK(is_input_event(event_type::key_down));
            CHECK(is_input_event(event_type::mouse_motion));
            CHECK(is_input_event(event_type::joystick_button_down));
            CHECK(is_input_event(event_type::gamepad_axis_motion));
            CHECK(is_input_event(event_type::finger_down));
            CHECK(is_input_event(event_type::pen_motion));
            
            CHECK_FALSE(is_input_event(event_type::quit));
            CHECK_FALSE(is_input_event(event_type::window_resized));
            CHECK_FALSE(is_input_event(event_type::audio_device_added));
        }

        SUBCASE("is_device_category") {
            CHECK(is_device_category(event_category::audio));
            CHECK(is_device_category(event_category::camera));
            CHECK(is_device_category(event_category::display));
            
            CHECK_FALSE(is_device_category(event_category::keyboard));
            CHECK_FALSE(is_device_category(event_category::window));
        }

        SUBCASE("is_device_event") {
            CHECK(is_device_event(event_type::audio_device_added));
            CHECK(is_device_event(event_type::camera_device_removed));
            CHECK(is_device_event(event_type::display_added));
            
            CHECK_FALSE(is_device_event(event_type::key_down));
            CHECK_FALSE(is_device_event(event_type::window_resized));
        }

        SUBCASE("event_category_to_string") {
            CHECK(std::string(event_category_to_string(event_category::application)) == "application");
            CHECK(std::string(event_category_to_string(event_category::window)) == "window");
            CHECK(std::string(event_category_to_string(event_category::keyboard)) == "keyboard");
            CHECK(std::string(event_category_to_string(event_category::mouse)) == "mouse");
            CHECK(std::string(event_category_to_string(event_category::joystick)) == "joystick");
            CHECK(std::string(event_category_to_string(event_category::gamepad)) == "gamepad");
            CHECK(std::string(event_category_to_string(event_category::touch)) == "touch");
            CHECK(std::string(event_category_to_string(event_category::pen)) == "pen");
            CHECK(std::string(event_category_to_string(event_category::clipboard)) == "clipboard");
            CHECK(std::string(event_category_to_string(event_category::drop)) == "drop");
            CHECK(std::string(event_category_to_string(event_category::audio)) == "audio");
            CHECK(std::string(event_category_to_string(event_category::sensor)) == "sensor");
            CHECK(std::string(event_category_to_string(event_category::camera)) == "camera");
            CHECK(std::string(event_category_to_string(event_category::display)) == "display");
            CHECK(std::string(event_category_to_string(event_category::render)) == "render");
            CHECK(std::string(event_category_to_string(event_category::user)) == "user");
            CHECK(std::string(event_category_to_string(event_category::unknown)) == "unknown");
        }
    }
}