//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file display.hh
 * @brief Modern C++ wrapper for SDL3 display querying functionality
 * 
 * This header provides RAII-managed wrappers around SDL3's display system,
 * allowing safe and convenient access to display information, display modes,
 * and display properties.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/video/pixels.hh>
#include <string>
#include <vector>

namespace sdlpp {
    /**
     * @brief Display orientation
     */
    enum class display_orientation : int {
        unknown = SDL_ORIENTATION_UNKNOWN,
        landscape = SDL_ORIENTATION_LANDSCAPE,
        landscape_flipped = SDL_ORIENTATION_LANDSCAPE_FLIPPED,
        portrait = SDL_ORIENTATION_PORTRAIT,
        portrait_flipped = SDL_ORIENTATION_PORTRAIT_FLIPPED
    };

    /**
     * @brief System theme
     */
    enum class system_theme : int {
        unknown = SDL_SYSTEM_THEME_UNKNOWN,
        light = SDL_SYSTEM_THEME_LIGHT,
        dark = SDL_SYSTEM_THEME_DARK
    };

    /**
     * @brief Display mode information
     */
    struct display_mode {
        SDL_DisplayID display_id = 0;
        pixel_format_enum format = pixel_format_enum::unknown;
        size_t width = 0;
        size_t height = 0;
        float pixel_density = 1.0f;
        float refresh_rate = 0.0f;
        int refresh_rate_numerator = 0;
        int refresh_rate_denominator = 0;

        /**
         * @brief Construct from SDL_DisplayMode
         */
        static display_mode from_sdl(const SDL_DisplayMode& mode) {
            return {
                mode.displayID,
                static_cast <pixel_format_enum>(mode.format),
                mode.w > 0 ? static_cast<size_t>(mode.w) : 0,
                mode.h > 0 ? static_cast<size_t>(mode.h) : 0,
                mode.pixel_density,
                mode.refresh_rate,
                mode.refresh_rate_numerator,
                mode.refresh_rate_denominator
            };
        }

        /**
         * @brief Convert to SDL_DisplayMode
         */
        [[nodiscard]] SDL_DisplayMode to_sdl() const {
            SDL_DisplayMode mode{};
            mode.displayID = display_id;
            mode.format = static_cast <SDL_PixelFormat>(format);
            mode.w = static_cast<int>(width);
            mode.h = static_cast<int>(height);
            mode.pixel_density = pixel_density;
            mode.refresh_rate = refresh_rate;
            mode.refresh_rate_numerator = refresh_rate_numerator;
            mode.refresh_rate_denominator = refresh_rate_denominator;
            mode.internal = nullptr;
            return mode;
        }

        /**
         * @brief Get resolution as size
         * @tparam S Size type to return (defaults to built-in size if available)
         * @return Resolution as size type
         */
        template<size_like S = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
            size_i
#else
            void
#endif
        >
        [[nodiscard]] constexpr S resolution() const
            requires (!std::is_void_v<S>) {
            return S{static_cast<typename S::value_type>(width), 
                     static_cast<typename S::value_type>(height)};
        }

        /**
         * @brief Get precise refresh rate as a float
         */
        [[nodiscard]] constexpr float get_precise_refresh_rate() const {
            if (refresh_rate_denominator > 0) {
                return static_cast <float>(refresh_rate_numerator) / static_cast <float>(refresh_rate_denominator);
            }
            return refresh_rate;
        }

        /**
         * @brief Check if this is a high-DPI mode
         */
        [[nodiscard]] constexpr bool is_high_dpi() const {
            return pixel_density > 1.0f;
        }
    };

    /**
     * @brief Display information wrapper
     *
     * This class provides access to display properties and capabilities.
     * Display IDs are managed by SDL and remain valid for the lifetime
     * of the SDL video subsystem.
     */
    class display {
        private:
            SDL_DisplayID id = 0;

        public:
            /**
             * @brief Default constructor - creates invalid display
             */
            display() = default;

            /**
             * @brief Construct from display ID
             */
            explicit display(SDL_DisplayID display_id)
                : id(display_id) {
            }

            /**
             * @brief Check if display is valid
             */
            [[nodiscard]] bool is_valid() const { return id != 0; }
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get display ID
             */
            [[nodiscard]] SDL_DisplayID get_id() const { return id; }

            /**
             * @brief Get display name
             * @return Display name or error message
             */
            [[nodiscard]] expected <std::string, std::string> get_name() const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                const char* name = SDL_GetDisplayName(id);
                if (!name) {
                    return make_unexpectedf(get_error());
                }

                return std::string(name);
            }

            /**
             * @brief Get current display mode
             * @return Current display mode or error message
             */
            [[nodiscard]] expected <display_mode, std::string> get_current_mode() const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(id);
                if (!mode) {
                    return make_unexpectedf(get_error());
                }

