#pragma once

/**
 * @file events.hh
 * @brief Event system wrapper for SDL3
 * 
 * This header provides C++ wrappers around SDL3's event system, offering
 * type-safe event handling with multiple access patterns.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/expected.hh>

#include <cstdint>
#include <string>
#include <optional>
#include <variant>
#include <functional>
#include <chrono>
#include <vector>

namespace sdlpp {
    /**
     * @brief Event types enumeration
     */
    enum class event_type : Uint32 {
        // Application events
        first_event = SDL_EVENT_FIRST,
        quit = SDL_EVENT_QUIT,
        terminating = SDL_EVENT_TERMINATING,
        low_memory = SDL_EVENT_LOW_MEMORY,
        will_enter_background = SDL_EVENT_WILL_ENTER_BACKGROUND,
        did_enter_background = SDL_EVENT_DID_ENTER_BACKGROUND,
        will_enter_foreground = SDL_EVENT_WILL_ENTER_FOREGROUND,
        did_enter_foreground = SDL_EVENT_DID_ENTER_FOREGROUND,
        locale_changed = SDL_EVENT_LOCALE_CHANGED,
        system_theme_changed = SDL_EVENT_SYSTEM_THEME_CHANGED,

        // Display events
        display_orientation = SDL_EVENT_DISPLAY_ORIENTATION,
        display_added = SDL_EVENT_DISPLAY_ADDED,
        display_removed = SDL_EVENT_DISPLAY_REMOVED,
        display_moved = SDL_EVENT_DISPLAY_MOVED,
        display_desktop_mode_changed = SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED,
        display_current_mode_changed = SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED,
        display_content_scale_changed = SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED,

        // Window events
        window_shown = SDL_EVENT_WINDOW_SHOWN,
        window_hidden = SDL_EVENT_WINDOW_HIDDEN,
        window_exposed = SDL_EVENT_WINDOW_EXPOSED,
        window_moved = SDL_EVENT_WINDOW_MOVED,
        window_resized = SDL_EVENT_WINDOW_RESIZED,
        window_pixel_size_changed = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,
        window_metal_view_resized = SDL_EVENT_WINDOW_METAL_VIEW_RESIZED,
        window_minimized = SDL_EVENT_WINDOW_MINIMIZED,
        window_maximized = SDL_EVENT_WINDOW_MAXIMIZED,
        window_restored = SDL_EVENT_WINDOW_RESTORED,
        window_mouse_enter = SDL_EVENT_WINDOW_MOUSE_ENTER,
        window_mouse_leave = SDL_EVENT_WINDOW_MOUSE_LEAVE,
        window_focus_gained = SDL_EVENT_WINDOW_FOCUS_GAINED,
        window_focus_lost = SDL_EVENT_WINDOW_FOCUS_LOST,
        window_close_requested = SDL_EVENT_WINDOW_CLOSE_REQUESTED,
        window_hit_test = SDL_EVENT_WINDOW_HIT_TEST,
        window_iccprof_changed = SDL_EVENT_WINDOW_ICCPROF_CHANGED,
        window_display_changed = SDL_EVENT_WINDOW_DISPLAY_CHANGED,
        window_display_scale_changed = SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED,
        window_safe_area_changed = SDL_EVENT_WINDOW_SAFE_AREA_CHANGED,
        window_occluded = SDL_EVENT_WINDOW_OCCLUDED,
        window_enter_fullscreen = SDL_EVENT_WINDOW_ENTER_FULLSCREEN,
        window_leave_fullscreen = SDL_EVENT_WINDOW_LEAVE_FULLSCREEN,
        window_destroyed = SDL_EVENT_WINDOW_DESTROYED,
        window_hdr_state_changed = SDL_EVENT_WINDOW_HDR_STATE_CHANGED,

        // Keyboard events
        key_down = SDL_EVENT_KEY_DOWN,
        key_up = SDL_EVENT_KEY_UP,
        text_editing = SDL_EVENT_TEXT_EDITING,
        text_input = SDL_EVENT_TEXT_INPUT,
        keymap_changed = SDL_EVENT_KEYMAP_CHANGED,
        keyboard_added = SDL_EVENT_KEYBOARD_ADDED,
        keyboard_removed = SDL_EVENT_KEYBOARD_REMOVED,
        text_editing_candidates = SDL_EVENT_TEXT_EDITING_CANDIDATES,

        // Mouse events
        mouse_motion = SDL_EVENT_MOUSE_MOTION,
        mouse_button_down = SDL_EVENT_MOUSE_BUTTON_DOWN,
        mouse_button_up = SDL_EVENT_MOUSE_BUTTON_UP,
        mouse_wheel = SDL_EVENT_MOUSE_WHEEL,
        mouse_added = SDL_EVENT_MOUSE_ADDED,
        mouse_removed = SDL_EVENT_MOUSE_REMOVED,

        // Joystick events
        joystick_axis_motion = SDL_EVENT_JOYSTICK_AXIS_MOTION,
        joystick_ball_motion = SDL_EVENT_JOYSTICK_BALL_MOTION,
        joystick_hat_motion = SDL_EVENT_JOYSTICK_HAT_MOTION,
        joystick_button_down = SDL_EVENT_JOYSTICK_BUTTON_DOWN,
        joystick_button_up = SDL_EVENT_JOYSTICK_BUTTON_UP,
        joystick_added = SDL_EVENT_JOYSTICK_ADDED,
        joystick_removed = SDL_EVENT_JOYSTICK_REMOVED,
        joystick_battery_updated = SDL_EVENT_JOYSTICK_BATTERY_UPDATED,
        joystick_update_complete = SDL_EVENT_JOYSTICK_UPDATE_COMPLETE,

        // Gamepad events
        gamepad_axis_motion = SDL_EVENT_GAMEPAD_AXIS_MOTION,
        gamepad_button_down = SDL_EVENT_GAMEPAD_BUTTON_DOWN,
        gamepad_button_up = SDL_EVENT_GAMEPAD_BUTTON_UP,
        gamepad_added = SDL_EVENT_GAMEPAD_ADDED,
        gamepad_removed = SDL_EVENT_GAMEPAD_REMOVED,
        gamepad_remapped = SDL_EVENT_GAMEPAD_REMAPPED,
        gamepad_touchpad_down = SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN,
        gamepad_touchpad_motion = SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION,
        gamepad_touchpad_up = SDL_EVENT_GAMEPAD_TOUCHPAD_UP,
        gamepad_sensor_update = SDL_EVENT_GAMEPAD_SENSOR_UPDATE,
        gamepad_update_complete = SDL_EVENT_GAMEPAD_UPDATE_COMPLETE,
        gamepad_steam_handle_updated = SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED,

        // Touch events
        finger_down = SDL_EVENT_FINGER_DOWN,
        finger_up = SDL_EVENT_FINGER_UP,
        finger_motion = SDL_EVENT_FINGER_MOTION,

        // Pen events
        pen_proximity_in = SDL_EVENT_PEN_PROXIMITY_IN,
        pen_proximity_out = SDL_EVENT_PEN_PROXIMITY_OUT,
        pen_down = SDL_EVENT_PEN_DOWN,
        pen_up = SDL_EVENT_PEN_UP,
        pen_button_down = SDL_EVENT_PEN_BUTTON_DOWN,
        pen_button_up = SDL_EVENT_PEN_BUTTON_UP,
        pen_motion = SDL_EVENT_PEN_MOTION,
        pen_axis = SDL_EVENT_PEN_AXIS,

        // Clipboard events
        clipboard_update = SDL_EVENT_CLIPBOARD_UPDATE,

        // Drag and drop events
        drop_file = SDL_EVENT_DROP_FILE,
        drop_text = SDL_EVENT_DROP_TEXT,
        drop_begin = SDL_EVENT_DROP_BEGIN,
        drop_complete = SDL_EVENT_DROP_COMPLETE,
        drop_position = SDL_EVENT_DROP_POSITION,

        // Audio events
        audio_device_added = SDL_EVENT_AUDIO_DEVICE_ADDED,
        audio_device_removed = SDL_EVENT_AUDIO_DEVICE_REMOVED,
        audio_device_format_changed = SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED,

        // Sensor events
        sensor_update = SDL_EVENT_SENSOR_UPDATE,

        // Camera events
        camera_device_added = SDL_EVENT_CAMERA_DEVICE_ADDED,
        camera_device_removed = SDL_EVENT_CAMERA_DEVICE_REMOVED,
        camera_device_approved = SDL_EVENT_CAMERA_DEVICE_APPROVED,
        camera_device_denied = SDL_EVENT_CAMERA_DEVICE_DENIED,

        // Render events
        render_targets_reset = SDL_EVENT_RENDER_TARGETS_RESET,
        render_device_reset = SDL_EVENT_RENDER_DEVICE_RESET,
        render_device_lost = SDL_EVENT_RENDER_DEVICE_LOST,

        // User events
        user = SDL_EVENT_USER,
        last = SDL_EVENT_LAST
    };

    /**
     * @brief Check if event type is in a range
     */
    [[nodiscard]] inline bool is_event_type_in_range(event_type type, event_type first, event_type last) noexcept {
        auto t = static_cast <Uint32>(type);
        return t >= static_cast <Uint32>(first) && t <= static_cast <Uint32>(last);
    }
} // namespace sdlpp

