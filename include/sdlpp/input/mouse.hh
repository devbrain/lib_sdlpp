#pragma once

/**
 * @file mouse.hh
 * @brief Mouse input functionality
 * 
 * This header provides C++ wrappers around SDL3's mouse API, offering
 * mouse state querying, cursor management, and mouse control utilities.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/events/mouse_codes.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/utility/geometry_concepts.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/input/input_constants.hh>

#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace sdlpp {
    /**
     * @brief Mouse instance ID type
     */
    using mouse_instance_id = SDL_MouseID;
    /**
     * @brief Special mouse IDs
     */
    namespace mouse_special_id {
        constexpr mouse_instance_id touch = input_constants::touch_as_mouse;
        constexpr mouse_instance_id pen = input_constants::pen_as_mouse;
    }
    /**
     * @brief System cursor types
     */
    enum class system_cursor : int {
        default_cursor = SDL_SYSTEM_CURSOR_DEFAULT,
        text = SDL_SYSTEM_CURSOR_TEXT,
        wait = SDL_SYSTEM_CURSOR_WAIT,
        crosshair = SDL_SYSTEM_CURSOR_CROSSHAIR,
        progress = SDL_SYSTEM_CURSOR_PROGRESS,
        nwse_resize = SDL_SYSTEM_CURSOR_NWSE_RESIZE,
        nesw_resize = SDL_SYSTEM_CURSOR_NESW_RESIZE,
        ew_resize = SDL_SYSTEM_CURSOR_EW_RESIZE,
        ns_resize = SDL_SYSTEM_CURSOR_NS_RESIZE,
        move = SDL_SYSTEM_CURSOR_MOVE,
        not_allowed = SDL_SYSTEM_CURSOR_NOT_ALLOWED,
        pointer = SDL_SYSTEM_CURSOR_POINTER,
        nw_resize = SDL_SYSTEM_CURSOR_NW_RESIZE,
        n_resize = SDL_SYSTEM_CURSOR_N_RESIZE,
        ne_resize = SDL_SYSTEM_CURSOR_NE_RESIZE,
        e_resize = SDL_SYSTEM_CURSOR_E_RESIZE,
        se_resize = SDL_SYSTEM_CURSOR_SE_RESIZE,
        s_resize = SDL_SYSTEM_CURSOR_S_RESIZE,
        sw_resize = SDL_SYSTEM_CURSOR_SW_RESIZE,
        w_resize = SDL_SYSTEM_CURSOR_W_RESIZE
    };

    /**
     * @brief Smart pointer type for SDL_Cursor with automatic cleanup
     */
    using cursor_ptr = pointer <SDL_Cursor, SDL_DestroyCursor>;

    /**
     * @brief Check if the system has a mouse
     *
     * @return true if at least one mouse is available
     */
    [[nodiscard]] inline bool has_mouse() noexcept {
        return SDL_HasMouse();
    }

    /**
     * @brief Get list of available mice
     *
     * @return Vector of mouse instance IDs
     */
    [[nodiscard]] inline std::vector <mouse_instance_id> get_mice() {
        int count = 0;
        const SDL_MouseID* mice = SDL_GetMice(&count);
        if (!mice || count <= 0) {
            return {};
        }
        return std::vector <mouse_instance_id>(mice, mice + count);
    }

    /**
     * @brief Get the name of a specific mouse
     *
     * @param instance_id The mouse instance ID
     * @return Mouse name, or empty string if not found
     */
    [[nodiscard]] inline std::string get_mouse_name(mouse_instance_id instance_id) {
        const char* name = SDL_GetMouseNameForID(instance_id);
        return name ? name : "";
    }

    /**
     * @brief Get the window that currently has mouse focus
     *
     * @return Window with mouse focus, or nullptr if none
     */
    [[nodiscard]] inline SDL_Window* get_mouse_focus() noexcept {
        return SDL_GetMouseFocus();
    }

    /**
     * @brief Mouse state information
     */
    struct mouse_state {
        int x = 0;
        int y = 0;
        mouse_button_mask buttons = mouse_button_mask::none;

        [[nodiscard]] bool is_button_pressed(mouse_button button) const noexcept {
            return has_button(buttons, button_to_mask(button));
        }
    };

    /**
     * @brief Get the current mouse state relative to the focused window
     *
     * @return Current mouse state
     */
    [[nodiscard]] inline mouse_state get_mouse_state() noexcept {
        mouse_state state;
        float fx, fy;
        Uint32 button_state = SDL_GetMouseState(&fx, &fy);
        state.x = static_cast <int>(fx);
        state.y = static_cast <int>(fy);
        state.buttons = static_cast <mouse_button_mask>(button_state);
        return state;
    }

    /**
     * @brief Get the global mouse state
     *
     * @return Current global mouse state
     */
    [[nodiscard]] inline mouse_state get_global_mouse_state() noexcept {
        mouse_state state;
        float fx, fy;
        Uint32 button_state = SDL_GetGlobalMouseState(&fx, &fy);
        state.x = static_cast <int>(fx);
        state.y = static_cast <int>(fy);
        state.buttons = static_cast <mouse_button_mask>(button_state);
        return state;
    }

    /**
     * @brief Get the relative mouse state
     *
     * @return Relative mouse movement and button state
     */
    [[nodiscard]] inline mouse_state get_relative_mouse_state() noexcept {
        mouse_state state;
        float fx, fy;
        Uint32 button_state = SDL_GetRelativeMouseState(&fx, &fy);
        state.x = static_cast <int>(fx);
        state.y = static_cast <int>(fy);
        state.buttons = static_cast <mouse_button_mask>(button_state);
        return state;
    }

    /**
     * @brief Warp the mouse to a position within a window
     *
     * @param win The window
     * @param x X coordinate
     * @param y Y coordinate
     */
    inline void warp_mouse_in_window(const window& win, int x, int y) {
        if (win.get()) {
            SDL_WarpMouseInWindow(win.get(), static_cast <float>(x), static_cast <float>(y));
        }
    }

    /**
     * @brief Warp the mouse to a position within a window
     *
     * @tparam P Point-like type
     * @param win The window
     * @param pos The position
     */
    template<point_like P>
    inline void warp_mouse_in_window(const window& win, const P& pos) {
        warp_mouse_in_window(win, static_cast<int>(get_x(pos)), static_cast<int>(get_y(pos)));
    }

    /**
     * @brief Warp the mouse to a global position
     *
     * @param x Global X coordinate
     * @param y Global Y coordinate
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> warp_mouse_global(int x, int y) {
        if (!SDL_WarpMouseGlobal(static_cast <float>(x), static_cast <float>(y))) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Warp the mouse to a global position
     *
     * @tparam P Point-like type
     * @param pos The global position
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P>
    inline expected <void, std::string> warp_mouse_global(const P& pos) {
        return warp_mouse_global(static_cast<int>(get_x(pos)), static_cast<int>(get_y(pos)));
    }

    /**
     * @brief Set relative mouse mode for a window
     *
     * While the mouse is in relative mode, the cursor is hidden and mouse
     * movement is not bounded by the screen edges.
     *
     * @param win The window
     * @param enabled true to enable relative mode
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> set_window_relative_mouse_mode(const window& win, bool enabled) {
        if (!win.get()) {
            return make_unexpectedf("Invalid window");
        }

        if (!SDL_SetWindowRelativeMouseMode(win.get(), enabled)) {
            return make_unexpectedf(get_error());
        }

        return {};
    }

    /**
     * @brief Get the relative mouse mode state for a window
     *
     * @param win The window
     * @return true if relative mouse mode is enabled
     */
    [[nodiscard]] inline bool get_window_relative_mouse_mode(const window& win) noexcept {
        return win.get() && SDL_GetWindowRelativeMouseMode(win.get());
    }

    /**
     * @brief RAII wrapper for relative mouse mode
     *
     * This class manages relative mouse mode, automatically restoring
     * the previous state when destroyed.
     */
    class relative_mouse_mode {
        public:
            /**
             * @brief Enable relative mouse mode for a window
             *
             * @param win The window
             */
            explicit relative_mouse_mode(const window& win)
                : window_(&win), was_enabled_(win.get() ? SDL_GetWindowRelativeMouseMode(win.get()) : false) {
                if (win.get() && !was_enabled_) {
                    SDL_SetWindowRelativeMouseMode(win.get(), true);
                }
            }

            // Non-copyable
            relative_mouse_mode(const relative_mouse_mode&) = delete;
            relative_mouse_mode& operator=(const relative_mouse_mode&) = delete;

            // Movable
            relative_mouse_mode(relative_mouse_mode&& other) noexcept
                : window_(other.window_), was_enabled_(other.was_enabled_) {
                other.window_ = nullptr;
            }

            relative_mouse_mode& operator=(relative_mouse_mode&& other) noexcept {
                if (this != &other) {
                    restore();
                    window_ = other.window_;
                    was_enabled_ = other.was_enabled_;
                    other.window_ = nullptr;
                }
                return *this;
            }

            ~relative_mouse_mode() {
                restore();
            }

            /**
             * @brief Check if this session is active
             *
             * @return true if managing relative mouse mode
             */
            [[nodiscard]] bool is_active() const noexcept {
                return window_ != nullptr;
            }

            /**
             * @brief Restore the original mouse mode
             */
            void restore() noexcept {
                if (window_ && window_->get() && !was_enabled_) {
                    SDL_SetWindowRelativeMouseMode(window_->get(), false);
                }
                window_ = nullptr;
            }

        private:
            const window* window_;
            bool was_enabled_;
    };

    /**
     * @brief Capture the mouse
     *
     * When mouse is captured, mouse events will continue to be delivered to
     * the current window even if the mouse leaves the window.
     *
     * @param enable true to capture, false to release
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> capture_mouse(bool enable) {
        if (!SDL_CaptureMouse(enable)) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief RAII wrapper for mouse capture
     */
    class mouse_capture {
        public:
            mouse_capture() {
                SDL_CaptureMouse(true);
            }

            ~mouse_capture() {
                SDL_CaptureMouse(false);
            }

            // Non-copyable, non-movable
            mouse_capture(const mouse_capture&) = delete;
            mouse_capture& operator=(const mouse_capture&) = delete;
            mouse_capture(mouse_capture&&) = delete;
            mouse_capture& operator=(mouse_capture&&) = delete;
    };

    /**
     * @brief RAII wrapper for SDL_Cursor
     *
     * This class provides a safe, RAII-managed interface to SDL's cursor functionality.
     * The cursor is automatically destroyed when the object goes out of scope.
     */
    class cursor {
        private:
            cursor_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty cursor
             */
            cursor() = default;

            /**
             * @brief Construct from existing SDL_Cursor pointer
             * @param c Existing SDL_Cursor pointer (takes ownership)
             */
            explicit cursor(SDL_Cursor* c)
                : ptr(c) {
            }

            /**
             * @brief Move constructor
             */
            cursor(cursor&&) = default;

            /**
             * @brief Move assignment operator
             */
            cursor& operator=(cursor&&) = default;

            // Delete copy operations - cursors are move-only
            cursor(const cursor&) = delete;
            cursor& operator=(const cursor&) = delete;

            /**
             * @brief Check if the cursor is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Cursor pointer
             */
            [[nodiscard]] SDL_Cursor* get() const noexcept { return ptr.get(); }

            /**
             * @brief Create a monochrome cursor from data
             *
             * @param data Cursor bitmap data
             * @param mask Cursor mask data
             * @param w Width in pixels (must be multiple of 8)
             * @param h Height in pixels
             * @param hot_x X position of hot spot
             * @param hot_y Y position of hot spot
             * @return Expected containing cursor, or error message
             */
            static expected <cursor, std::string> create(
                const Uint8* data, const Uint8* mask,
                int w, int h, int hot_x, int hot_y) {
                auto* c = SDL_CreateCursor(data, mask, w, h, hot_x, hot_y);
                if (!c) {
                    return make_unexpectedf(get_error());
                }
                return cursor(c);
            }

            /**
             * @brief Create a color cursor from a surface
             *
             * @param surf The surface containing cursor image
             * @param hot_x X position of hot spot
             * @param hot_y Y position of hot spot
             * @return Expected containing cursor, or error message
             */
            static expected <cursor, std::string> create_color(
                const surface& surf, int hot_x, int hot_y) {
                if (!surf.get()) {
                    return make_unexpectedf("Invalid surface");
                }

                auto* c = SDL_CreateColorCursor(surf.get(), hot_x, hot_y);
                if (!c) {
                    return make_unexpectedf(get_error());
                }
                return cursor(c);
            }

            /**
             * @brief Create a system cursor
             *
             * @param id System cursor type
             * @return Expected containing cursor, or error message
             */
            static expected <cursor, std::string> create_system(system_cursor id) {
                auto* c = SDL_CreateSystemCursor(static_cast <SDL_SystemCursor>(id));
                if (!c) {
                    return make_unexpectedf(get_error());
                }
                return cursor(c);
            }

            /**
             * @brief Set this cursor as the active cursor
             *
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid cursor");
                }

                if (!SDL_SetCursor(ptr.get())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }
    };

    /**
     * @brief Get the current cursor
     *
     * @return Current cursor (non-owning)
     */
    [[nodiscard]] inline SDL_Cursor* get_cursor() noexcept {
        return SDL_GetCursor();
    }

    /**
     * @brief Get the default cursor
     *
     * @return Default cursor (non-owning)
     */
    [[nodiscard]] inline SDL_Cursor* get_default_cursor() noexcept {
        return SDL_GetDefaultCursor();
    }

    /**
     * @brief Show the cursor
     *
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> show_cursor() {
        if (!SDL_ShowCursor()) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Hide the cursor
     *
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> hide_cursor() {
        if (!SDL_HideCursor()) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Check if cursor is visible
     *
     * @return true if cursor is visible
     */
    [[nodiscard]] inline bool is_cursor_visible() noexcept {
        return SDL_CursorVisible();
    }

    /**
     * @brief RAII wrapper for cursor visibility
     */
    class cursor_visibility {
        public:
            /**
             * @brief Set cursor visibility
             *
             * @param visible true to show, false to hide
             */
            explicit cursor_visibility(bool visible)
                : was_visible_(SDL_CursorVisible()) {
                if (visible) {
                    SDL_ShowCursor();
                } else {
                    SDL_HideCursor();
                }
            }

            ~cursor_visibility() {
                if (was_visible_) {
                    SDL_ShowCursor();
                } else {
                    SDL_HideCursor();
                }
            }

            // Non-copyable, non-movable
            cursor_visibility(const cursor_visibility&) = delete;
            cursor_visibility& operator=(const cursor_visibility&) = delete;
            cursor_visibility(cursor_visibility&&) = delete;
            cursor_visibility& operator=(cursor_visibility&&) = delete;

        private:
            bool was_visible_;
    };

    /**
     * @brief Helper class for checking mouse state
     *
     * This class provides convenient methods for checking mouse buttons
     * and position in a snapshot of the mouse state.
     */
    class mouse_state_helper {
        public:
            /**
             * @brief Construct mouse state helper
             *
             * @param global true to capture global state, false for window-relative
             */
            explicit mouse_state_helper(bool global = false) noexcept
                : state_(global ? get_global_mouse_state() : get_mouse_state()) {
            }

            /**
             * @brief Get X position
             */
            [[nodiscard]] int x() const noexcept { return state_.x; }

            /**
             * @brief Get Y position
             */
            [[nodiscard]] int y() const noexcept { return state_.y; }

            /**
             * @brief Get position as a point
             * @tparam P Point type to return (defaults to built-in point if available)
             */
            template<point_like P = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                point_i
#else
                void
#endif
            >
            [[nodiscard]] P position() const noexcept
                requires (!std::is_void_v<P>) {
                return P{state_.x, state_.y};
            }

            /**
             * @brief Check if a button is pressed
             *
             * @param button The button to check
             * @return true if pressed
             */
            [[nodiscard]] bool is_button_pressed(mouse_button button) const noexcept {
                return state_.is_button_pressed(button);
            }

            /**
             * @brief Check if left button is pressed
             */
            [[nodiscard]] bool is_left_pressed() const noexcept {
                return is_button_pressed(mouse_button::left);
            }

            /**
             * @brief Check if middle button is pressed
             */
            [[nodiscard]] bool is_middle_pressed() const noexcept {
                return is_button_pressed(mouse_button::middle);
            }

            /**
             * @brief Check if right button is pressed
             */
            [[nodiscard]] bool is_right_pressed() const noexcept {
                return is_button_pressed(mouse_button::right);
            }

            /**
             * @brief Check if X1 button is pressed
             */
            [[nodiscard]] bool is_x1_pressed() const noexcept {
                return is_button_pressed(mouse_button::x1);
            }

            /**
             * @brief Check if X2 button is pressed
             */
            [[nodiscard]] bool is_x2_pressed() const noexcept {
                return is_button_pressed(mouse_button::x2);
            }

            /**
             * @brief Check if any button is pressed
             */
            [[nodiscard]] bool any_button_pressed() const noexcept {
                return state_.buttons != mouse_button_mask::none;
            }

            /**
             * @brief Get all pressed buttons as a mask
             */
            [[nodiscard]] mouse_button_mask get_buttons() const noexcept {
                return state_.buttons;
            }

        private:
            mouse_state state_;
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for system_cursor
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, system_cursor value);

/**
 * @brief Stream input operator for system_cursor
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, system_cursor& value);

}