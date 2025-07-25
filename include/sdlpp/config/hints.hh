//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file hints.hh
 * @brief Modern C++ wrapper for SDL3 hints system
 * 
 * This header provides RAII-managed wrappers around SDL3's hints system,
 * which allows configuration of various SDL behaviors through key-value pairs.
 * Hints can be set programmatically or through environment variables.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/expected.hh>
#include <string>
#include <optional>
#include <functional>
#include <unordered_map>
#include <memory>

namespace sdlpp {
    /**
     * @brief Hint priority levels
     *
     * These determine which hint setting takes precedence when
     * multiple sources try to set the same hint.
     */
    enum class hint_priority {
        default_priority = SDL_HINT_DEFAULT, ///< Low priority, can be overridden
        normal = SDL_HINT_NORMAL, ///< Normal priority
        override_priority = SDL_HINT_OVERRIDE ///< High priority, overrides others
    };

    /**
     * @brief Common hint names as strongly-typed constants
     *
     * This namespace provides type-safe access to SDL hint names.
     * Using these constants helps prevent typos and provides IDE autocomplete.
     */
    namespace hints {
        // Video hints
        inline constexpr std::string_view framebuffer_acceleration = SDL_HINT_FRAMEBUFFER_ACCELERATION;
        inline constexpr std::string_view render_driver = SDL_HINT_RENDER_DRIVER;
        inline constexpr std::string_view render_vsync = SDL_HINT_RENDER_VSYNC;
        inline constexpr std::string_view video_driver = SDL_HINT_VIDEO_DRIVER;
        inline constexpr std::string_view video_x11_xrandr = SDL_HINT_VIDEO_X11_XRANDR;

        // Window hints
        inline constexpr std::string_view mouse_focus_clickthrough = SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH;

        // Audio hints
        inline constexpr std::string_view audio_device_app_icon_name = SDL_HINT_AUDIO_DEVICE_APP_ICON_NAME;
        inline constexpr std::string_view audio_device_stream_name = SDL_HINT_AUDIO_DEVICE_STREAM_NAME;
        inline constexpr std::string_view audio_driver = SDL_HINT_AUDIO_DRIVER;

        // Joystick/Controller hints
        inline constexpr std::string_view joystick_allow_background_events = SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS;
        inline constexpr std::string_view joystick_hidapi = SDL_HINT_JOYSTICK_HIDAPI;
        inline constexpr std::string_view joystick_hidapi_ps4 = SDL_HINT_JOYSTICK_HIDAPI_PS4;
        inline constexpr std::string_view joystick_hidapi_ps5 = SDL_HINT_JOYSTICK_HIDAPI_PS5;
        inline constexpr std::string_view joystick_hidapi_xbox = SDL_HINT_JOYSTICK_HIDAPI_XBOX;

        // Platform-specific hints
        inline constexpr std::string_view mac_ctrl_click_emulate_right_click = SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK;

        // Thread hints
        inline constexpr std::string_view thread_priority_policy = SDL_HINT_THREAD_PRIORITY_POLICY;

        // App info hints
        inline constexpr std::string_view app_name = SDL_HINT_APP_NAME;
        inline constexpr std::string_view app_id = SDL_HINT_APP_ID;

        // Timer hints
        inline constexpr std::string_view timer_resolution = SDL_HINT_TIMER_RESOLUTION;

        // OpenGL hints
        inline constexpr std::string_view opengl_es_driver = SDL_HINT_OPENGL_ES_DRIVER;
    }

    /**
     * @brief Hint callback function type
     */
    using hint_callback = std::function <void(const std::string& name,
                                              const std::optional <std::string>& old_value,
                                              const std::optional <std::string>& new_value)>;

    /**
     * @brief RAII wrapper for hint callbacks
     *
     * Automatically removes the callback when destroyed
     */
    class hint_callback_guard {
        private:
            std::string hint_name;
            SDL_HintCallback callback;
            void* userdata;
            bool active;

        public:
            hint_callback_guard(const std::string& name, SDL_HintCallback cb, void* data)
                : hint_name(name), callback(cb), userdata(data), active(true) {
            }

            ~hint_callback_guard() {
                if (active) {
                    SDL_RemoveHintCallback(hint_name.c_str(), callback, userdata);
                }
            }

            // Move-only
            hint_callback_guard(hint_callback_guard&& other) noexcept
                : hint_name(std::move(other.hint_name))
                  , callback(other.callback)
                  , userdata(other.userdata)
                  , active(other.active) {
                other.active = false;
            }