// Include key/scancode definitions
#include <sdlpp/events/keyboard_codes.hh>
#include <sdlpp/events/mouse_codes.hh>

// Include event type definitions
#include <sdlpp/events/event_types.hh>
#include <sdlpp/events/event_category.hh>

namespace sdlpp {
    /**
     * @brief Base event wrapper
     */
    class event {
        public:
            // Event variant type
            using event_variant = std::variant <
                common_event,
                quit_event,
                window_event,
                keyboard_device_event,
                keyboard_event,
                text_editing_event,
                text_editing_candidates_event,
                text_input_event,
                mouse_device_event,
                mouse_motion_event,
                mouse_button_event,
                mouse_wheel_event,
                joystick_device_event,
                joystick_axis_event,
                joystick_ball_event,
                joystick_hat_event,
                joystick_button_event,
                joystick_battery_event,
                gamepad_device_event,
                gamepad_axis_event,
                gamepad_button_event,
                gamepad_touchpad_event,
                gamepad_sensor_event,
                audio_device_event,
                camera_device_event,
                sensor_event,
                touch_finger_event,
                pen_proximity_event,
                pen_touch_event,
                pen_motion_event,
                pen_button_event,
                pen_axis_event,
                drop_event,
                clipboard_event,
                display_event,
                render_event,
                user_event
            >;

