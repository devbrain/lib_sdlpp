#include <sdlpp/events/events.hh>
#include <sdlpp/detail/type_utils.hh>

namespace sdlpp {
    // Event queue implementation
    std::optional <event> event_queue::poll() {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            return event(std::move(e));
        }
        return std::nullopt;
    }

    expected <event, std::string> event_queue::wait() {
        SDL_Event e;
        if (SDL_WaitEvent(&e)) {
            return event(std::move(e));
        }
        return unexpected(get_error());
    }

    expected <event, std::string> event_queue::wait_timeout(std::chrono::milliseconds timeout) {
        SDL_Event e;
        if (SDL_WaitEventTimeout(&e, static_cast <Sint32>(timeout.count()))) {
            return event(std::move(e));
        }
        return unexpected(get_error());
    }

    expected <void, std::string> event_queue::push(const event& e) {
        SDL_Event copy = e.raw();
        if (SDL_PushEvent(&copy)) {
            return {};
        }
        return unexpected(get_error());
    }

    void event_queue::pump() {
        SDL_PumpEvents();
    }

    void event_queue::flush(event_type type) {
        SDL_FlushEvent(static_cast <Uint32>(type));
    }

    void event_queue::flush_range(event_type min_type, event_type max_type) {
        SDL_FlushEvents(static_cast <Uint32>(min_type), static_cast <Uint32>(max_type));
    }

    bool event_queue::has_event(event_type type) {
        return SDL_HasEvent(static_cast <Uint32>(type));
    }

    bool event_queue::has_events(event_type min_type, event_type max_type) {
        return SDL_HasEvents(static_cast <Uint32>(min_type), static_cast <Uint32>(max_type));
    }

    // Event filter implementation
    event_filter::event_filter(filter_func func, void* userdata)
        : func_(func), userdata_(userdata) {
        SDL_GetEventFilter(&prev_func_, &prev_userdata_);
        SDL_SetEventFilter(func_, userdata_);
    }

    event_filter::~event_filter() {
        if (func_) {
            SDL_SetEventFilter(prev_func_, prev_userdata_);
        }
    }

    event_filter::event_filter(event_filter&& other) noexcept
        : func_(std::exchange(other.func_, nullptr)),
          userdata_(std::exchange(other.userdata_, nullptr)),
          prev_func_(std::exchange(other.prev_func_, nullptr)),
          prev_userdata_(std::exchange(other.prev_userdata_, nullptr)) {
    }

    event_filter& event_filter::operator=(event_filter&& other) noexcept {
        if (this != &other) {
            if (func_) {
                SDL_SetEventFilter(prev_func_, prev_userdata_);
            }
            func_ = std::exchange(other.func_, nullptr);
            userdata_ = std::exchange(other.userdata_, nullptr);
            prev_func_ = std::exchange(other.prev_func_, nullptr);
            prev_userdata_ = std::exchange(other.prev_userdata_, nullptr);
        }
        return *this;
    }

    // Event watcher implementation
    event_watcher::event_watcher(watcher_func func, void* userdata)
        : func_(func), userdata_(userdata) {
        SDL_AddEventWatch(func_, userdata_);
    }

    event_watcher::~event_watcher() {
        if (func_) {
            SDL_RemoveEventWatch(func_, userdata_);
        }
    }

    event_watcher::event_watcher(event_watcher&& other) noexcept
        : func_(std::exchange(other.func_, nullptr)),
          userdata_(std::exchange(other.userdata_, nullptr)) {
    }

    event_watcher& event_watcher::operator=(event_watcher&& other) noexcept {
        if (this != &other) {
            if (func_) {
                SDL_RemoveEventWatch(func_, userdata_);
            }
            func_ = std::exchange(other.func_, nullptr);
            userdata_ = std::exchange(other.userdata_, nullptr);
        }
        return *this;
    }

    // Event registry implementation
    expected <Uint32, std::string> event_registry::register_events(size_t count) {
        auto int_count = detail::size_to_int(count);
        if (!int_count) {
            return unexpected("Event count too large: " + int_count.error());
        }
        Uint32 event_id = SDL_RegisterEvents(*int_count);
        if (event_id == static_cast <Uint32>(-1)) {
            return unexpected("Failed to register events");
        }
        return event_id;
    }

    bool event_registry::is_custom(event_type type) noexcept {
        return is_custom(static_cast <Uint32>(type));
    }

    bool event_registry::is_custom(Uint32 type) noexcept {
        return type >= SDL_EVENT_USER && type <= SDL_EVENT_LAST;
    }
} // namespace sdlpp