            hint_callback_guard& operator=(hint_callback_guard&& other) noexcept {
                if (this != &other) {
                    if (active) {
                        SDL_RemoveHintCallback(hint_name.c_str(), callback, userdata);
                    }
                    hint_name = std::move(other.hint_name);
                    callback = other.callback;
                    userdata = other.userdata;
                    active = other.active;
                    other.active = false;
                }
                return *this;
            }

            hint_callback_guard(const hint_callback_guard&) = delete;
            hint_callback_guard& operator=(const hint_callback_guard&) = delete;
    };

    /**
     * @brief Static hint management class
     *
     * Provides a high-level interface to SDL's hint system with
     * type safety, RAII callback management, and convenient utilities.
     */
    class hint_manager {
        private:
            // Storage for C++ callbacks to handle lifetime correctly
            struct callback_data {
                hint_callback func;

                static void sdl_callback(void* userdata, const char* name,
                                         const char* old_value, const char* new_value) {
                    auto* data = static_cast <callback_data*>(userdata);
                    std::optional <std::string> old_opt = old_value ? std::make_optional(old_value) : std::nullopt;
                    std::optional <std::string> new_opt = new_value ? std::make_optional(new_value) : std::nullopt;
                    data->func(name, old_opt, new_opt);
                }
            };

            // Static storage for callbacks (needed for lifetime management)
            inline static std::unordered_map <std::string,
                                              std::vector <std::unique_ptr <callback_data>>> callbacks;

        public:
            /**
             * @brief Set a hint value
             * @param name Hint name (use constants from hints namespace)
             * @param value Hint value
             * @param priority Priority level
             * @return true if hint was set successfully
             */
            [[nodiscard]] static bool set(std::string_view name,
                                          std::string_view value,
                                          hint_priority priority = hint_priority::normal) {
                // SDL requires null-terminated strings
                std::string name_str(name);
                std::string value_str(value);
                return SDL_SetHintWithPriority(name_str.c_str(), value_str.c_str(),
                                               static_cast <SDL_HintPriority>(priority)) == true;
            }

            /**
             * @brief Get a hint value
             * @param name Hint name
             * @return Current hint value, or nullopt if not set
             */
            [[nodiscard]] static std::optional <std::string> get(std::string_view name) {
                std::string name_str(name);
                const char* value = SDL_GetHint(name_str.c_str());
                return value ? std::make_optional(value) : std::nullopt;
            }

            /**
             * @brief Get a hint value with default
             * @param name Hint name
             * @param default_value Value to return if hint is not set
             * @return Hint value or default
             */
            [[nodiscard]] static std::string get_or(std::string_view name,
                                                    std::string_view default_value) {
                std::string name_str(name);
                const char* value = SDL_GetHint(name_str.c_str());
                return value ? value : std::string(default_value);
            }

            /**
             * @brief Get a boolean hint value
             * @param name Hint name
             * @param default_value Default value if hint is not set
             * @return Boolean interpretation of hint
             */
            [[nodiscard]] static bool get_boolean(std::string_view name,
                                                  bool default_value = false) {
                std::string name_str(name);
                return SDL_GetHintBoolean(name_str.c_str(), default_value ? 1 : 0) == 1;
            }

            /**
             * @brief Set a boolean hint
             * @param name Hint name
             * @param value Boolean value
             * @param priority Priority level
             * @return true if hint was set successfully
             */
            [[nodiscard]] static bool set_boolean(std::string_view name,
                                                  bool value,
                                                  hint_priority priority = hint_priority::normal) {
                return set(name, value ? "1" : "0", priority);
            }

            /**
             * @brief Reset a specific hint to its default value
             * @param name Hint name
             * @return true if hint was reset successfully
             */
            [[nodiscard]] static bool reset(std::string_view name) {
                std::string name_str(name);
                return SDL_ResetHint(name_str.c_str()) == true;
            }

            /**
             * @brief Reset all hints to their default values
             */
            static void reset_all() {
                SDL_ResetHints();
            }

            /**
             * @brief Add a callback for hint changes
             * @param name Hint name to watch
             * @param callback Function to call when hint changes
             * @return RAII guard that removes callback when destroyed
             */
            [[nodiscard]] static hint_callback_guard add_callback(std::string_view name,
                                                                  hint_callback callback) {
                std::string name_str(name);
                auto data = std::make_unique <callback_data>();
                data->func = std::move(callback);

                SDL_AddHintCallback(name_str.c_str(), callback_data::sdl_callback, data.get());

                auto* raw_data = data.get();
                callbacks[name_str].push_back(std::move(data));

                return hint_callback_guard(name_str, callback_data::sdl_callback, raw_data);
            }