        private:
            SDL_Event raw_;
            mutable std::optional <event_variant> variant_;

            void ensure_variant() const;

        public:
            // Constructors
            explicit event(const SDL_Event& e)
                : raw_(e) {
            }

            explicit event(SDL_Event&& e)
                : raw_(std::move(e)) {
            }

            // Type checking
            [[nodiscard]] event_type type() const noexcept {
                return static_cast <event_type>(raw_.type);
            }

            [[nodiscard]] Uint64 timestamp() const noexcept {
                return raw_.common.timestamp;
            }

            // Fast type test
            template<typename T>
            [[nodiscard]] bool is() const noexcept;

            // Safe cast with pointer
            template<typename T>
            [[nodiscard]] T* as() noexcept;

            template<typename T>
            [[nodiscard]] const T* as() const noexcept;

            // Direct access (user must check type first)
            [[nodiscard]] SDL_CommonEvent& common() noexcept { return raw_.common; }
            [[nodiscard]] SDL_QuitEvent& quit() noexcept { return raw_.quit; }
            [[nodiscard]] SDL_WindowEvent& window() noexcept { return raw_.window; }
            [[nodiscard]] SDL_KeyboardEvent& key() noexcept { return raw_.key; }
            [[nodiscard]] SDL_TextEditingEvent& edit() noexcept { return raw_.edit; }
            [[nodiscard]] SDL_TextEditingCandidatesEvent& edit_candidates() noexcept { return raw_.edit_candidates; }
            [[nodiscard]] SDL_TextInputEvent& text() noexcept { return raw_.text; }
            [[nodiscard]] SDL_MouseMotionEvent& motion() noexcept { return raw_.motion; }
            [[nodiscard]] SDL_MouseButtonEvent& button() noexcept { return raw_.button; }
            [[nodiscard]] SDL_MouseWheelEvent& wheel() noexcept { return raw_.wheel; }
            [[nodiscard]] SDL_JoyAxisEvent& jaxis() noexcept { return raw_.jaxis; }
            [[nodiscard]] SDL_JoyBallEvent& jball() noexcept { return raw_.jball; }
            [[nodiscard]] SDL_JoyHatEvent& jhat() noexcept { return raw_.jhat; }
            [[nodiscard]] SDL_JoyButtonEvent& jbutton() noexcept { return raw_.jbutton; }
            [[nodiscard]] SDL_JoyBatteryEvent& jbattery() noexcept { return raw_.jbattery; }
            [[nodiscard]] SDL_GamepadAxisEvent& gaxis() noexcept { return raw_.gaxis; }
            [[nodiscard]] SDL_GamepadButtonEvent& gbutton() noexcept { return raw_.gbutton; }
            [[nodiscard]] SDL_GamepadTouchpadEvent& gtouchpad() noexcept { return raw_.gtouchpad; }
            [[nodiscard]] SDL_GamepadSensorEvent& gsensor() noexcept { return raw_.gsensor; }
            [[nodiscard]] SDL_AudioDeviceEvent& adevice() noexcept { return raw_.adevice; }
            [[nodiscard]] SDL_CameraDeviceEvent& cdevice() noexcept { return raw_.cdevice; }
            [[nodiscard]] SDL_SensorEvent& sensor() noexcept { return raw_.sensor; }
            [[nodiscard]] SDL_TouchFingerEvent& tfinger() noexcept { return raw_.tfinger; }
            [[nodiscard]] SDL_PenProximityEvent& pproximity() noexcept { return raw_.pproximity; }
            [[nodiscard]] SDL_PenTouchEvent& ptouch() noexcept { return raw_.ptouch; }
            [[nodiscard]] SDL_PenMotionEvent& pmotion() noexcept { return raw_.pmotion; }
            [[nodiscard]] SDL_PenButtonEvent& pbutton() noexcept { return raw_.pbutton; }
            [[nodiscard]] SDL_PenAxisEvent& paxis() noexcept { return raw_.paxis; }
            [[nodiscard]] SDL_DropEvent& drop() noexcept { return raw_.drop; }
            [[nodiscard]] SDL_ClipboardEvent& clipboard() noexcept { return raw_.clipboard; }
            [[nodiscard]] SDL_UserEvent& user() noexcept { return raw_.user; }
            [[nodiscard]] SDL_DisplayEvent& display() noexcept { return raw_.display; }
            [[nodiscard]] SDL_RenderEvent& render() noexcept { return raw_.render; }

