//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file window.hh
 * @brief Modern C++ wrapper for SDL3 window functionality
 * 
 * This header provides RAII-managed wrappers around SDL3's window system,
 * which represents operating system windows for rendering and input.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/video/pixels.hh>
#include <sdlpp/utility/dimension.hh>
#include <string>
#include <span>
#include <vector>

namespace sdlpp {
    // Forward declarations
    class renderer;

    // SDL conversion helpers for window-specific types
    namespace detail {
        template<rect_like R>
        [[nodiscard]] SDL_Rect to_sdl_rect(const R& r) {
            return SDL_Rect{
                static_cast<int>(get_x(r)),
                static_cast<int>(get_y(r)),
                static_cast<int>(get_width(r)),
                static_cast<int>(get_height(r))
            };
        }
    }

    /**
     * @brief Smart pointer type for SDL_Window with automatic cleanup
     */
    using window_ptr = pointer <SDL_Window, SDL_DestroyWindow>;

    /**
     * @brief Window position constants
     */
    namespace window_pos {
        constexpr int undefined = SDL_WINDOWPOS_UNDEFINED;
        constexpr int centered = SDL_WINDOWPOS_CENTERED;
    }

    /**
     * @brief Window flags for creation and state
     */
    enum class window_flags : uint64_t {
        none = 0,
        fullscreen = SDL_WINDOW_FULLSCREEN,
        opengl = SDL_WINDOW_OPENGL,
        occluded = SDL_WINDOW_OCCLUDED,
        hidden = SDL_WINDOW_HIDDEN,
        borderless = SDL_WINDOW_BORDERLESS,
        resizable = SDL_WINDOW_RESIZABLE,
        minimized = SDL_WINDOW_MINIMIZED,
        maximized = SDL_WINDOW_MAXIMIZED,
        mouse_grabbed = SDL_WINDOW_MOUSE_GRABBED,
        input_focus = SDL_WINDOW_INPUT_FOCUS,
        mouse_focus = SDL_WINDOW_MOUSE_FOCUS,
        external = SDL_WINDOW_EXTERNAL,
        modal = SDL_WINDOW_MODAL,
        high_pixel_density = SDL_WINDOW_HIGH_PIXEL_DENSITY,
        mouse_capture = SDL_WINDOW_MOUSE_CAPTURE,
        always_on_top = SDL_WINDOW_ALWAYS_ON_TOP,
        utility = SDL_WINDOW_UTILITY,
        tooltip = SDL_WINDOW_TOOLTIP,
        popup_menu = SDL_WINDOW_POPUP_MENU,
        keyboard_grabbed = SDL_WINDOW_KEYBOARD_GRABBED,
        vulkan = SDL_WINDOW_VULKAN,
        metal = SDL_WINDOW_METAL,
        transparent = SDL_WINDOW_TRANSPARENT,
        not_focusable = SDL_WINDOW_NOT_FOCUSABLE
    };

    // Enable bitwise operations for window_flags
    [[nodiscard]] constexpr window_flags operator|(window_flags a, window_flags b) {
        return static_cast <window_flags>(
            static_cast <uint64_t>(a) | static_cast <uint64_t>(b)
        );
    }

    [[nodiscard]] constexpr window_flags operator&(window_flags a, window_flags b) {
        return static_cast <window_flags>(
            static_cast <uint64_t>(a) & static_cast <uint64_t>(b)
        );
    }

    [[nodiscard]] constexpr window_flags operator^(window_flags a, window_flags b) {
        return static_cast <window_flags>(
            static_cast <uint64_t>(a) ^ static_cast <uint64_t>(b)
        );
    }

    [[nodiscard]] constexpr window_flags operator~(window_flags a) {
        return static_cast <window_flags>(~static_cast <uint64_t>(a));
    }

    constexpr window_flags& operator|=(window_flags& a, window_flags b) {
        a = a | b;
        return a;
    }

    constexpr window_flags& operator&=(window_flags& a, window_flags b) {
        a = a & b;
        return a;
    }

