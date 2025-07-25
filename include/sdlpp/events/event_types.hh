#pragma once

/**
 * @file event_types.hh
 * @brief Event type wrapper definitions
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/id_types.hh>
#include <sdlpp/system/power_state.hh>
#include <sdlpp/input/input_id_types.hh>
#include <sdlpp/input/sensor_types.hh>
#include <sdlpp/input/pen_types.hh>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sdlpp {
    // Forward declarations
    enum class event_type : Uint32;
    // keycode is a type alias, not an enum
    // scancode is already defined in keyboard_codes.hh
    // keymod is already defined in keyboard_codes.hh
    // mouse_button is already defined in mouse_codes.hh
    // mouse_button_mask is already defined in mouse_codes.hh
    // mouse_wheel_direction is already defined in mouse_codes.hh
    
    // Re-export the raw audio device ID type for events
    // The opaque audio_device_id class is in audio/audio.hh
    using audio_device_id = audio_device_id_raw;

    /**
     * @brief Common event data
     */
    struct common_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
    };

    /**
     * @brief Application quit event
     */
    struct quit_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
    };

    /**
     * @brief Window event
     */
    struct window_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        Sint32 data1 = 0;
        Sint32 data2 = 0;

        [[nodiscard]] bool is_shown() const noexcept { return type == event_type::window_shown; }
        [[nodiscard]] bool is_hidden() const noexcept { return type == event_type::window_hidden; }
        [[nodiscard]] bool is_exposed() const noexcept { return type == event_type::window_exposed; }
        [[nodiscard]] bool is_moved() const noexcept { return type == event_type::window_moved; }
        [[nodiscard]] bool is_resized() const noexcept { return type == event_type::window_resized; }
        [[nodiscard]] bool is_minimized() const noexcept { return type == event_type::window_minimized; }
        [[nodiscard]] bool is_maximized() const noexcept { return type == event_type::window_maximized; }
        [[nodiscard]] bool is_restored() const noexcept { return type == event_type::window_restored; }
        [[nodiscard]] bool is_mouse_entered() const noexcept { return type == event_type::window_mouse_enter; }
        [[nodiscard]] bool is_mouse_left() const noexcept { return type == event_type::window_mouse_leave; }
        [[nodiscard]] bool is_focus_gained() const noexcept { return type == event_type::window_focus_gained; }
        [[nodiscard]] bool is_focus_lost() const noexcept { return type == event_type::window_focus_lost; }
        [[nodiscard]] bool is_close_requested() const noexcept { return type == event_type::window_close_requested; }

        // Position for moved events
        [[nodiscard]] Sint32 x() const noexcept { return data1; }
        [[nodiscard]] Sint32 y() const noexcept { return data2; }

        // Size for resized events
        [[nodiscard]] Sint32 width() const noexcept { return data1; }
        [[nodiscard]] Sint32 height() const noexcept { return data2; }
    };

    /**
     * @brief Keyboard device event
     */
    struct keyboard_device_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        keyboard_id which = 0;

        [[nodiscard]] bool is_added() const noexcept { return type == event_type::keyboard_added; }
        [[nodiscard]] bool is_removed() const noexcept { return type == event_type::keyboard_removed; }
    };

    /**
     * @brief Keyboard key event
     */
    struct keyboard_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        keyboard_id which = 0;
        keycode key = keycodes::unknown;
        scancode scan = scancode::unknown;
        keymod mod = keymod::none;
        Uint16 raw = 0;
        bool down = false;
        bool repeat = false;

        [[nodiscard]] bool is_pressed() const noexcept { return type == event_type::key_down; }
        [[nodiscard]] bool is_released() const noexcept { return type == event_type::key_up; }
        [[nodiscard]] bool is_repeat() const noexcept { return repeat; }

        [[nodiscard]] scancode get_scancode() const noexcept {
            return scan;
        }

        [[nodiscard]] keycode get_keycode() const noexcept {
            return key;
        }

        [[nodiscard]] keymod get_modifiers() const noexcept {
            return mod;
        }
    };

    /**
     * @brief Text editing event
     */
    struct text_editing_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        std::string text;
        Sint32 start = 0;
        Sint32 length = 0;
        
        // Internal use only - for conversion from SDL event
        void set_text_from_sdl(const char* sdl_text) {
            text = sdl_text ? sdl_text : "";
        }
    };

    /**
     * @brief Text editing candidates event
     */
    struct text_editing_candidates_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        std::vector<std::string> candidates;
        Sint32 selected_candidate = 0;
        bool horizontal = false;
        
        // Internal use only - for conversion from SDL event
        void set_candidates_from_sdl(const char* const* sdl_candidates, Sint32 count) {
            candidates.clear();
            if (sdl_candidates && count > 0) {
                candidates.reserve(static_cast<size_t>(count));
                for (Sint32 i = 0; i < count; ++i) {
                    candidates.emplace_back(sdl_candidates[i] ? sdl_candidates[i] : "");
                }
            }
        }
    };

    /**
     * @brief Text input event
     */
    struct text_input_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        std::string text;

        [[nodiscard]] std::string_view get_text() const noexcept {
            return text;
        }
        
        // Internal use only - for conversion from SDL event
        void set_text_from_sdl(const char* sdl_text) {
            text = sdl_text ? sdl_text : "";
        }
    };

    /**
     * @brief Mouse device event
     */
    struct mouse_device_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        mouse_id which = 0;

        [[nodiscard]] bool is_added() const noexcept { return type == event_type::mouse_added; }
        [[nodiscard]] bool is_removed() const noexcept { return type == event_type::mouse_removed; }
    };

    /**
     * @brief Mouse motion event
     */
    struct mouse_motion_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        mouse_id which = 0;
        mouse_button_mask state = mouse_button_mask::none;
        float x = 0.0f;
        float y = 0.0f;
        float xrel = 0.0f;
        float yrel = 0.0f;

        [[nodiscard]] mouse_button_mask get_button_state() const noexcept {
            return state;
        }
    };

    /**
     * @brief Mouse button event
     */
    struct mouse_button_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        mouse_id which = 0;
        Uint8 button = 0;
        bool down = false;
        Uint8 clicks = 0;
        float x = 0.0f;
        float y = 0.0f;

        [[nodiscard]] bool is_pressed() const noexcept { return type == event_type::mouse_button_down; }
        [[nodiscard]] bool is_released() const noexcept { return type == event_type::mouse_button_up; }
        [[nodiscard]] bool is_double_click() const noexcept { return clicks == 2; }

        [[nodiscard]] mouse_button get_button() const noexcept {
            return static_cast <mouse_button>(button);
        }
    };

    /**
     * @brief Mouse wheel event
     */
    struct mouse_wheel_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        mouse_id which = 0;
        float x = 0.0f;
        float y = 0.0f;
        mouse_wheel_direction direction = mouse_wheel_direction::normal;
        float mouse_x = 0.0f;
        float mouse_y = 0.0f;

        [[nodiscard]] mouse_wheel_direction get_direction() const noexcept {
            return direction;
        }
    };

    /**
     * @brief Joystick device event
     */
    struct joystick_device_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;

        [[nodiscard]] bool is_added() const noexcept { return type == event_type::joystick_added; }
        [[nodiscard]] bool is_removed() const noexcept { return type == event_type::joystick_removed; }
        [[nodiscard]] bool is_update_complete() const noexcept { return type == event_type::joystick_update_complete; }
    };

    /**
     * @brief Joystick axis event
     */
    struct joystick_axis_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        Uint8 axis = 0;
        Sint16 value = 0;
    };

    /**
     * @brief Joystick ball event
     */
    struct joystick_ball_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        Uint8 ball = 0;
        Sint16 xrel = 0;
        Sint16 yrel = 0;
    };

    /**
     * @brief Joystick hat event
     */
    struct joystick_hat_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        Uint8 hat = 0;
        Uint8 value = 0;
    };

    /**
     * @brief Joystick button event
     */
    struct joystick_button_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        Uint8 button = 0;
        bool down = false;

        [[nodiscard]] bool is_pressed() const noexcept { return type == event_type::joystick_button_down; }
        [[nodiscard]] bool is_released() const noexcept { return type == event_type::joystick_button_up; }
    };

    /**
     * @brief Joystick battery event
     */
    struct joystick_battery_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        power_state state = power_state::unknown;
        int percent = 0;
    };

    /**
     * @brief Gamepad device event
     */
    struct gamepad_device_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;

        [[nodiscard]] bool is_added() const noexcept { return type == event_type::gamepad_added; }
        [[nodiscard]] bool is_removed() const noexcept { return type == event_type::gamepad_removed; }
        [[nodiscard]] bool is_remapped() const noexcept { return type == event_type::gamepad_remapped; }
        [[nodiscard]] bool is_update_complete() const noexcept { return type == event_type::gamepad_update_complete; }

        [[nodiscard]] bool is_steam_handle_updated() const noexcept {
            return type == event_type::gamepad_steam_handle_updated;
        }
    };

    /**
     * @brief Gamepad axis event
     */
    struct gamepad_axis_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        Uint8 axis = 0;
        Sint16 value = 0;
    };

    /**
     * @brief Gamepad button event
     */
    struct gamepad_button_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        Uint8 button = 0;
        bool down = false;

        [[nodiscard]] bool is_pressed() const noexcept { return type == event_type::gamepad_button_down; }
        [[nodiscard]] bool is_released() const noexcept { return type == event_type::gamepad_button_up; }
    };

    /**
     * @brief Gamepad touchpad event
     */
    struct gamepad_touchpad_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        Sint32 touchpad = 0;
        Sint32 finger = 0;
        float x = 0.0f;
        float y = 0.0f;
        float pressure = 0.0f;

        [[nodiscard]] bool is_down() const noexcept { return type == event_type::gamepad_touchpad_down; }
        [[nodiscard]] bool is_motion() const noexcept { return type == event_type::gamepad_touchpad_motion; }
        [[nodiscard]] bool is_up() const noexcept { return type == event_type::gamepad_touchpad_up; }
    };

    /**
     * @brief Gamepad sensor event
     */
    struct gamepad_sensor_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        joystick_id which = 0;
        sensor_type sensor = sensor_type::invalid;
        float data[3] = {0.0f, 0.0f, 0.0f};
        Uint64 sensor_timestamp = 0;
    };

    /**
     * @brief Audio device event
     */
    struct audio_device_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        audio_device_id which = 0;
        bool recording = false;

        [[nodiscard]] bool is_added() const noexcept { return type == event_type::audio_device_added; }
        [[nodiscard]] bool is_removed() const noexcept { return type == event_type::audio_device_removed; }

        [[nodiscard]] bool is_format_changed() const noexcept {
            return type == event_type::audio_device_format_changed;
        }

        [[nodiscard]] bool is_playback() const noexcept { return !recording; }
        [[nodiscard]] bool is_recording() const noexcept { return recording; }
    };

    /**
     * @brief Camera device event
     */
    struct camera_device_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        camera_id which = 0;

        [[nodiscard]] bool is_added() const noexcept { return type == event_type::camera_device_added; }
        [[nodiscard]] bool is_removed() const noexcept { return type == event_type::camera_device_removed; }
        [[nodiscard]] bool is_approved() const noexcept { return type == event_type::camera_device_approved; }
        [[nodiscard]] bool is_denied() const noexcept { return type == event_type::camera_device_denied; }
    };

    /**
     * @brief Sensor event
     */
    struct sensor_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        sensor_id which = 0;
        float data[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        Uint64 sensor_timestamp = 0;
    };

    /**
     * @brief Touch finger event
     */
    struct touch_finger_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        touch_id touchID = 0;
        finger_id fingerID = 0;
        float x = 0.0f;
        float y = 0.0f;
        float dx = 0.0f;
        float dy = 0.0f;
        float pressure = 0.0f;

        [[nodiscard]] bool is_down() const noexcept { return type == event_type::finger_down; }
        [[nodiscard]] bool is_up() const noexcept { return type == event_type::finger_up; }
        [[nodiscard]] bool is_motion() const noexcept { return type == event_type::finger_motion; }
    };

    /**
     * @brief Pen proximity event
     */
    struct pen_proximity_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        pen_id which = 0;

        [[nodiscard]] bool is_in() const noexcept { return type == event_type::pen_proximity_in; }
        [[nodiscard]] bool is_out() const noexcept { return type == event_type::pen_proximity_out; }
    };

    /**
     * @brief Pen touch event
     */
    struct pen_touch_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        pen_id which = 0;
        pen_input_flags pen_state = pen_input_flags::none;
        float x = 0.0f;
        float y = 0.0f;
        bool eraser = false;
        bool down = false;

        [[nodiscard]] bool is_down() const noexcept { return type == event_type::pen_down; }
        [[nodiscard]] bool is_up() const noexcept { return type == event_type::pen_up; }
    };

    /**
     * @brief Pen motion event
     */
    struct pen_motion_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        pen_id which = 0;
        pen_input_flags pen_state = pen_input_flags::none;
        float x = 0.0f;
        float y = 0.0f;
    };

    /**
     * @brief Pen button event
     */
    struct pen_button_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        pen_id which = 0;
        pen_input_flags pen_state = pen_input_flags::none;
        float x = 0.0f;
        float y = 0.0f;
        Uint8 button = 0;
        bool down = false;

        [[nodiscard]] bool is_pressed() const noexcept { return type == event_type::pen_button_down; }
        [[nodiscard]] bool is_released() const noexcept { return type == event_type::pen_button_up; }
    };

    /**
     * @brief Pen axis event
     */
    struct pen_axis_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        pen_id which = 0;
        pen_input_flags pen_state = pen_input_flags::none;
        float x = 0.0f;
        float y = 0.0f;
        pen_axis axis = static_cast<pen_axis>(0);
        float value = 0.0f;
    };

    /**
     * @brief Drop event
     */
    struct drop_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        float x = 0.0f;
        float y = 0.0f;
        std::string source;
        std::string data;

        [[nodiscard]] bool is_file() const noexcept { return type == event_type::drop_file; }
        [[nodiscard]] bool is_text() const noexcept { return type == event_type::drop_text; }
        [[nodiscard]] bool is_begin() const noexcept { return type == event_type::drop_begin; }
        [[nodiscard]] bool is_complete() const noexcept { return type == event_type::drop_complete; }
        [[nodiscard]] bool is_position() const noexcept { return type == event_type::drop_position; }

        [[nodiscard]] std::string_view get_source() const noexcept {
            return source;
        }

        [[nodiscard]] std::string_view get_data() const noexcept {
            return data;
        }
        
        // Internal use only - for conversion from SDL event
        void set_source_from_sdl(const char* sdl_source) {
            source = sdl_source ? sdl_source : "";
        }
        
        void set_data_from_sdl(const char* sdl_data) {
            data = sdl_data ? sdl_data : "";
        }
    };

    /**
     * @brief Clipboard event
     */
    struct clipboard_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        bool owner = false;
    };

    /**
     * @brief Display event
     */
    struct display_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        display_id displayID = 0;
        Sint32 data1 = 0;
        Sint32 data2 = 0;
    };

    /**
     * @brief Render event
     */
    struct render_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;

        [[nodiscard]] bool is_targets_reset() const noexcept { return type == event_type::render_targets_reset; }
        [[nodiscard]] bool is_device_reset() const noexcept { return type == event_type::render_device_reset; }
        [[nodiscard]] bool is_device_lost() const noexcept { return type == event_type::render_device_lost; }
    };

    /**
     * @brief User-defined event
     */
    struct user_event {
        event_type type = event_type::first_event;
        Uint64 timestamp = 0;
        window_id windowID = 0;
        Sint32 code = 0;
        void* data1 = nullptr;
        void* data2 = nullptr;
    };
} // namespace sdlpp