            // Const versions
            [[nodiscard]] const SDL_CommonEvent& common() const noexcept { return raw_.common; }
            [[nodiscard]] const SDL_QuitEvent& quit() const noexcept { return raw_.quit; }
            [[nodiscard]] const SDL_WindowEvent& window() const noexcept { return raw_.window; }
            [[nodiscard]] const SDL_KeyboardEvent& key() const noexcept { return raw_.key; }
            [[nodiscard]] const SDL_TextEditingEvent& edit() const noexcept { return raw_.edit; }

            [[nodiscard]] const SDL_TextEditingCandidatesEvent& edit_candidates() const noexcept {
                return raw_.edit_candidates;
            }

            [[nodiscard]] const SDL_TextInputEvent& text() const noexcept { return raw_.text; }
            [[nodiscard]] const SDL_MouseMotionEvent& motion() const noexcept { return raw_.motion; }
            [[nodiscard]] const SDL_MouseButtonEvent& button() const noexcept { return raw_.button; }
            [[nodiscard]] const SDL_MouseWheelEvent& wheel() const noexcept { return raw_.wheel; }
            [[nodiscard]] const SDL_JoyAxisEvent& jaxis() const noexcept { return raw_.jaxis; }
            [[nodiscard]] const SDL_JoyBallEvent& jball() const noexcept { return raw_.jball; }
            [[nodiscard]] const SDL_JoyHatEvent& jhat() const noexcept { return raw_.jhat; }
            [[nodiscard]] const SDL_JoyButtonEvent& jbutton() const noexcept { return raw_.jbutton; }
            [[nodiscard]] const SDL_JoyBatteryEvent& jbattery() const noexcept { return raw_.jbattery; }
            [[nodiscard]] const SDL_GamepadAxisEvent& gaxis() const noexcept { return raw_.gaxis; }
            [[nodiscard]] const SDL_GamepadButtonEvent& gbutton() const noexcept { return raw_.gbutton; }
            [[nodiscard]] const SDL_GamepadTouchpadEvent& gtouchpad() const noexcept { return raw_.gtouchpad; }
            [[nodiscard]] const SDL_GamepadSensorEvent& gsensor() const noexcept { return raw_.gsensor; }
            [[nodiscard]] const SDL_AudioDeviceEvent& adevice() const noexcept { return raw_.adevice; }
            [[nodiscard]] const SDL_CameraDeviceEvent& cdevice() const noexcept { return raw_.cdevice; }
            [[nodiscard]] const SDL_SensorEvent& sensor() const noexcept { return raw_.sensor; }
            [[nodiscard]] const SDL_TouchFingerEvent& tfinger() const noexcept { return raw_.tfinger; }
            [[nodiscard]] const SDL_PenProximityEvent& pproximity() const noexcept { return raw_.pproximity; }
            [[nodiscard]] const SDL_PenTouchEvent& ptouch() const noexcept { return raw_.ptouch; }
            [[nodiscard]] const SDL_PenMotionEvent& pmotion() const noexcept { return raw_.pmotion; }
            [[nodiscard]] const SDL_PenButtonEvent& pbutton() const noexcept { return raw_.pbutton; }
            [[nodiscard]] const SDL_PenAxisEvent& paxis() const noexcept { return raw_.paxis; }
            [[nodiscard]] const SDL_DropEvent& drop() const noexcept { return raw_.drop; }
            [[nodiscard]] const SDL_ClipboardEvent& clipboard() const noexcept { return raw_.clipboard; }
            [[nodiscard]] const SDL_UserEvent& user() const noexcept { return raw_.user; }
            [[nodiscard]] const SDL_DisplayEvent& display() const noexcept { return raw_.display; }
            [[nodiscard]] const SDL_RenderEvent& render() const noexcept { return raw_.render; }