    constexpr window_flags& operator^=(window_flags& a, window_flags b) {
        a = a ^ b;
        return a;
    }

    /**
     * @brief Window fullscreen modes
     */
    enum class fullscreen_mode : uint32_t {
        windowed = 0,
        fullscreen = SDL_WINDOW_FULLSCREEN
        // Note: SDL3 removed the desktop fullscreen mode distinction
    };

    /**
     * @brief Window progress state for taskbar indicators (SDL 3.4.0+)
     *
     * Used to show progress indicators in the window's taskbar icon
     * on Windows and Linux.
     */
    enum class window_progress_state : int {
        invalid = SDL_PROGRESS_STATE_INVALID,      ///< Invalid/unknown state
        none = SDL_PROGRESS_STATE_NONE,            ///< No progress indicator
        indeterminate = SDL_PROGRESS_STATE_INDETERMINATE, ///< Spinning/indeterminate progress
        normal = SDL_PROGRESS_STATE_NORMAL,        ///< Normal progress bar
        error = SDL_PROGRESS_STATE_ERROR,          ///< Error state (red on Windows)
        paused = SDL_PROGRESS_STATE_PAUSED         ///< Paused state (yellow on Windows)
    };

    /**
     * @brief RAII wrapper for SDL_Window
     *
     * This class provides a safe, RAII-managed interface to SDL's window
     * functionality. Windows are automatically destroyed when the object
     * goes out of scope.
     */
    class window {
        private:
            window_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty window
             */
            window() = default;

            /**
             * @brief Construct from existing SDL_Window pointer
             * @param w Existing SDL_Window pointer (takes ownership)
             */
            explicit window(SDL_Window* w)
                : ptr(w) {
            }

            /**
             * @brief Move constructor
             */
            window(window&&) = default;

            /**
             * @brief Move assignment operator
             */
            window& operator=(window&&) = default;

            // Delete copy operations - windows are move-only
            window(const window&) = delete;
            window& operator=(const window&) = delete;

            /**
             * @brief Check if the window is valid
             */
            [[nodiscard]] bool is_valid() const { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Window pointer
             */
            [[nodiscard]] SDL_Window* get() const { return ptr.get(); }

            /**
             * @brief Get window title
             * @return Window title string
             */
            [[nodiscard]] std::string get_title() const {
                if (!ptr) return "";
                const char* title = SDL_GetWindowTitle(ptr.get());
                return title ? title : "";
            }

            /**
             * @brief Set window title
             * @param title New window title
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_title(const std::string& title) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowTitle(ptr.get(), title.c_str())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get window ID
             * @return Window ID or 0 if invalid
             */
            [[nodiscard]] uint32_t get_id() const {
                return ptr ? SDL_GetWindowID(ptr.get()) : 0;
            }

            /**
             * @brief Get window position
             * @tparam P Point type to return (defaults to built-in point if available)
             * @return Expected containing position, or error message
             */
            template<point_like P = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                point_i
#else
                void
#endif
            >
            [[nodiscard]] expected <P, std::string> get_position() const
                requires (!std::is_void_v<P>) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                int x, y;
                if (!SDL_GetWindowPosition(ptr.get(), &x, &y)) {
                    return make_unexpectedf(get_error());
                }

                return P{x, y};
            }

            /**
             * @brief Set window position
             * @param x X coordinate (or window_pos constant)
             * @param y Y coordinate (or window_pos constant)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_position(int x, int y) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowPosition(ptr.get(), x, y)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Set window position
             * @param pos New position
             * @return Expected<void> - empty on success, error message on failure
             */
            template<point_like P>
            expected <void, std::string> set_position(const P& pos) {
                return set_position(static_cast <int>(pos.x), static_cast <int>(pos.y));
            }

            /**
             * @brief Get window size
             * @tparam S Size type to return (defaults to built-in size if available)
             * @return Expected containing size, or error message
             */
            template<size_like S = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                size_i
#else
                void
#endif
            >
            [[nodiscard]] expected <S, std::string> get_size() const
                requires (!std::is_void_v<S>) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                int w, h;
                if (!SDL_GetWindowSize(ptr.get(), &w, &h)) {
                    return make_unexpectedf(get_error());
                }