// Auto-generated enum stream operators
namespace sdlpp {
    std::ostream& operator<<(std::ostream& os, event_type value) {
        switch (value) {
            case event_type::first_event:
                return os << "first_event";
            case event_type::quit:
                return os << "quit";
            case event_type::terminating:
                return os << "terminating";
            case event_type::low_memory:
                return os << "low_memory";
            case event_type::will_enter_background:
                return os << "will_enter_background";
            case event_type::did_enter_background:
                return os << "did_enter_background";
            case event_type::will_enter_foreground:
                return os << "will_enter_foreground";
            case event_type::did_enter_foreground:
                return os << "did_enter_foreground";
            case event_type::locale_changed:
                return os << "locale_changed";
            case event_type::system_theme_changed:
                return os << "system_theme_changed";
            case event_type::display_orientation:
                return os << "display_orientation";
            case event_type::display_added:
                return os << "display_added";
            case event_type::display_removed:
                return os << "display_removed";
            case event_type::display_moved:
                return os << "display_moved";
            case event_type::display_desktop_mode_changed:
                return os << "display_desktop_mode_changed";
            case event_type::display_current_mode_changed:
                return os << "display_current_mode_changed";
            case event_type::display_content_scale_changed:
                return os << "display_content_scale_changed";
            case event_type::window_shown:
                return os << "window_shown";
            case event_type::window_hidden:
                return os << "window_hidden";
            case event_type::window_exposed:
                return os << "window_exposed";
            case event_type::window_moved:
                return os << "window_moved";
            case event_type::window_resized:
                return os << "window_resized";
            case event_type::window_pixel_size_changed:
                return os << "window_pixel_size_changed";
            case event_type::window_metal_view_resized:
                return os << "window_metal_view_resized";
            case event_type::window_minimized:
                return os << "window_minimized";
            case event_type::window_maximized:
                return os << "window_maximized";
            case event_type::window_restored:
                return os << "window_restored";
            case event_type::window_mouse_enter:
                return os << "window_mouse_enter";
            case event_type::window_mouse_leave:
                return os << "window_mouse_leave";
            case event_type::window_focus_gained:
                return os << "window_focus_gained";
            case event_type::window_focus_lost:
                return os << "window_focus_lost";
            case event_type::window_close_requested:
                return os << "window_close_requested";
            case event_type::window_hit_test:
                return os << "window_hit_test";
            case event_type::window_iccprof_changed:
                return os << "window_iccprof_changed";
            case event_type::window_display_changed:
                return os << "window_display_changed";
            case event_type::window_display_scale_changed:
                return os << "window_display_scale_changed";
            case event_type::window_safe_area_changed:
                return os << "window_safe_area_changed";
            case event_type::window_occluded:
                return os << "window_occluded";
            case event_type::window_enter_fullscreen:
                return os << "window_enter_fullscreen";
            case event_type::window_leave_fullscreen:
                return os << "window_leave_fullscreen";
            case event_type::window_destroyed:
                return os << "window_destroyed";
            case event_type::window_hdr_state_changed:
                return os << "window_hdr_state_changed";
            case event_type::key_down:
                return os << "key_down";
            case event_type::key_up:
                return os << "key_up";
            case event_type::text_editing:
                return os << "text_editing";
            case event_type::text_input:
                return os << "text_input";
            case event_type::keymap_changed:
                return os << "keymap_changed";
            case event_type::keyboard_added:
                return os << "keyboard_added";
            case event_type::keyboard_removed:
                return os << "keyboard_removed";
            case event_type::text_editing_candidates:
                return os << "text_editing_candidates";
            case event_type::mouse_motion:
                return os << "mouse_motion";
            case event_type::mouse_button_down:
                return os << "mouse_button_down";
            case event_type::mouse_button_up:
                return os << "mouse_button_up";
            case event_type::mouse_wheel:
                return os << "mouse_wheel";
            case event_type::mouse_added:
                return os << "mouse_added";
            case event_type::mouse_removed:
                return os << "mouse_removed";
            case event_type::joystick_axis_motion:
                return os << "joystick_axis_motion";
            case event_type::joystick_ball_motion:
                return os << "joystick_ball_motion";
            case event_type::joystick_hat_motion:
                return os << "joystick_hat_motion";
            case event_type::joystick_button_down:
                return os << "joystick_button_down";
            case event_type::joystick_button_up:
                return os << "joystick_button_up";
            case event_type::joystick_added:
                return os << "joystick_added";
            case event_type::joystick_removed:
                return os << "joystick_removed";
            case event_type::joystick_battery_updated:
                return os << "joystick_battery_updated";
            case event_type::joystick_update_complete:
                return os << "joystick_update_complete";
            case event_type::gamepad_axis_motion:
                return os << "gamepad_axis_motion";
            case event_type::gamepad_button_down:
                return os << "gamepad_button_down";
            case event_type::gamepad_button_up:
                return os << "gamepad_button_up";
            case event_type::gamepad_added:
                return os << "gamepad_added";
            case event_type::gamepad_removed:
                return os << "gamepad_removed";
            case event_type::gamepad_remapped:
                return os << "gamepad_remapped";
            case event_type::gamepad_touchpad_down:
                return os << "gamepad_touchpad_down";
            case event_type::gamepad_touchpad_motion:
                return os << "gamepad_touchpad_motion";
            case event_type::gamepad_touchpad_up:
                return os << "gamepad_touchpad_up";
            case event_type::gamepad_sensor_update:
                return os << "gamepad_sensor_update";
            case event_type::gamepad_update_complete:
                return os << "gamepad_update_complete";
            case event_type::gamepad_steam_handle_updated:
                return os << "gamepad_steam_handle_updated";
            case event_type::finger_down:
                return os << "finger_down";
            case event_type::finger_up:
                return os << "finger_up";
            case event_type::finger_motion:
                return os << "finger_motion";
            case event_type::pen_proximity_in:
                return os << "pen_proximity_in";
            case event_type::pen_proximity_out:
                return os << "pen_proximity_out";
            case event_type::pen_down:
                return os << "pen_down";
            case event_type::pen_up:
                return os << "pen_up";
            case event_type::pen_button_down:
                return os << "pen_button_down";
            case event_type::pen_button_up:
                return os << "pen_button_up";
            case event_type::pen_motion:
                return os << "pen_motion";
            case event_type::pen_axis:
                return os << "pen_axis";
            case event_type::clipboard_update:
                return os << "clipboard_update";
            case event_type::drop_file:
                return os << "drop_file";
            case event_type::drop_text:
                return os << "drop_text";
            case event_type::drop_begin:
                return os << "drop_begin";
            case event_type::drop_complete:
                return os << "drop_complete";
            case event_type::drop_position:
                return os << "drop_position";
            case event_type::audio_device_added:
                return os << "audio_device_added";
            case event_type::audio_device_removed:
                return os << "audio_device_removed";
            case event_type::audio_device_format_changed:
                return os << "audio_device_format_changed";
            case event_type::sensor_update:
                return os << "sensor_update";
            case event_type::camera_device_added:
                return os << "camera_device_added";
            case event_type::camera_device_removed:
                return os << "camera_device_removed";
            case event_type::camera_device_approved:
                return os << "camera_device_approved";
            case event_type::camera_device_denied:
                return os << "camera_device_denied";
            case event_type::render_targets_reset:
                return os << "render_targets_reset";
            case event_type::render_device_reset:
                return os << "render_device_reset";
            case event_type::render_device_lost:
                return os << "render_device_lost";
            case event_type::user:
                return os << "user";
            case event_type::last:
                return os << "last";
            default:
                return os << "Unknown_event_type(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, event_type& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <event_type>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "first_event") {
            value = event_type::first_event;
        } else if (str == "quit") {
            value = event_type::quit;
        } else if (str == "terminating") {
            value = event_type::terminating;
        } else if (str == "low_memory") {
            value = event_type::low_memory;
        } else if (str == "will_enter_background") {
            value = event_type::will_enter_background;
        } else if (str == "did_enter_background") {
            value = event_type::did_enter_background;
        } else if (str == "will_enter_foreground") {
            value = event_type::will_enter_foreground;
        } else if (str == "did_enter_foreground") {
            value = event_type::did_enter_foreground;
        } else if (str == "locale_changed") {
            value = event_type::locale_changed;
        } else if (str == "system_theme_changed") {
            value = event_type::system_theme_changed;
        } else if (str == "display_orientation") {
            value = event_type::display_orientation;
        } else if (str == "display_added") {
            value = event_type::display_added;
        } else if (str == "display_removed") {
            value = event_type::display_removed;
        } else if (str == "display_moved") {
            value = event_type::display_moved;
        } else if (str == "display_desktop_mode_changed") {
            value = event_type::display_desktop_mode_changed;
        } else if (str == "display_current_mode_changed") {
            value = event_type::display_current_mode_changed;
        } else if (str == "display_content_scale_changed") {
            value = event_type::display_content_scale_changed;
        } else if (str == "window_shown") {
            value = event_type::window_shown;
        } else if (str == "window_hidden") {
            value = event_type::window_hidden;
        } else if (str == "window_exposed") {
            value = event_type::window_exposed;
        } else if (str == "window_moved") {
            value = event_type::window_moved;
        } else if (str == "window_resized") {
            value = event_type::window_resized;
        } else if (str == "window_pixel_size_changed") {
            value = event_type::window_pixel_size_changed;
        } else if (str == "window_metal_view_resized") {
            value = event_type::window_metal_view_resized;
        } else if (str == "window_minimized") {
            value = event_type::window_minimized;
        } else if (str == "window_maximized") {
            value = event_type::window_maximized;
        } else if (str == "window_restored") {
            value = event_type::window_restored;
        } else if (str == "window_mouse_enter") {
            value = event_type::window_mouse_enter;
        } else if (str == "window_mouse_leave") {
            value = event_type::window_mouse_leave;
        } else if (str == "window_focus_gained") {
            value = event_type::window_focus_gained;
        } else if (str == "window_focus_lost") {
            value = event_type::window_focus_lost;
        } else if (str == "window_close_requested") {
            value = event_type::window_close_requested;
        } else if (str == "window_hit_test") {
            value = event_type::window_hit_test;
        } else if (str == "window_iccprof_changed") {
            value = event_type::window_iccprof_changed;
        } else if (str == "window_display_changed") {
            value = event_type::window_display_changed;
        } else if (str == "window_display_scale_changed") {
            value = event_type::window_display_scale_changed;
        } else if (str == "window_safe_area_changed") {
            value = event_type::window_safe_area_changed;
        } else if (str == "window_occluded") {
            value = event_type::window_occluded;
        } else if (str == "window_enter_fullscreen") {
            value = event_type::window_enter_fullscreen;
        } else if (str == "window_leave_fullscreen") {
            value = event_type::window_leave_fullscreen;
        } else if (str == "window_destroyed") {
            value = event_type::window_destroyed;
        } else if (str == "window_hdr_state_changed") {
            value = event_type::window_hdr_state_changed;
        } else if (str == "key_down") {
            value = event_type::key_down;
        } else if (str == "key_up") {
            value = event_type::key_up;
        } else if (str == "text_editing") {
            value = event_type::text_editing;
        } else if (str == "text_input") {
            value = event_type::text_input;
        } else if (str == "keymap_changed") {
            value = event_type::keymap_changed;
        } else if (str == "keyboard_added") {
            value = event_type::keyboard_added;
        } else if (str == "keyboard_removed") {
            value = event_type::keyboard_removed;
        } else if (str == "text_editing_candidates") {
            value = event_type::text_editing_candidates;
        } else if (str == "mouse_motion") {
            value = event_type::mouse_motion;
        } else if (str == "mouse_button_down") {
            value = event_type::mouse_button_down;
        } else if (str == "mouse_button_up") {
            value = event_type::mouse_button_up;
        } else if (str == "mouse_wheel") {
            value = event_type::mouse_wheel;
        } else if (str == "mouse_added") {
            value = event_type::mouse_added;
        } else if (str == "mouse_removed") {
            value = event_type::mouse_removed;
        } else if (str == "joystick_axis_motion") {
            value = event_type::joystick_axis_motion;
        } else if (str == "joystick_ball_motion") {
            value = event_type::joystick_ball_motion;
        } else if (str == "joystick_hat_motion") {
            value = event_type::joystick_hat_motion;
        } else if (str == "joystick_button_down") {
            value = event_type::joystick_button_down;
        } else if (str == "joystick_button_up") {
            value = event_type::joystick_button_up;
        } else if (str == "joystick_added") {
            value = event_type::joystick_added;
        } else if (str == "joystick_removed") {
            value = event_type::joystick_removed;
        } else if (str == "joystick_battery_updated") {
            value = event_type::joystick_battery_updated;
        } else if (str == "joystick_update_complete") {
            value = event_type::joystick_update_complete;
        } else if (str == "gamepad_axis_motion") {
            value = event_type::gamepad_axis_motion;
        } else if (str == "gamepad_button_down") {
            value = event_type::gamepad_button_down;
        } else if (str == "gamepad_button_up") {
            value = event_type::gamepad_button_up;
        } else if (str == "gamepad_added") {
            value = event_type::gamepad_added;
        } else if (str == "gamepad_removed") {
            value = event_type::gamepad_removed;
        } else if (str == "gamepad_remapped") {
            value = event_type::gamepad_remapped;
        } else if (str == "gamepad_touchpad_down") {
            value = event_type::gamepad_touchpad_down;
        } else if (str == "gamepad_touchpad_motion") {
            value = event_type::gamepad_touchpad_motion;
        } else if (str == "gamepad_touchpad_up") {
            value = event_type::gamepad_touchpad_up;
        } else if (str == "gamepad_sensor_update") {
            value = event_type::gamepad_sensor_update;
        } else if (str == "gamepad_update_complete") {
            value = event_type::gamepad_update_complete;
        } else if (str == "gamepad_steam_handle_updated") {
            value = event_type::gamepad_steam_handle_updated;
        } else if (str == "finger_down") {
            value = event_type::finger_down;
        } else if (str == "finger_up") {
            value = event_type::finger_up;
        } else if (str == "finger_motion") {
            value = event_type::finger_motion;
        } else if (str == "pen_proximity_in") {
            value = event_type::pen_proximity_in;
        } else if (str == "pen_proximity_out") {
            value = event_type::pen_proximity_out;
        } else if (str == "pen_down") {
            value = event_type::pen_down;
        } else if (str == "pen_up") {
            value = event_type::pen_up;
        } else if (str == "pen_button_down") {
            value = event_type::pen_button_down;
        } else if (str == "pen_button_up") {
            value = event_type::pen_button_up;
        } else if (str == "pen_motion") {
            value = event_type::pen_motion;
        } else if (str == "pen_axis") {
            value = event_type::pen_axis;
        } else if (str == "clipboard_update") {
            value = event_type::clipboard_update;
        } else if (str == "drop_file") {
            value = event_type::drop_file;
        } else if (str == "drop_text") {
            value = event_type::drop_text;
        } else if (str == "drop_begin") {
            value = event_type::drop_begin;
        } else if (str == "drop_complete") {
            value = event_type::drop_complete;
        } else if (str == "drop_position") {
            value = event_type::drop_position;
        } else if (str == "audio_device_added") {
            value = event_type::audio_device_added;
        } else if (str == "audio_device_removed") {
            value = event_type::audio_device_removed;
        } else if (str == "audio_device_format_changed") {
            value = event_type::audio_device_format_changed;
        } else if (str == "sensor_update") {
            value = event_type::sensor_update;
        } else if (str == "camera_device_added") {
            value = event_type::camera_device_added;
        } else if (str == "camera_device_removed") {
            value = event_type::camera_device_removed;
        } else if (str == "camera_device_approved") {
            value = event_type::camera_device_approved;
        } else if (str == "camera_device_denied") {
            value = event_type::camera_device_denied;
        } else if (str == "render_targets_reset") {
            value = event_type::render_targets_reset;
        } else if (str == "render_device_reset") {
            value = event_type::render_device_reset;
        } else if (str == "render_device_lost") {
            value = event_type::render_device_lost;
        } else if (str == "user") {
            value = event_type::user;
        } else if (str == "last") {
            value = event_type::last;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <event_type>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }
} // namespace