            // Functional style
            template<typename T, typename F>
            bool handle(F&& f);

            template<typename T, typename F>
            bool handle(F&& f) const;

            // std::visit support
            [[nodiscard]] const event_variant& variant() const {
                ensure_variant();
                return *variant_;
            }

            template<typename Visitor>
            auto visit(Visitor&& vis) const {
                ensure_variant();
                return std::visit(std::forward <Visitor>(vis), *variant_);
            }

            // Raw access
            [[nodiscard]] SDL_Event& raw() noexcept { return raw_; }
            [[nodiscard]] const SDL_Event& raw() const noexcept { return raw_; }
    };

    // Template method implementations
    template<typename T>
    bool event::is() const noexcept {
        if constexpr (std::is_same_v <T, quit_event>) {
            return type() == event_type::quit || type() == event_type::terminating ||
                   type() == event_type::low_memory || type() == event_type::will_enter_background ||
                   type() == event_type::did_enter_background || type() == event_type::will_enter_foreground ||
                   type() == event_type::did_enter_foreground || type() == event_type::locale_changed ||
                   type() == event_type::system_theme_changed;
        } else if constexpr (std::is_same_v <T, window_event>) {
            return is_event_type_in_range(type(), event_type::window_shown, event_type::window_hdr_state_changed);
        } else if constexpr (std::is_same_v <T, keyboard_event>) {
            return type() == event_type::key_down || type() == event_type::key_up;
        } else if constexpr (std::is_same_v <T, keyboard_device_event>) {
            return type() == event_type::keyboard_added || type() == event_type::keyboard_removed;
        } else if constexpr (std::is_same_v <T, text_editing_event>) {
            return type() == event_type::text_editing;
        } else if constexpr (std::is_same_v <T, text_editing_candidates_event>) {
            return type() == event_type::text_editing_candidates;
        } else if constexpr (std::is_same_v <T, text_input_event>) {
            return type() == event_type::text_input;
        } else if constexpr (std::is_same_v <T, mouse_device_event>) {
            return type() == event_type::mouse_added || type() == event_type::mouse_removed;
        } else if constexpr (std::is_same_v <T, mouse_motion_event>) {
            return type() == event_type::mouse_motion;
        } else if constexpr (std::is_same_v <T, mouse_button_event>) {
            return type() == event_type::mouse_button_down || type() == event_type::mouse_button_up;
        } else if constexpr (std::is_same_v <T, mouse_wheel_event>) {
            return type() == event_type::mouse_wheel;
        } else if constexpr (std::is_same_v <T, joystick_device_event>) {
            return type() == event_type::joystick_added || type() == event_type::joystick_removed ||
                   type() == event_type::joystick_update_complete;
        } else if constexpr (std::is_same_v <T, joystick_axis_event>) {
            return type() == event_type::joystick_axis_motion;
        } else if constexpr (std::is_same_v <T, joystick_ball_event>) {
            return type() == event_type::joystick_ball_motion;
        } else if constexpr (std::is_same_v <T, joystick_hat_event>) {
            return type() == event_type::joystick_hat_motion;
        } else if constexpr (std::is_same_v <T, joystick_button_event>) {
            return type() == event_type::joystick_button_down || type() == event_type::joystick_button_up;
        } else if constexpr (std::is_same_v <T, joystick_battery_event>) {
            return type() == event_type::joystick_battery_updated;
        } else if constexpr (std::is_same_v <T, gamepad_device_event>) {
            return type() == event_type::gamepad_added || type() == event_type::gamepad_removed ||
                   type() == event_type::gamepad_remapped || type() == event_type::gamepad_update_complete ||
                   type() == event_type::gamepad_steam_handle_updated;
        } else if constexpr (std::is_same_v <T, gamepad_axis_event>) {
            return type() == event_type::gamepad_axis_motion;
        } else if constexpr (std::is_same_v <T, gamepad_button_event>) {
            return type() == event_type::gamepad_button_down || type() == event_type::gamepad_button_up;
        } else if constexpr (std::is_same_v <T, gamepad_touchpad_event>) {
            return type() == event_type::gamepad_touchpad_down || type() == event_type::gamepad_touchpad_motion ||
                   type() == event_type::gamepad_touchpad_up;
        } else if constexpr (std::is_same_v <T, gamepad_sensor_event>) {
            return type() == event_type::gamepad_sensor_update;
        } else if constexpr (std::is_same_v <T, audio_device_event>) {
            return type() == event_type::audio_device_added || type() == event_type::audio_device_removed ||
                   type() == event_type::audio_device_format_changed;
        } else if constexpr (std::is_same_v <T, camera_device_event>) {
            return type() == event_type::camera_device_added || type() == event_type::camera_device_removed ||
                   type() == event_type::camera_device_approved || type() == event_type::camera_device_denied;
        } else if constexpr (std::is_same_v <T, sensor_event>) {
            return type() == event_type::sensor_update;
        } else if constexpr (std::is_same_v <T, touch_finger_event>) {
            return type() == event_type::finger_down || type() == event_type::finger_up ||
                   type() == event_type::finger_motion;
        } else if constexpr (std::is_same_v <T, pen_proximity_event>) {
            return type() == event_type::pen_proximity_in || type() == event_type::pen_proximity_out;
        } else if constexpr (std::is_same_v <T, pen_touch_event>) {
            return type() == event_type::pen_down || type() == event_type::pen_up;
        } else if constexpr (std::is_same_v <T, pen_motion_event>) {
            return type() == event_type::pen_motion;
        } else if constexpr (std::is_same_v <T, pen_button_event>) {
            return type() == event_type::pen_button_down || type() == event_type::pen_button_up;
        } else if constexpr (std::is_same_v <T, pen_axis_event>) {
            return type() == event_type::pen_axis;
        } else if constexpr (std::is_same_v <T, drop_event>) {
            return type() == event_type::drop_file || type() == event_type::drop_text ||
                   type() == event_type::drop_begin || type() == event_type::drop_complete ||
                   type() == event_type::drop_position;
        } else if constexpr (std::is_same_v <T, clipboard_event>) {
            return type() == event_type::clipboard_update;
        } else if constexpr (std::is_same_v <T, display_event>) {
            return is_event_type_in_range(type(), event_type::display_orientation,
                                          event_type::display_content_scale_changed);
        } else if constexpr (std::is_same_v <T, render_event>) {
            return type() == event_type::render_targets_reset || type() == event_type::render_device_reset ||
                   type() == event_type::render_device_lost;
        } else if constexpr (std::is_same_v <T, user_event>) {
            return static_cast <Uint32>(type()) >= SDL_EVENT_USER;
        } else if constexpr (std::is_same_v <T, common_event>) {
            return true; // common_event can represent any event
        } else {
            return false;
        }
    }