                return S{w, h};
            }

            /**
             * @brief Get window dimensions as type-safe dimensions
             * @return Expected containing dimensions, or error message
             */
            [[nodiscard]] expected <window_dimensions, std::string> get_dimensions() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                int w, h;
                if (!SDL_GetWindowSize(ptr.get(), &w, &h)) {
                    return make_unexpectedf(get_error());
                }

                return window_dimensions(w, h);
            }

            /**
             * @brief Set window size with type-safe dimensions
             * @param dims New dimensions (guaranteed non-negative)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_size(const window_dimensions& dims) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowSize(ptr.get(), dims.width, dims.height)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Set window size (compatibility overload)
             * @param width New width
             * @param height New height
             * @return Expected<void> - empty on success, error message on failure
             * @note Negative dimensions will be clamped to 0
             */
            expected <void, std::string> set_size(int width, int height) {
                return set_size(window_dimensions(width, height));
            }

            /**
             * @brief Set window size
             * @param s New size
             * @return Expected<void> - empty on success, error message on failure
             */
            template<size_like S>
            expected <void, std::string> set_size(const S& s) {
                return set_size(static_cast <int>(s.width), static_cast <int>(s.height));
            }

            /**
             * @brief Get minimum window size
             * @tparam S Size type to return (defaults to built-in size if available)
             * @return Expected containing minimum size, or error message
             */
            template<size_like S = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                size_i
#else
                void
#endif
            >
            [[nodiscard]] expected <S, std::string> get_minimum_size() const
                requires (!std::is_void_v<S>) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                int w, h;
                if (!SDL_GetWindowMinimumSize(ptr.get(), &w, &h)) {
                    return make_unexpectedf(get_error());
                }

                return S{w, h};
            }

            /**
             * @brief Set minimum window size
             * @tparam S Size type (must satisfy size_like)
             * @param min_size Minimum size
             * @return Expected<void> - empty on success, error message on failure
             */
            template<size_like S>
            expected <void, std::string> set_minimum_size(const S& min_size) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowMinimumSize(ptr.get(), 
                    static_cast<int>(get_width(min_size)), 
                    static_cast<int>(get_height(min_size)))) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Set minimum window size (compatibility overload)
             * @param width Minimum width
             * @param height Minimum height
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_minimum_size(int width, int height) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowMinimumSize(ptr.get(), width, height)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get maximum window size
             * @tparam S Size type to return (defaults to built-in size if available)
             * @return Expected containing maximum size, or error message
             */
            template<size_like S = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                size_i
#else
                void
