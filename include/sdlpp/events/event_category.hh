#pragma once

/**
 * @file event_category.hh
 * @brief Event categorization utilities
 */

#include <sdlpp/core/sdl.hh>
#include <cstdint>

namespace sdlpp {
    // Forward declarations
    class event;
    enum class event_type : Uint32;
    /**
     * @brief Event category enumeration
     * 
     * Logical grouping of event types for easier handling and filtering
     */
    enum class event_category {
        application,    ///< Application lifecycle events (quit, foreground/background, etc.)
        window,         ///< Window-related events (resize, focus, etc.)
        keyboard,       ///< Keyboard input events (key press, text input, etc.)
        mouse,          ///< Mouse input events (motion, buttons, wheel)
        joystick,       ///< Joystick/controller events (axis, buttons, etc.)
        gamepad,        ///< Gamepad-specific events (standardized controller)
        touch,          ///< Touch/finger events
        pen,            ///< Pen/stylus events
        clipboard,      ///< Clipboard events
        drop,           ///< Drag and drop events
        audio,          ///< Audio device events
        sensor,         ///< Sensor data events
        camera,         ///< Camera device events
        display,        ///< Display/monitor events
        render,         ///< Render subsystem events
        user,           ///< User-defined events
        unknown         ///< Unknown/unrecognized events
    };

    /**
     * @brief Get the category of an event type
     * 
     * @param type The event type to categorize
     * @return The category the event belongs to
     */
    [[nodiscard]] constexpr event_category get_event_category(event_type type) noexcept {
        const auto t = static_cast<Uint32>(type);
        
        // Application events
        if (t == SDL_EVENT_QUIT ||
            t == SDL_EVENT_TERMINATING ||
            t == SDL_EVENT_LOW_MEMORY ||
            t == SDL_EVENT_WILL_ENTER_BACKGROUND ||
            t == SDL_EVENT_DID_ENTER_BACKGROUND ||
            t == SDL_EVENT_WILL_ENTER_FOREGROUND ||
            t == SDL_EVENT_DID_ENTER_FOREGROUND ||
            t == SDL_EVENT_LOCALE_CHANGED ||
            t == SDL_EVENT_SYSTEM_THEME_CHANGED) {
            return event_category::application;
        }
        
        // Window events
        if (t >= SDL_EVENT_WINDOW_SHOWN &&
            t <= SDL_EVENT_WINDOW_HDR_STATE_CHANGED) {
            return event_category::window;
        }
        
        // Keyboard events
        if (t == SDL_EVENT_KEY_DOWN ||
            t == SDL_EVENT_KEY_UP ||
            t == SDL_EVENT_TEXT_EDITING ||
            t == SDL_EVENT_TEXT_INPUT ||
            t == SDL_EVENT_KEYMAP_CHANGED ||
            t == SDL_EVENT_KEYBOARD_ADDED ||
            t == SDL_EVENT_KEYBOARD_REMOVED ||
            t == SDL_EVENT_TEXT_EDITING_CANDIDATES) {
            return event_category::keyboard;
        }
        
        // Mouse events
        if (t == SDL_EVENT_MOUSE_MOTION ||
            t == SDL_EVENT_MOUSE_BUTTON_DOWN ||
            t == SDL_EVENT_MOUSE_BUTTON_UP ||
            t == SDL_EVENT_MOUSE_WHEEL ||
            t == SDL_EVENT_MOUSE_ADDED ||
            t == SDL_EVENT_MOUSE_REMOVED) {
            return event_category::mouse;
        }
        
        // Joystick events
        if (t >= SDL_EVENT_JOYSTICK_AXIS_MOTION &&
            t <= SDL_EVENT_JOYSTICK_UPDATE_COMPLETE) {
            return event_category::joystick;
        }
        
        // Gamepad events
        if (t >= SDL_EVENT_GAMEPAD_AXIS_MOTION &&
            t <= SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED) {
            return event_category::gamepad;
        }
        
        // Touch events
        if (t == SDL_EVENT_FINGER_DOWN ||
            t == SDL_EVENT_FINGER_UP ||
            t == SDL_EVENT_FINGER_MOTION) {
            return event_category::touch;
        }
        
        // Pen events
        if (t >= SDL_EVENT_PEN_PROXIMITY_IN &&
            t <= SDL_EVENT_PEN_AXIS) {
            return event_category::pen;
        }
        
        // Clipboard events
        if (t == SDL_EVENT_CLIPBOARD_UPDATE) {
            return event_category::clipboard;
        }
        
        // Drop events
        if (t == SDL_EVENT_DROP_FILE ||
            t == SDL_EVENT_DROP_TEXT ||
            t == SDL_EVENT_DROP_BEGIN ||
            t == SDL_EVENT_DROP_COMPLETE ||
            t == SDL_EVENT_DROP_POSITION) {
            return event_category::drop;
        }
        
        // Audio events
        if (t == SDL_EVENT_AUDIO_DEVICE_ADDED ||
            t == SDL_EVENT_AUDIO_DEVICE_REMOVED ||
            t == SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED) {
            return event_category::audio;
        }
        
        // Sensor events
        if (t == SDL_EVENT_SENSOR_UPDATE) {
            return event_category::sensor;
        }
        
        // Camera events
        if (t == SDL_EVENT_CAMERA_DEVICE_ADDED ||
            t == SDL_EVENT_CAMERA_DEVICE_REMOVED ||
            t == SDL_EVENT_CAMERA_DEVICE_APPROVED ||
            t == SDL_EVENT_CAMERA_DEVICE_DENIED) {
            return event_category::camera;
        }
        
        // Display events
        if (t >= SDL_EVENT_DISPLAY_ORIENTATION &&
            t <= SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED) {
            return event_category::display;
        }
        
        // Render events
        if (t == SDL_EVENT_RENDER_TARGETS_RESET ||
            t == SDL_EVENT_RENDER_DEVICE_RESET ||
            t == SDL_EVENT_RENDER_DEVICE_LOST) {
            return event_category::render;
        }
        
        // User events
        if (t >= SDL_EVENT_USER) {
            return event_category::user;
        }
        
        return event_category::unknown;
    }