    template<typename T>
    T* event::as() noexcept {
        if (is <T>()) {
            ensure_variant();
            return std::get_if <T>(&*variant_);
        }
        return nullptr;
    }

    template<typename T>
    const T* event::as() const noexcept {
        return const_cast <event*>(this)->as <T>();
    }

    template<typename T, typename F>
    bool event::handle(F&& f) {
        if (auto* e = as <T>()) {
            f(*e);
            return true;
        }
        return false;
    }

    template<typename T, typename F>
    bool event::handle(F&& f) const {
        if (auto* e = as <T>()) {
            f(*e);
            return true;
        }
        return false;
    }

    /**
     * @brief Event queue interface
     */
    class event_queue {
        public:
            /**
             * @brief Poll for next event
             * @return Event if available
             */
            [[nodiscard]] SDLPP_EXPORT static std::optional <event> poll();

            /**
             * @brief Wait for next event
             * @return Event or error
             */
            [[nodiscard]] SDLPP_EXPORT static sdlpp::expected <event, std::string> wait();

            /**
             * @brief Wait for next event with timeout
             * @param timeout Maximum time to wait
             * @return Event or error
             */
            [[nodiscard]] SDLPP_EXPORT static sdlpp::expected <event, std::string> wait_timeout(std::chrono::milliseconds timeout);