#endif
            >
            [[nodiscard]] expected <S, std::string> get_maximum_size() const
                requires (!std::is_void_v<S>) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                int w, h;
                if (!SDL_GetWindowMaximumSize(ptr.get(), &w, &h)) {
                    return make_unexpectedf(get_error());
                }

                return S{w, h};
            }

            /**
             * @brief Set maximum window size
             * @tparam S Size type (must satisfy size_like)
             * @param max_size Maximum size
             * @return Expected<void> - empty on success, error message on failure
             */
            template<size_like S>
            expected <void, std::string> set_maximum_size(const S& max_size) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowMaximumSize(ptr.get(), 
                    static_cast<int>(get_width(max_size)), 
                    static_cast<int>(get_height(max_size)))) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Set maximum window size (compatibility overload)
             * @param width Maximum width
             * @param height Maximum height
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_maximum_size(int width, int height) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowMaximumSize(ptr.get(), width, height)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get window flags
             * @return Window flags
             */
            [[nodiscard]] window_flags get_flags() const {
                return ptr ? static_cast <window_flags>(SDL_GetWindowFlags(ptr.get())) : window_flags::none;
            }

            /**
             * @brief Show window
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> show() {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_ShowWindow(ptr.get())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Hide window
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> hide() {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_HideWindow(ptr.get())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Raise window above other windows
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> raise() {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_RaiseWindow(ptr.get())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Maximize window
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> maximize() {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_MaximizeWindow(ptr.get())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Minimize window
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> minimize() {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_MinimizeWindow(ptr.get())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Restore window from minimized/maximized state
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> restore() {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_RestoreWindow(ptr.get())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Set window fullscreen mode
             * @param mode Fullscreen mode
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_fullscreen(fullscreen_mode mode) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowFullscreen(ptr.get(), mode == fullscreen_mode::fullscreen)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Check if window is fullscreen
             * @return true if fullscreen
             */
            [[nodiscard]] bool is_fullscreen() const {
                return (get_flags() & window_flags::fullscreen) != window_flags::none;
            }

            /**
             * @brief Set window resizable
             * @param resizable true to allow resizing
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_resizable(bool resizable) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowResizable(ptr.get(), resizable)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Set window always on top
             * @param on_top true to keep window on top
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_always_on_top(bool on_top) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowAlwaysOnTop(ptr.get(), on_top)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get window opacity
             * @return Opacity (0.0-1.0)
             */
            [[nodiscard]] float get_opacity() const {
                if (!ptr) {
                    return 1.0f;
                }

                return SDL_GetWindowOpacity(ptr.get());
            }

            /**
             * @brief Get window display scale (DPI scale factor)
             * @return Display scale factor (1.0 = 100%, 2.0 = 200% / HiDPI)
             */
            [[nodiscard]] float display_scale() const {
                if (!ptr) {
                    return 1.0f;
                }

                return SDL_GetWindowDisplayScale(ptr.get());
            }

            /**
             * @brief Set window opacity
             * @param opacity Opacity value (0.0 = transparent, 1.0 = opaque)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_opacity(float opacity) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowOpacity(ptr.get(), opacity)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get window pixel format
             * @return Expected containing pixel format, or error message
             */
            [[nodiscard]] expected <pixel_format_enum, std::string> get_pixel_format() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                SDL_PixelFormat format = SDL_GetWindowPixelFormat(ptr.get());
                if (format == SDL_PIXELFORMAT_UNKNOWN) {
                    return make_unexpectedf(get_error());
                }

                return static_cast <pixel_format_enum>(format);
            }

            /**
             * @brief Flash window to get user attention
             * @param operation Flash operation
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> flash(SDL_FlashOperation operation) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_FlashWindow(ptr.get(), operation)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Set window icon
             * @param icon Surface to use as icon
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_icon(SDL_Surface* icon) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!icon) {
                    return make_unexpectedf("Invalid icon surface");
                }

                if (!SDL_SetWindowIcon(ptr.get(), icon)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            // ==================== Progress Indicators (SDL 3.4.0+) ====================

            /**
             * @brief Set the window's progress state (SDL 3.4.0+)
             *
             * Sets the taskbar progress indicator state. On Windows, this shows as
             * a colored overlay on the taskbar icon. On Linux, desktop environment
             * support varies.
             *
             * @param state The progress state to set
             * @return Expected<void> - empty on success, error message on failure
             *
             * Example:
             * @code
             * // Show indeterminate progress during operation
             * win.set_progress_state(window_progress_state::indeterminate);
             * // ... do work ...
             * win.set_progress_state(window_progress_state::none);
             * @endcode
             */
            expected<void, std::string> set_progress_state(window_progress_state state) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowProgressState(ptr.get(), static_cast<SDL_ProgressState>(state))) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get the window's progress state (SDL 3.4.0+)
             *
             * @return The current progress state
             */
            [[nodiscard]] window_progress_state get_progress_state() const {
                if (!ptr) {
                    return window_progress_state::invalid;
                }

                return static_cast<window_progress_state>(SDL_GetWindowProgressState(ptr.get()));
            }

            /**
             * @brief Set the window's progress value (SDL 3.4.0+)
             *
             * Sets the progress bar fill level. Only visible when the progress state
             * is set to normal, error, or paused.
             *
             * @param value Progress value (0.0 = empty, 1.0 = full)
             * @return Expected<void> - empty on success, error message on failure
             *
             * Example:
             * @code
             * win.set_progress_state(window_progress_state::normal);
             * for (float p = 0.0f; p <= 1.0f; p += 0.1f) {
             *     win.set_progress_value(p);
             *     // ... do work ...
             * }
             * win.set_progress_state(window_progress_state::none);
             * @endcode
             */
            expected<void, std::string> set_progress_value(float value) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_SetWindowProgressValue(ptr.get(), value)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get the window's progress value (SDL 3.4.0+)
             *
             * @return The current progress value (0.0 - 1.0), or -1.0 if invalid
             */
            [[nodiscard]] float get_progress_value() const {
                if (!ptr) {
                    return -1.0f;
                }

                return SDL_GetWindowProgressValue(ptr.get());
            }

            // ==================== Display and Surface ====================

            /**
             * @brief Get the display containing the window
             * @return Expected containing display ID, or error message
             */
            [[nodiscard]] expected <SDL_DisplayID, std::string> get_display() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                SDL_DisplayID display = SDL_GetDisplayForWindow(ptr.get());
                if (!display) {
                    return make_unexpectedf(get_error());
                }

                return display;
            }

            /**
             * @brief Get window surface for software rendering
             * @return Expected containing surface pointer, or error message
             * @note The returned surface is owned by the window and should not be freed
             */
            [[nodiscard]] expected <SDL_Surface*, std::string> get_surface() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                SDL_Surface* surface = SDL_GetWindowSurface(ptr.get());
                if (!surface) {
                    return make_unexpectedf(get_error());
                }

                return surface;
            }

            /**
             * @brief Update window surface
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> update_surface() {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_UpdateWindowSurface(ptr.get())) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Update specific areas of window surface  
             * @tparam Container Container type holding rect_like elements
             * @param update_rects Container of rectangles to update
             * @return Expected<void> - empty on success, error message on failure
             */
            template<typename Container>
            requires requires(Container c) {
                typename Container::value_type;
                requires rect_like<typename Container::value_type>;
                { std::begin(c) };
                { std::end(c) };
            }
            expected <void, std::string> update_surface_rects(const Container& update_rects) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                // Convert to SDL_Rect array
                std::vector<SDL_Rect> sdl_rects;
                sdl_rects.reserve(static_cast<size_t>(std::distance(std::begin(update_rects), std::end(update_rects))));
                
                for (const auto& rect : update_rects) {
                    sdl_rects.push_back(detail::to_sdl_rect(rect));
                }
                
                if (!SDL_UpdateWindowSurfaceRects(ptr.get(), sdl_rects.data(),
                                                  static_cast <int>(sdl_rects.size()))) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Update specific areas of window surface (SDL_Rect compatibility overload)
             * @param update_rects Areas to update
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> update_surface_rects(std::span <const SDL_Rect> update_rects) {
                if (!ptr) {
                    return make_unexpectedf("Invalid window");
                }

                if (!SDL_UpdateWindowSurfaceRects(ptr.get(), update_rects.data(),
                                                  static_cast <int>(update_rects.size()))) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            // Renderer-related methods

            /**
             * @brief Create a renderer for this window
             * @param driver_name Name of rendering driver (nullptr for best available)
             * @return Expected containing new renderer, or error message
             * @note This creates a new renderer. If a renderer already exists for this window,
             *       creating a new one may fail or invalidate the existing renderer.
             */
            SDLPP_EXPORT expected <renderer, std::string> create_renderer(const char* driver_name = nullptr) const;

            /**
             * @brief Get the raw SDL_Renderer pointer associated with this window
             * @return SDL_Renderer pointer or nullptr if none exists
             * @note The returned pointer is not owned by the caller and should not be freed
             */
            [[nodiscard]] SDL_Renderer* get_renderer_ptr() const {
                if (!ptr) return nullptr;
                return SDL_GetRenderer(ptr.get());
            }

            /**
             * @brief Check if this window has an associated renderer
             * @return true if a renderer exists for this window
             */
            [[nodiscard]] bool has_renderer() const {
                return get_renderer_ptr() != nullptr;
            }

            // Static factory methods

            /**
             * @brief Create a window with size_like type
             * @param title Window title
             * @param s Window size (must satisfy size_like concept)
             * @param flags Window creation flags
             * @return Expected containing new window, or error message
             */
            template<size_like S>
            static expected <window, std::string> create(
                const std::string& title,
                const S& s,
                window_flags flags = window_flags::none) {
                SDL_Window* window = SDL_CreateWindow(
                    title.c_str(),
                    static_cast<int>(get_width(s)), 
                    static_cast<int>(get_height(s)),
                    static_cast <uint64_t>(flags)
                );

                if (!window) {
                    return make_unexpectedf(get_error());
                }

                return sdlpp::window(window);
            }

            /**
             * @brief Create a window with any dimensions-like type
             * @param title Window title
             * @param dims Window dimensions (must satisfy dimensions_like concept)
             * @param flags Window creation flags
             * @return Expected containing new window, or error message
             */
            template<dimensions_like D>
            static expected <window, std::string> create(
                const std::string& title,
                const D& dims,
                window_flags flags = window_flags::none) {
                auto [w, h] = to_sdl_dimensions(dimensions <int>(
                    static_cast <int>(dims.width.value()),
                    static_cast <int>(dims.height.value())
                ));

                SDL_Window* window = SDL_CreateWindow(
                    title.c_str(),
                    w, h,
                    static_cast <uint64_t>(flags)
                );

                if (!window) {
                    return make_unexpectedf(get_error());
                }

                return sdlpp::window(window);
            }

            /**
             * @brief Create a window (compatibility overload)
             * @param title Window title
             * @param width Window width
             * @param height Window height
             * @param flags Window creation flags
             * @return Expected containing new window, or error message
             * @note Negative dimensions will be clamped to 0
             */
            static expected <window, std::string> create(
                const std::string& title,
                int width, int height,
                window_flags flags = window_flags::none) {
                return create(title, window_dimensions(width, height), flags);
            }

            /**
             * @brief Create a window with position and size
             * @tparam P Point type (must satisfy point_like)
             * @tparam S Size type (must satisfy size_like)
             * @param title Window title
             * @param pos Window position
             * @param size Window size
             * @param flags Window creation flags
             * @return Expected containing new window, or error message
             */
            template<point_like P, size_like S>
            static expected <window, std::string> create_at(
                const std::string& title,
                const P& pos,
                const S& size,
                window_flags flags = window_flags::none) {
                auto window_result = create(title, size, flags);
                if (!window_result) {
                    return window_result;
                }

                auto pos_result = window_result->set_position(pos);
                if (!pos_result) {
                    return make_unexpectedf(pos_result.error());
                }

                return window_result;
            }

            /**
             * @brief Create a window with position (compatibility overload)
             * @param title Window title
             * @param x X position (or window_pos constant)
             * @param y Y position (or window_pos constant)
             * @param width Window width
             * @param height Window height
             * @param flags Window creation flags
             * @return Expected containing new window, or error message
             */
            static expected <window, std::string> create_at(
                const std::string& title,
                int x, int y,
                int width, int height,
                window_flags flags = window_flags::none) {
                auto window_result = create(title, width, height, flags);
                if (!window_result) {
                    return window_result;
                }

                auto pos_result = window_result->set_position(x, y);
                if (!pos_result) {
                    return make_unexpectedf(pos_result.error());
                }

                return window_result;
            }

            /**
             * @brief Create a centered window
             * @param title Window title
             * @param width Window width
             * @param height Window height
             * @param flags Window creation flags
             * @return Expected containing new window, or error message
             */
            static expected <window, std::string> create_centered(
                const std::string& title,
                int width, int height,
                window_flags flags = window_flags::none) {
                return create_at(title, window_pos::centered, window_pos::centered,
                                 width, height, flags);
            }
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for window_flags
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, window_flags value);

/**
 * @brief Stream input operator for window_flags
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, window_flags& value);

/**
 * @brief Stream output operator for fullscreen_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, fullscreen_mode value);

/**
 * @brief Stream input operator for fullscreen_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, fullscreen_mode& value);

}