            /**
             * @brief RAII hint setter that restores previous value
             */
            class scoped_hint {
                private:
                    std::string name;
                    std::optional <std::string> old_value;
                    bool should_restore;

                public:
                    scoped_hint(std::string_view hint_name, std::string_view value,
                                hint_priority priority = hint_priority::normal)
                        : name(hint_name), should_restore(true) {
                        old_value = get(name);
                        [[maybe_unused]] auto set_result = set(name, value, priority);
                    }

                    ~scoped_hint() {
                        if (should_restore) {
                            if (old_value) {
                                [[maybe_unused]] auto set_result = set(name, *old_value,
                                                                       hint_priority::override_priority);
                            } else {
                                [[maybe_unused]] auto reset_result = reset(name);
                            }
                        }
                    }

                    // Move-only
                    scoped_hint(scoped_hint&& other) noexcept
                        : name(std::move(other.name))
                          , old_value(std::move(other.old_value))
                          , should_restore(other.should_restore) {
                        other.should_restore = false;
                    }

                    scoped_hint& operator=(scoped_hint&& other) noexcept {
                        if (this != &other) {
                            if (should_restore && old_value) {
                                [[maybe_unused]] auto set_result = set(name, *old_value,
                                                                       hint_priority::override_priority);
                            }
                            name = std::move(other.name);
                            old_value = std::move(other.old_value);
                            should_restore = other.should_restore;
                            other.should_restore = false;
                        }
                        return *this;
                    }

                    scoped_hint(const scoped_hint&) = delete;
                    scoped_hint& operator=(const scoped_hint&) = delete;
            };

            /**
             * @brief Create a scoped hint setter
             * @param name Hint name
             * @param value Temporary value to set
             * @param priority Priority level
             * @return Scoped hint object that restores original value when destroyed
             */
            [[nodiscard]] static scoped_hint set_scoped(std::string_view name,
                                                        std::string_view value,
                                                        hint_priority priority = hint_priority::normal) {
                return scoped_hint(name, value, priority);
            }

            /**
             * @brief Utility to set multiple hints at once
             * @param hints Map of hint names to values
             * @param priority Priority level for all hints
             * @return Number of hints successfully set
             */
            static size_t set_multiple(const std::unordered_map <std::string, std::string>& hints,
                                       hint_priority priority = hint_priority::normal) {
                size_t count = 0;
                for (const auto& [name, value] : hints) {
                    if (set(name, value, priority)) {
                        ++count;
                    }
                }
                return count;
            }

            /**
             * @brief Check if a hint is set
             * @param name Hint name
             * @return true if hint has a value
             */
            [[nodiscard]] static bool is_set(const std::string& name) {
                return SDL_GetHint(name.c_str()) != nullptr;
            }
    };

    /**
     * @brief Convenience functions for common hint patterns
     */
    namespace hint_utils {
        /**
         * @brief Enable vsync through hints
         * @param enable true to enable vsync
         * @return true if hint was set
         */
        inline bool set_vsync(bool enable) {
            return hint_manager::set_boolean(hints::render_vsync, enable);
        }

        /**
         * @brief Set video driver hint
         * @param driver Driver name (e.g., "x11", "wayland", "windows")
         * @return true if hint was set
         */
        inline bool set_video_driver(const std::string& driver) {
            return hint_manager::set(hints::video_driver, driver);
        }

        /**
         * @brief Set audio driver hint
         * @param driver Driver name (e.g., "alsa", "pulse", "wasapi")
         * @return true if hint was set
         */
        inline bool set_audio_driver(const std::string& driver) {
            return hint_manager::set(hints::audio_driver, driver);
        }

        /**
         * @brief Allow joystick events when app is in background
         * @param allow true to allow background events
         * @return true if hint was set
         */
        inline bool allow_background_joystick_events(bool allow) {
            return hint_manager::set_boolean(hints::joystick_allow_background_events, allow);
        }

        /**
         * @brief Set app name hint
         * @param name Application name
         * @return true if hint was set
         */
        inline bool set_app_name(const std::string& name) {
            return hint_manager::set(hints::app_name, name);
        }
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for hint_priority
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, hint_priority value);

/**
 * @brief Stream input operator for hint_priority
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, hint_priority& value);

}