            /**
             * @brief Push event to queue
             * @param e Event to push
             * @return Success or error
             */
            [[nodiscard]] SDLPP_EXPORT static sdlpp::expected <void, std::string> push(const event& e);

            /**
             * @brief Pump events (process OS events)
             */
            SDLPP_EXPORT static void pump();

            /**
             * @brief Flush events of specific type
             * @param type Event type to flush
             */
            SDLPP_EXPORT static void flush(event_type type);

            /**
             * @brief Flush events in range
             * @param min_type Minimum event type
             * @param max_type Maximum event type
             */
            SDLPP_EXPORT static void flush_range(event_type min_type, event_type max_type);

            /**
             * @brief Has events of specific type
             * @param type Event type to check
             * @return true if events present
             */
            [[nodiscard]] SDLPP_EXPORT static bool has_event(event_type type);

            /**
             * @brief Has events in range
             * @param min_type Minimum event type
             * @param max_type Maximum event type
             * @return true if events present
             */
            [[nodiscard]] SDLPP_EXPORT static bool has_events(event_type min_type, event_type max_type);
    };

    // Inline implementations for event category functions that depend on event class
    inline event_category get_event_category(const event& e) noexcept {
        return get_event_category(e.type());
    }

    inline bool is_event_in_category(const event& e, event_category category) noexcept {
        return get_event_category(e.type()) == category;
    }