    // Forward declaration - implementation will be in events.hh after event class is defined
    [[nodiscard]] inline event_category get_event_category(const event& e) noexcept;

    /**
     * @brief Get a string representation of an event category
     * 
     * @param category The category to convert
     * @return String representation of the category
     */
    [[nodiscard]] constexpr const char* event_category_to_string(event_category category) noexcept {
        switch (category) {
            case event_category::application: return "application";
            case event_category::window: return "window";
            case event_category::keyboard: return "keyboard";
            case event_category::mouse: return "mouse";
            case event_category::joystick: return "joystick";
            case event_category::gamepad: return "gamepad";
            case event_category::touch: return "touch";
            case event_category::pen: return "pen";
            case event_category::clipboard: return "clipboard";
            case event_category::drop: return "drop";
            case event_category::audio: return "audio";
            case event_category::sensor: return "sensor";
            case event_category::camera: return "camera";
            case event_category::display: return "display";
            case event_category::render: return "render";
            case event_category::user: return "user";
            case event_category::unknown: return "unknown";
        }
        return "unknown";
    }

    /**
     * @brief Check if an event belongs to a specific category
     * 
     * @param type The event type to check
     * @param category The category to check against
     * @return true if the event belongs to the category
     */
    [[nodiscard]] constexpr bool is_event_in_category(event_type type, event_category category) noexcept {
        return get_event_category(type) == category;
    }

    // Forward declaration - implementation will be in events.hh after event class is defined
    [[nodiscard]] inline bool is_event_in_category(const event& e, event_category category) noexcept;

    /**
     * @brief Check if an event category is an input category
     * 
     * @param category The category to check
     * @return true if the category is input-related
     */
    [[nodiscard]] constexpr bool is_input_category(event_category category) noexcept {
        return category == event_category::keyboard ||
               category == event_category::mouse ||
               category == event_category::joystick ||
               category == event_category::gamepad ||
               category == event_category::touch ||
               category == event_category::pen;
    }

    /**
     * @brief Check if an event type is an input event
     * 
     * @param type The event type to check
     * @return true if the event is input-related
     */
    [[nodiscard]] constexpr bool is_input_event(event_type type) noexcept {
        return is_input_category(get_event_category(type));
    }

    // Forward declaration - implementation will be in events.hh after event class is defined
    [[nodiscard]] inline bool is_input_event(const event& e) noexcept;

    /**
     * @brief Check if an event category is a device category
     * 
     * @param category The category to check
     * @return true if the category is device-related
     */
    [[nodiscard]] constexpr bool is_device_category(event_category category) noexcept {
        return category == event_category::audio ||
               category == event_category::camera ||
               category == event_category::display;
    }

    /**
     * @brief Check if an event type is a device event
     * 
     * @param type The event type to check
     * @return true if the event is device-related
     */
    [[nodiscard]] constexpr bool is_device_event(event_type type) noexcept {
        return is_device_category(get_event_category(type));
    }

    // Forward declaration - implementation will be in events.hh after event class is defined
    [[nodiscard]] inline bool is_device_event(const event& e) noexcept;

} // namespace sdlpp