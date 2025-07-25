#pragma once

/**
 * @file keyboard.hh
 * @brief Keyboard input functionality
 * 
 * This header provides C++ wrappers around SDL3's keyboard API, offering
 * keyboard state querying, text input management, and key mapping utilities.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/events/keyboard_codes.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/utility/geometry_concepts.hh>

#include <string>
#include <vector>
#include <span>
#include <optional>
#include <memory>

namespace sdlpp {
    /**
     * @brief Keyboard instance ID type
     */
    using keyboard_instance_id = SDL_KeyboardID;

    /**
     * @brief Get the current state of the keyboard
     *
     * This returns a read-only view of the keyboard state array indexed by scancode.
     * The pointer is valid for the lifetime of the application and should not be freed.
     *
     * @return Span of keyboard state (1 = pressed, 0 = released)
     */
    [[nodiscard]] inline std::span <const bool> get_keyboard_state() noexcept {
        int numkeys = 0;
        const bool* state = SDL_GetKeyboardState(&numkeys);
        return std::span <const bool>(state, static_cast <size_t>(numkeys));
    }

    /**
     * @brief Check if a specific key is currently pressed
     *
     * @param scan The scancode to check
     * @return true if the key is pressed, false otherwise
     */
    [[nodiscard]] inline bool is_key_pressed(scancode scan) noexcept {
        auto state = get_keyboard_state();
        auto index = static_cast <size_t>(scan);
        return index < state.size() && state[index];
    }

    /**
     * @brief Get the current keyboard modifier state
     *
     * @return Current modifier flags
     */
    [[nodiscard]] inline keymod get_mod_state() noexcept {
        return static_cast <keymod>(SDL_GetModState());
    }

    /**
     * @brief Set the keyboard modifier state
     *
     * This is typically only used for testing or key simulation.
     *
     * @param modstate The modifier state to set
     */
    inline void set_mod_state(keymod modstate) noexcept {
        SDL_SetModState(static_cast <SDL_Keymod>(modstate));
    }

    /**
     * @brief Convert a scancode to a keycode
     *
     * @param code The scancode to convert
     * @param modstate Optional modifier state (defaults to none)
     * @param key_event Whether this is for a key event (defaults to false)
     * @return The corresponding keycode
     */
    [[nodiscard]] inline keycode get_key_from_scancode(
        scancode code,
        keymod modstate = keymod::none,
        bool key_event = false) noexcept {
        return SDL_GetKeyFromScancode(
            static_cast <SDL_Scancode>(code),
            static_cast <SDL_Keymod>(modstate),
            key_event
        );
    }

    /**
     * @brief Convert a keycode to a scancode
     *
     * @param key The keycode to convert
     * @return The corresponding scancode, or scancode::unknown if not found
     */
    [[nodiscard]] inline scancode get_scancode_from_key(keycode key) noexcept {
        return static_cast <scancode>(
            SDL_GetScancodeFromKey(key, nullptr)
        );
    }

    /**
     * @brief Convert a keycode to a scancode with modifier hint
     *
     * @param key The keycode to convert
     * @param modstate Output parameter for suggested modifiers
     * @return The corresponding scancode, or scancode::unknown if not found
     */
    [[nodiscard]] inline scancode get_scancode_from_key(keycode key, keymod& modstate) noexcept {
        SDL_Keymod mod = SDL_KMOD_NONE;
        auto scan = static_cast <scancode>(
            SDL_GetScancodeFromKey(key, &mod)
        );
        modstate = static_cast <keymod>(mod);
        return scan;
    }

    /**
     * @brief Get the human-readable name of a key
     *
     * @param key The keycode
     * @return Key name (e.g., "Space", "Return", "A")
     */
    [[nodiscard]] inline std::string get_key_name(keycode key) {
        const char* name = SDL_GetKeyName(key);
        return name ? name : "";
    }

    /**
     * @brief Get keycode from a key name
     *
     * @param name The key name (e.g., "Space", "Return", "A")
     * @return The keycode, or SDLK_UNKNOWN if not found
     */
    [[nodiscard]] inline keycode get_key_from_name(const std::string& name) noexcept {
        return SDL_GetKeyFromName(name.c_str());
    }

    // Note: get_scancode_name is already defined in keyboard_codes.hh

    /**
     * @brief Get scancode from a scancode name
     *
     * @param name The scancode name
     * @return The scancode, or scancode::unknown if not found
     */
    [[nodiscard]] inline scancode get_scancode_from_name(const std::string& name) noexcept {
        return static_cast <scancode>(
            SDL_GetScancodeFromName(name.c_str())
        );
    }

    /**
     * @brief Check if the system has a keyboard
     *
     * @return true if at least one keyboard is available
     */
    [[nodiscard]] inline bool has_keyboard() noexcept {
        return SDL_HasKeyboard();
    }

    /**
     * @brief Get list of available keyboards
     *
     * @return Vector of keyboard instance IDs
     */
    [[nodiscard]] inline std::vector <keyboard_instance_id> get_keyboards() {
        int count = 0;
        const SDL_KeyboardID* keyboards = SDL_GetKeyboards(&count);
        if (!keyboards || count <= 0) {
            return {};
        }
        return std::vector <keyboard_instance_id>(keyboards, keyboards + count);
    }

    /**
     * @brief Get the name of a specific keyboard
     *
     * @param instance_id The keyboard instance ID
     * @return Keyboard name, or empty string if not found
     */
    [[nodiscard]] inline std::string get_keyboard_name(keyboard_instance_id instance_id) {
        const char* name = SDL_GetKeyboardNameForID(instance_id);
        return name ? name : "";
    }

    /**
     * @brief Get the window that currently has keyboard focus
     *
     * @return Window with keyboard focus, or nullptr if none
     */
    [[nodiscard]] inline SDL_Window* get_keyboard_focus() noexcept {
        return SDL_GetKeyboardFocus();
    }

    /**
     * @brief Check if the platform supports on-screen keyboard
     *
     * @return true if on-screen keyboard is supported
     */
    [[nodiscard]] inline bool has_screen_keyboard_support() noexcept {
        return SDL_HasScreenKeyboardSupport();
    }

    /**
     * @brief Check if on-screen keyboard is currently shown
     *
     * @param window The window to check
     * @return true if on-screen keyboard is visible
     */
    [[nodiscard]] inline bool is_screen_keyboard_shown(const window& win) noexcept {
        return win.get() && SDL_ScreenKeyboardShown(win.get());
    }

    /**
     * @brief RAII wrapper for text input mode
     *
     * This class manages the text input state, automatically stopping
     * text input when destroyed.
     */
    class text_input_session {
        public:
            /**
             * @brief Start text input for a window
             *
             * @tparam R Rectangle type (must satisfy rect_like)
             * @param window The window to receive text input
             * @param area Optional text input area for IME
             */
            template<rect_like R = rect_i>
            explicit text_input_session(const window& win,
                                        std::optional<R> area = std::nullopt)
                : window_(&win), was_active_(win.get() ? SDL_TextInputActive(win.get()) : false) {
                if (win.get() && !was_active_) {
                    if (area) {
                        SDL_Rect sdl_area{
                            static_cast<int>(get_x(*area)),
                            static_cast<int>(get_y(*area)),
                            static_cast<int>(get_width(*area)),
                            static_cast<int>(get_height(*area))
                        };
                        SDL_SetTextInputArea(win.get(), &sdl_area, 0);
                    }
                    SDL_StartTextInput(win.get());
                }
            }

            /**
             * @brief Start text input with properties
             *
             * @param window The window to receive text input
             * @param props Properties for text input
             */
            text_input_session(const window& win, SDL_PropertiesID props)
                : window_(&win), was_active_(win.get() ? SDL_TextInputActive(win.get()) : false) {
                if (win.get() && !was_active_) {
                    SDL_StartTextInputWithProperties(win.get(), props);
                }
            }

            // Non-copyable
            text_input_session(const text_input_session&) = delete;
            text_input_session& operator=(const text_input_session&) = delete;

            // Movable
            text_input_session(text_input_session&& other) noexcept
                : window_(other.window_), was_active_(other.was_active_) {
                other.window_ = nullptr;
            }

            text_input_session& operator=(text_input_session&& other) noexcept {
                if (this != &other) {
                    stop();
                    window_ = other.window_;
                    was_active_ = other.was_active_;
                    other.window_ = nullptr;
                }
                return *this;
            }

            ~text_input_session() {
                stop();
            }

            /**
             * @brief Update the text input area for IME
             *
             * @param area The input area rectangle
             * @param cursor Cursor position within the text
             */
            void set_input_area(const SDL_Rect& area, int cursor = 0) {
                if (window_ && window_->get()) {
                    SDL_SetTextInputArea(window_->get(), &area, cursor);
                }
            }

            /**
             * @brief Update the text input area with concepts
             *
             * @tparam R Rectangle-like type
             * @param rect The input area
             * @param cursor Cursor position within the text
             */
            template<rect_like R>
            void set_input_area(const R& r, int cursor = 0) {
                SDL_Rect area{
                    static_cast<int>(get_x(r)),
                    static_cast<int>(get_y(r)),
                    static_cast<int>(get_width(r)),
                    static_cast<int>(get_height(r))
                };
                set_input_area(area, cursor);
            }

            /**
             * @brief Check if this session is active
             *
             * @return true if text input is active
             */
            [[nodiscard]] bool is_active() const noexcept {
                return window_ != nullptr;
            }

            /**
             * @brief Stop text input early
             */
            void stop() noexcept {
                if (window_ && window_->get() && !was_active_) {
                    SDL_StopTextInput(window_->get());
                }
                window_ = nullptr;
            }

        private:
            const window* window_;
            bool was_active_;
    };

    /**
     * @brief Check if text input is currently active for a window
     *
     * @param win The window to check
     * @return true if text input mode is active for this window
     */
    [[nodiscard]] inline bool is_text_input_active(const window& win) noexcept {
        return win.get() && SDL_TextInputActive(win.get());
    }

    /**
     * @brief Keyboard state helper for checking multiple keys
     *
     * This class provides convenient methods for checking keyboard state,
     * including key combinations and modifiers.
     */
    class keyboard_state {
        public:
            /**
             * @brief Construct keyboard state snapshot
             */
            keyboard_state() noexcept
                : state_(get_keyboard_state()), mods_(get_mod_state()) {
            }

            /**
             * @brief Check if a key is pressed
             *
             * @param scan The scancode to check
             * @return true if pressed
             */
            [[nodiscard]] bool is_pressed(scancode scan) const noexcept {
                auto index = static_cast <size_t>(scan);
                return index < state_.size() && state_[index];
            }

            /**
             * @brief Check if any of the given keys are pressed
             *
             * @param scans List of scancodes to check
             * @return true if any are pressed
             */
            [[nodiscard]] bool any_pressed(std::initializer_list <scancode> scans) const noexcept {
                for (auto scan : scans) {
                    if (is_pressed(scan)) return true;
                }
                return false;
            }

            /**
             * @brief Check if all of the given keys are pressed
             *
             * @param scans List of scancodes to check
             * @return true if all are pressed
             */
            [[nodiscard]] bool all_pressed(std::initializer_list <scancode> scans) const noexcept {
                for (auto scan : scans) {
                    if (!is_pressed(scan)) return false;
                }
                return true;
            }

            /**
             * @brief Check if specific modifiers are active
             *
             * @param check Modifiers to check
             * @return true if all specified modifiers are active
             */
            [[nodiscard]] bool has_mods(keymod check) const noexcept {
                return has_keymod(mods_, check);
            }

            /**
             * @brief Get the current modifier state
             *
             * @return Current modifiers
             */
            [[nodiscard]] keymod get_mods() const noexcept {
                return mods_;
            }

            /**
             * @brief Check for common key combinations
             */
            [[nodiscard]] bool is_ctrl_pressed() const noexcept {
                return has_mods(keymod::ctrl);
            }

            [[nodiscard]] bool is_shift_pressed() const noexcept {
                return has_mods(keymod::shift);
            }

            [[nodiscard]] bool is_alt_pressed() const noexcept {
                return has_mods(keymod::alt);
            }

            [[nodiscard]] bool is_gui_pressed() const noexcept {
                return has_mods(keymod::gui);
            }

            /**
             * @brief Check for key with modifiers
             *
             * @param scan The scancode to check
             * @param mods Required modifiers
             * @return true if key is pressed with exact modifiers
             */
            [[nodiscard]] bool is_pressed_with_mods(scancode scan, keymod mods) const noexcept {
                return is_pressed(scan) && (mods_ == mods);
            }

            /**
             * @brief Check for common shortcuts
             */
            [[nodiscard]] bool is_ctrl_c() const noexcept {
                return is_pressed(scancode::c) && has_mods(keymod::ctrl);
            }

            [[nodiscard]] bool is_ctrl_v() const noexcept {
                return is_pressed(scancode::v) && has_mods(keymod::ctrl);
            }

            [[nodiscard]] bool is_ctrl_x() const noexcept {
                return is_pressed(scancode::x) && has_mods(keymod::ctrl);
            }

            [[nodiscard]] bool is_ctrl_z() const noexcept {
                return is_pressed(scancode::z) && has_mods(keymod::ctrl);
            }

            [[nodiscard]] bool is_ctrl_a() const noexcept {
                return is_pressed(scancode::a) && has_mods(keymod::ctrl);
            }

            [[nodiscard]] bool is_ctrl_s() const noexcept {
                return is_pressed(scancode::s) && has_mods(keymod::ctrl);
            }

        private:
            std::span <const bool> state_;
            keymod mods_;
    };
} // namespace sdlpp