    inline bool is_input_event(const event& e) noexcept {
        return is_input_category(get_event_category(e.type()));
    }

    inline bool is_device_event(const event& e) noexcept {
        return is_device_category(get_event_category(e.type()));
    }

    /**
     * @brief Event filter RAII wrapper
     */
    class event_filter {
        public:
            using filter_func = SDL_EventFilter;

            SDLPP_EXPORT explicit event_filter(filter_func func, void* userdata = nullptr);
            SDLPP_EXPORT ~event_filter();

            event_filter(const event_filter&) = delete;
            event_filter& operator=(const event_filter&) = delete;
            SDLPP_EXPORT event_filter(event_filter&& other) noexcept;
            SDLPP_EXPORT event_filter& operator=(event_filter&& other) noexcept;

        private:
            filter_func func_{nullptr};
            void* userdata_{nullptr};
            filter_func prev_func_{nullptr};
            void* prev_userdata_{nullptr};
    };

    /**
     * @brief Event watcher RAII wrapper
     */
    class event_watcher {
        public:
            using watcher_func = SDL_EventFilter;

            explicit event_watcher(watcher_func func, void* userdata = nullptr);
            ~event_watcher();

            event_watcher(const event_watcher&) = delete;
            event_watcher& operator=(const event_watcher&) = delete;
            event_watcher(event_watcher&& other) noexcept;
            event_watcher& operator=(event_watcher&& other) noexcept;

        private:
            watcher_func func_{nullptr};
            void* userdata_{nullptr};
    };

    /**
     * @brief Custom event registry
     */
    class event_registry {
        public:
            /**
             * @brief Register custom events
             * @param count Number of events to register
             * @return First event ID or error
             */
            [[nodiscard]] SDLPP_EXPORT static sdlpp::expected <Uint32, std::string> register_events(size_t count);

            /**
             * @brief Check if event type is custom
             * @param type Event type
             * @return true if custom event
             */
            [[nodiscard]] SDLPP_EXPORT static bool is_custom(event_type type) noexcept;

            /**
             * @brief Check if event type is custom
             * @param type Event type ID
             * @return true if custom event
             */
            [[nodiscard]] SDLPP_EXPORT static bool is_custom(Uint32 type) noexcept;
    };

    /**
     * @brief Get global event queue
     * @return Event queue instance
     */
    [[nodiscard]] inline event_queue& get_event_queue() {
        static event_queue queue;
        return queue;
    }
} // namespace sdlpp

// Include implementation details after all types are defined
#include <sdlpp/events/event_impl.hh>


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for event_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, event_type value);

/**
 * @brief Stream input operator for event_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, event_type& value);

}