                return display_mode::from_sdl(*mode);
            }

            /**
             * @brief Get desktop display mode
             * @return Desktop display mode or error message
             */
            [[nodiscard]] expected <display_mode, std::string> get_desktop_mode() const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(id);
                if (!mode) {
                    return make_unexpectedf(get_error());
                }

                return display_mode::from_sdl(*mode);
            }

            /**
             * @brief Get all fullscreen display modes
             * @return Vector of display modes or error message
             */
            [[nodiscard]] expected <std::vector <display_mode>, std::string> get_fullscreen_modes() const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                int count = 0;
                SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(id, &count);
                if (!modes && count != 0) {
                    return make_unexpectedf(get_error());
                }

                std::vector <display_mode> modes_vec;
                modes_vec.reserve(static_cast <std::size_t>(count));

                for (int i = 0; i < count; ++i) {
                    if (modes[i]) {
                        modes_vec.push_back(display_mode::from_sdl(*modes[i]));
                    }
                }

                SDL_free(modes);
                return modes_vec;
            }

            /**
             * @brief Get closest matching fullscreen display mode
             * @param width Desired width
             * @param height Desired height
             * @param refresh_rate Desired refresh rate (0.0 for any)
             * @param include_high_density_modes Include high-DPI modes
             * @return Closest matching mode or error message
             */
            [[nodiscard]] expected <display_mode, std::string> get_closest_fullscreen_mode(
                int width, int height,
                float refresh_rate = 0.0f,
                bool include_high_density_modes = false) const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                SDL_DisplayMode closest;
                bool found = SDL_GetClosestFullscreenDisplayMode(
                    id, width, height, refresh_rate, include_high_density_modes, &closest);

                if (!found) {
                    return make_unexpectedf(get_error());
                }

                return display_mode::from_sdl(closest);
            }

            /**
             * @brief Get display bounds
             * @tparam R Rectangle type to return (defaults to built-in rect if available)
             * @return Display bounds rectangle or error message
             */
            template<rect_like R = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                rect_i
#else
                void
#endif
            >
            [[nodiscard]] expected <R, std::string> get_bounds() const
                requires (!std::is_void_v<R>) {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                SDL_Rect bounds;
                if (!SDL_GetDisplayBounds(id, &bounds)) {
                    return make_unexpectedf(get_error());
                }

                return R{bounds.x, bounds.y, bounds.w, bounds.h};
            }

            /**
             * @brief Get usable display bounds (excluding taskbars, docks, etc.)
             * @tparam R Rectangle type to return (defaults to built-in rect if available)
             * @return Usable display bounds rectangle or error message
             */
            template<rect_like R = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                rect_i
#else
                void
#endif
            >
            [[nodiscard]] expected <R, std::string> get_usable_bounds() const
                requires (!std::is_void_v<R>) {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                SDL_Rect bounds;
                if (!SDL_GetDisplayUsableBounds(id, &bounds)) {
                    return make_unexpectedf(get_error());
                }

                return R{bounds.x, bounds.y, bounds.w, bounds.h};
            }

            /**
             * @brief Get display content scale
             * @return Content scale factor or error message
             */
            [[nodiscard]] expected <float, std::string> get_content_scale() const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                float scale = SDL_GetDisplayContentScale(id);
                if (scale <= 0.0f) {
                    return make_unexpectedf(get_error());
                }

                return scale;
            }

            /**
             * @brief Get current display orientation
             * @return Current orientation or error message
             */
            [[nodiscard]] expected <display_orientation, std::string> get_current_orientation() const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                SDL_DisplayOrientation orient = SDL_GetCurrentDisplayOrientation(id);
                if (orient == SDL_ORIENTATION_UNKNOWN) {
                    // Check if it's an actual error or just unknown
                    auto error = get_error();
                    if (!error.empty()) {
                        return make_unexpectedf(error);
                    }
                }

                return static_cast <display_orientation>(orient);
            }

            /**
             * @brief Get natural display orientation
             * @return Natural orientation or error message
             */
            [[nodiscard]] expected <display_orientation, std::string> get_natural_orientation() const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                SDL_DisplayOrientation orient = SDL_GetNaturalDisplayOrientation(id);
                if (orient == SDL_ORIENTATION_UNKNOWN) {
                    // Check if it's an actual error or just unknown
                    auto error = get_error();
                    if (!error.empty()) {
                        return make_unexpectedf(error);
                    }
                }

                return static_cast <display_orientation>(orient);
            }

            /**
             * @brief Get display properties
             * @return Properties ID or error message
             */
            [[nodiscard]] expected <SDL_PropertiesID, std::string> get_properties() const {
                if (!id) {
                    return make_unexpectedf("Invalid display");
                }

                SDL_PropertiesID props = SDL_GetDisplayProperties(id);
                if (!props) {
                    return make_unexpectedf(get_error());
                }

                return props;
            }

            /**
             * @brief Equality comparison
             */
            [[nodiscard]] bool operator==(const display& other) const {
                return id == other.id;
            }

            [[nodiscard]] bool operator!=(const display& other) const {
                return !(*this == other);
            }
    };

    /**
     * @brief Display manager for querying system displays
     */
    class display_manager {
        public:
            /**
             * @brief Get all connected displays
             * @return Vector of displays or error message
             */
            static expected <std::vector <display>, std::string> get_displays() {
                int count = 0;
                SDL_DisplayID* ids = SDL_GetDisplays(&count);

                if (!ids && count != 0) {
                    return make_unexpectedf(get_error());
                }

                std::vector <display> displays_vec;
                displays_vec.reserve(static_cast <std::size_t>(count));

                for (int i = 0; i < count; ++i) {
                    displays_vec.emplace_back(ids[i]);
                }

                SDL_free(ids);
                return displays_vec;
            }

            /**
             * @brief Get primary display
             * @return Primary display or error message
             */
            static expected <display, std::string> get_primary_display() {
                SDL_DisplayID id = SDL_GetPrimaryDisplay();
                if (!id) {
                    return make_unexpectedf(get_error());
                }

                return display(id);
            }

            /**
             * @brief Get display containing a point
             * @tparam P Point type (must satisfy point_like)
             * @param p Point to check
             * @return Display containing the point or error message
             */
            template<point_like P>
            static expected <display, std::string> get_display_for_point(const P& p) {
                SDL_Point sdl_point{static_cast<int>(get_x(p)), static_cast<int>(get_y(p))};
                SDL_DisplayID id = SDL_GetDisplayForPoint(&sdl_point);
                if (!id) {
                    return make_unexpectedf("No display found for point");
                }

                return display(id);
            }

            /**
             * @brief Get display best suited for a rectangle
             * @tparam R Rectangle type (must satisfy rect_like)
             * @param r Rectangle to check
             * @return Best display for the rectangle or error message
             */
            template<rect_like R>
            static expected <display, std::string> get_display_for_rect(const R& r) {
                SDL_Rect sdl_rect{static_cast<int>(get_x(r)), static_cast<int>(get_y(r)),
                                  static_cast<int>(get_width(r)), static_cast<int>(get_height(r))};
                SDL_DisplayID id = SDL_GetDisplayForRect(&sdl_rect);
                if (!id) {
                    return make_unexpectedf("No display found for rectangle");
                }

                return display(id);
            }

            /**
             * @brief Get current system theme
             * @return System theme
             */
            [[nodiscard]] static system_theme get_system_theme() {
                return static_cast <system_theme>(SDL_GetSystemTheme());
            }

            /**
             * @brief Get display count
             * @return Number of connected displays
             */
            [[nodiscard]] static size_t get_display_count() {
                int count = 0;
                SDL_DisplayID* ids = SDL_GetDisplays(&count);
                if (ids) {
                    SDL_free(ids);
                }
                return count > 0 ? static_cast<size_t>(count) : 0;
            }
    };

    /**
     * @brief Screen saver management
     *
     * This class provides static methods to control the system screen saver.
     * Disabling the screen saver is useful for applications that require
     * continuous display (games, video players, presentations, etc.).
     */
    class screen_saver {
        public:
            /**
             * @brief Disable the screen saver
             * @return true on success, false on failure
             * @note The screen saver is automatically re-enabled when SDL quits
             * @note You should only disable the screen saver if the user is actively
             *       using the application (e.g., playing a game, watching a video)
             */
            static bool disable() {
                return SDL_DisableScreenSaver();
            }

            /**
             * @brief Enable the screen saver
             * @return true on success, false on failure
             * @note This is the default state
             */
            static bool enable() {
                return SDL_EnableScreenSaver();
            }

            /**
             * @brief Check if the screen saver is currently enabled
             * @return true if enabled, false if disabled
             */
            [[nodiscard]] static bool is_enabled() {
                return SDL_ScreenSaverEnabled();
            }

            /**
             * @brief RAII guard for temporarily disabling the screen saver
             *
             * This class disables the screen saver on construction and
             * re-enables it on destruction (if it was originally enabled).
             *
             * Example:
             * @code
             * {
             *     screen_saver::guard no_screensaver;
             *     // Screen saver is disabled here
             *     play_video();
             * } // Screen saver is re-enabled here
             * @endcode
             */
            class guard {
                private:
                    bool was_enabled;
                    bool successfully_disabled;

                public:
                    /**
                     * @brief Construct guard and disable screen saver
                     */
                    guard()
                        : was_enabled(is_enabled()), successfully_disabled(false) {
                        if (was_enabled) {
                            successfully_disabled = disable();
                        }
                    }

                    /**
                     * @brief Destructor - restore screen saver state
                     */
                    ~guard() {
                        if (was_enabled && successfully_disabled) {
                            enable();
                        }
                    }

                    /**
                     * @brief Check if the screen saver was successfully disabled
                     */
                    [[nodiscard]] bool is_active() const {
                        return successfully_disabled;
                    }

                    // Non-copyable, non-movable
                    guard(const guard&) = delete;
                    guard& operator=(const guard&) = delete;
                    guard(guard&&) = delete;
                    guard& operator=(guard&&) = delete;
            };

            // Delete constructor - this is a static-only class
            screen_saver() = delete;
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for display_orientation
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, display_orientation value);

/**
 * @brief Stream input operator for display_orientation
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, display_orientation& value);

/**
 * @brief Stream output operator for system_theme
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, system_theme value);

/**
 * @brief Stream input operator for system_theme
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, system_theme& value);

}