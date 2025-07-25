//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file gl.hh
 * @brief Modern C++ wrapper for SDL3 OpenGL/EGL functionality
 * 
 * This header provides RAII-managed wrappers for SDL's OpenGL context management,
 * attribute configuration, and EGL integration.
 */

#include <sdlpp/core/sdl.hh>
#include <SDL3/SDL_video.h>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/video/window.hh>
#include <string>
#include <optional>
#include <memory>
#include <functional>

namespace sdlpp {
    /**
     * @brief OpenGL profile types
     */
    enum class gl_profile : Uint32 {
        core = SDL_GL_CONTEXT_PROFILE_CORE,
        compatibility = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY,
        es = SDL_GL_CONTEXT_PROFILE_ES
    };

    /**
     * @brief OpenGL context flags
     */
    enum class gl_context_flag : Uint32 {
        debug = SDL_GL_CONTEXT_DEBUG_FLAG,
        forward_compatible = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG,
        robust_access = SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG,
        reset_isolation = SDL_GL_CONTEXT_RESET_ISOLATION_FLAG
    };

    /**
     * @brief OpenGL context release behavior
     */
    enum class gl_release_behavior : Uint32 {
        none = SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE,
        flush = SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH
    };

    /**
     * @brief OpenGL context reset notification
     */
    enum class gl_reset_notification : Uint32 {
        no_notification = SDL_GL_CONTEXT_RESET_NO_NOTIFICATION,
        lose_context = SDL_GL_CONTEXT_RESET_LOSE_CONTEXT
    };

    /**
     * @brief OpenGL attribute names
     */
    enum class gl_attr {
        red_size = SDL_GL_RED_SIZE,
        green_size = SDL_GL_GREEN_SIZE,
        blue_size = SDL_GL_BLUE_SIZE,
        alpha_size = SDL_GL_ALPHA_SIZE,
        buffer_size = SDL_GL_BUFFER_SIZE,
        doublebuffer = SDL_GL_DOUBLEBUFFER,
        depth_size = SDL_GL_DEPTH_SIZE,
        stencil_size = SDL_GL_STENCIL_SIZE,
        accum_red_size = SDL_GL_ACCUM_RED_SIZE,
        accum_green_size = SDL_GL_ACCUM_GREEN_SIZE,
        accum_blue_size = SDL_GL_ACCUM_BLUE_SIZE,
        accum_alpha_size = SDL_GL_ACCUM_ALPHA_SIZE,
        stereo = SDL_GL_STEREO,
        multisamplebuffers = SDL_GL_MULTISAMPLEBUFFERS,
        multisamplesamples = SDL_GL_MULTISAMPLESAMPLES,
        accelerated_visual = SDL_GL_ACCELERATED_VISUAL,
        context_major_version = SDL_GL_CONTEXT_MAJOR_VERSION,
        context_minor_version = SDL_GL_CONTEXT_MINOR_VERSION,
        context_flags = SDL_GL_CONTEXT_FLAGS,
        context_profile_mask = SDL_GL_CONTEXT_PROFILE_MASK,
        share_with_current_context = SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
        framebuffer_srgb_capable = SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,
        context_release_behavior = SDL_GL_CONTEXT_RELEASE_BEHAVIOR,
        context_reset_notification = SDL_GL_CONTEXT_RESET_NOTIFICATION,
        context_no_error = SDL_GL_CONTEXT_NO_ERROR,
        floatbuffers = SDL_GL_FLOATBUFFERS,
        egl_platform = SDL_GL_EGL_PLATFORM
    };

    /**
     * @brief RAII wrapper for OpenGL context
     *
     * This class provides automatic management of SDL OpenGL contexts,
     * ensuring proper cleanup when the context goes out of scope.
     */
    class gl_context {
        private:
            SDL_GLContext context;

        public:
            /**
             * @brief Default constructor - creates null context
             */
            gl_context() noexcept
                : context(nullptr) {
            }

            /**
             * @brief Construct from SDL context handle
             * @param ctx SDL GL context (takes ownership)
             */
            explicit gl_context(SDL_GLContext ctx) noexcept
                : context(ctx) {
            }

            /**
             * @brief Move constructor
             */
            gl_context(gl_context&& other) noexcept
                : context(other.context) {
                other.context = nullptr;
            }

            /**
             * @brief Move assignment
             */
            gl_context& operator=(gl_context&& other) noexcept {
                if (this != &other) {
                    destroy();
                    context = other.context;
                    other.context = nullptr;
                }
                return *this;
            }

            /**
             * @brief Destructor
             */
            ~gl_context() {
                destroy();
            }

            // Delete copy operations
            gl_context(const gl_context&) = delete;
            gl_context& operator=(const gl_context&) = delete;

            /**
             * @brief Check if context is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return context != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying SDL context
             */
            [[nodiscard]] SDL_GLContext get() const noexcept { return context; }

            /**
             * @brief Release ownership of the context
             */
            [[nodiscard]] SDL_GLContext release() noexcept {
                SDL_GLContext tmp = context;
                context = nullptr;
                return tmp;
            }

            /**
             * @brief Make this context current for the window
             * @param window Window to bind context to
             * @return true if successful
             */
            [[nodiscard]] bool make_current(const window& window) const {
                return SDL_GL_MakeCurrent(window.get(), context);
            }

            /**
             * @brief Create a new OpenGL context for a window
             * @param window Window to create context for
             * @return Expected containing context or error message
             */
            [[nodiscard]] static expected <gl_context, std::string> create(const window& window) {
                SDL_GLContext ctx = SDL_GL_CreateContext(window.get());
                if (!ctx) {
                    return make_unexpected(get_error());
                }
                return gl_context(ctx);
            }

        private:
            void destroy() {
                if (context) {
                    SDL_GL_DestroyContext(context);
                    context = nullptr;
                }
            }
    };

    /**
     * @brief OpenGL configuration and utility functions
     */
    class gl {
        public:
            /**
             * @brief OpenGL attribute configuration
             */
            struct attribute_config {
                std::optional <int> red_size;
                std::optional <int> green_size;
                std::optional <int> blue_size;
                std::optional <int> alpha_size;
                std::optional <int> buffer_size;
                std::optional <bool> doublebuffer;
                std::optional <int> depth_size;
                std::optional <int> stencil_size;
                std::optional <int> accum_red_size;
                std::optional <int> accum_green_size;
                std::optional <int> accum_blue_size;
                std::optional <int> accum_alpha_size;
                std::optional <bool> stereo;
                std::optional <int> multisamplebuffers;
                std::optional <int> multisamplesamples;
                std::optional <int> accelerated_visual;
                std::optional <int> major_version;
                std::optional <int> minor_version;
                std::optional <Uint32> context_flags;
                std::optional <gl_profile> profile;
                std::optional <bool> share_with_current_context;
                std::optional <bool> framebuffer_srgb_capable;
                std::optional <gl_release_behavior> release_behavior;
                std::optional <gl_reset_notification> reset_notification;
                std::optional <bool> context_no_error;
                std::optional <int> floatbuffers;
                std::optional <int> egl_platform;

                /**
                 * @brief Apply this configuration to SDL
                 * @return true if all attributes were set successfully
                 */
                [[nodiscard]] bool apply() const {
                    bool success = true;

#define SET_ATTR(name, attr) \
                    if (name.has_value() && !set_attribute(gl_attr::attr, name.value())) { \
                        success = false; \
                    }

                    SET_ATTR(red_size, red_size)
                    SET_ATTR(green_size, green_size)
                    SET_ATTR(blue_size, blue_size)
                    SET_ATTR(alpha_size, alpha_size)
                    SET_ATTR(buffer_size, buffer_size)
                    if (doublebuffer.has_value() && !
                        set_attribute(gl_attr::doublebuffer, doublebuffer.value() ? 1 : 0)) {
                        success = false;
                    }
                    SET_ATTR(depth_size, depth_size)
                    SET_ATTR(stencil_size, stencil_size)
                    SET_ATTR(accum_red_size, accum_red_size)
                    SET_ATTR(accum_green_size, accum_green_size)
                    SET_ATTR(accum_blue_size, accum_blue_size)
                    SET_ATTR(accum_alpha_size, accum_alpha_size)
                    if (stereo.has_value() && !set_attribute(gl_attr::stereo, stereo.value() ? 1 : 0)) {
                        success = false;
                    }
                    SET_ATTR(multisamplebuffers, multisamplebuffers)
                    SET_ATTR(multisamplesamples, multisamplesamples)
                    SET_ATTR(accelerated_visual, accelerated_visual)
                    SET_ATTR(major_version, context_major_version)
                    SET_ATTR(minor_version, context_minor_version)
                    if (context_flags.has_value() && !set_attribute(gl_attr::context_flags,
                                                                    static_cast <int>(context_flags.value()))) {
                        success = false;
                    }
                    if (profile.has_value() && !set_attribute(gl_attr::context_profile_mask,
                                                              static_cast <int>(profile.value()))) {
                        success = false;
                    }
                    if (share_with_current_context.has_value() && !set_attribute(
                            gl_attr::share_with_current_context, share_with_current_context.value() ? 1 : 0)) {
                        success = false;
                    }
                    if (framebuffer_srgb_capable.has_value() && !set_attribute(
                            gl_attr::framebuffer_srgb_capable, framebuffer_srgb_capable.value() ? 1 : 0)) {
                        success = false;
                    }
                    if (release_behavior.has_value() && !set_attribute(gl_attr::context_release_behavior,
                                                                       static_cast <int>(release_behavior.value()))) {
                        success = false;
                    }
                    if (reset_notification.has_value() && !set_attribute(
                            gl_attr::context_reset_notification, static_cast <int>(reset_notification.value()))) {
                        success = false;
                    }
                    if (context_no_error.has_value() && !set_attribute(gl_attr::context_no_error,
                                                                       context_no_error.value() ? 1 : 0)) {
                        success = false;
                    }
                    SET_ATTR(floatbuffers, floatbuffers)
                    SET_ATTR(egl_platform, egl_platform)

#undef SET_ATTR

                    return success;
                }

                /**
                 * @brief Create a context configuration for OpenGL Core Profile
                 */
                static attribute_config core_profile(int major, int minor) {
                    attribute_config config;
                    config.major_version = major;
                    config.minor_version = minor;
                    config.profile = gl_profile::core;
                    config.doublebuffer = true;
                    config.depth_size = 24;
                    return config;
                }

                /**
                 * @brief Create a context configuration for OpenGL ES
                 */
                static attribute_config es_profile(int major, int minor) {
                    attribute_config config;
                    config.major_version = major;
                    config.minor_version = minor;
                    config.profile = gl_profile::es;
                    config.doublebuffer = true;
                    config.depth_size = 24;
                    return config;
                }
            };

            /**
             * @brief Load OpenGL library
             * @param path Path to library (nullptr for default)
             * @return true if successful
             */
            [[nodiscard]] static bool load_library(const char* path = nullptr) {
                return SDL_GL_LoadLibrary(path);
            }

            /**
             * @brief Unload OpenGL library
             */
            static void unload_library() {
                SDL_GL_UnloadLibrary();
            }

            /**
             * @brief Get OpenGL function pointer
             * @param proc Function name
             * @return Function pointer or nullptr
             */
            [[nodiscard]] static SDL_FunctionPointer get_proc_address(const char* proc) {
                return SDL_GL_GetProcAddress(proc);
            }

            /**
             * @brief Type-safe get proc address
             * @tparam T Function pointer type
             * @param proc Function name
             * @return Function pointer or nullptr
             */
            template<typename T>
            [[nodiscard]] static T get_proc_address(const char* proc) {
                return reinterpret_cast <T>(get_proc_address(proc));
            }

            /**
             * @brief Check if an extension is supported
             * @param extension Extension name
             * @return true if supported
             */
            [[nodiscard]] static bool extension_supported(const std::string& extension) {
                return SDL_GL_ExtensionSupported(extension.c_str());
            }

            /**
             * @brief Reset all GL attributes to defaults
             */
            static void reset_attributes() {
                SDL_GL_ResetAttributes();
            }

            /**
             * @brief Set a GL attribute
             * @param attr Attribute to set
             * @param value Value to set
             * @return true if successful
             */
            [[nodiscard]] static bool set_attribute(gl_attr attr, int value) {
                return SDL_GL_SetAttribute(static_cast <SDL_GLAttr>(attr), value);
            }

            /**
             * @brief Get a GL attribute
             * @param attr Attribute to get
             * @return Attribute value or nullopt on error
             */
            [[nodiscard]] static std::optional <int> get_attribute(gl_attr attr) {
                int value;
                if (SDL_GL_GetAttribute(static_cast <SDL_GLAttr>(attr), &value)) {
                    return value;
                }
                return std::nullopt;
            }

            /**
             * @brief Get current GL context
             * @return Current context handle (not owned)
             */
            [[nodiscard]] static SDL_GLContext get_current_context() {
                return SDL_GL_GetCurrentContext();
            }

            /**
             * @brief Get current GL window
             * @return Current window handle (not owned)
             */
            [[nodiscard]] static SDL_Window* get_current_window() {
                return SDL_GL_GetCurrentWindow();
            }

            /**
             * @brief Set swap interval (vsync)
             * @param interval 0 for immediate, 1 for vsync, -1 for adaptive vsync
             * @return true if successful
             */
            [[nodiscard]] static bool set_swap_interval(int interval) {
                return SDL_GL_SetSwapInterval(interval);
            }

            /**
             * @brief Get current swap interval
             * @return Swap interval or nullopt on error
             */
            [[nodiscard]] static std::optional <int> get_swap_interval() {
                int interval;
                if (SDL_GL_GetSwapInterval(&interval)) {
                    return interval;
                }
                return std::nullopt;
            }

            /**
             * @brief Swap window buffers
             * @param window Window to swap
             * @return true if successful
             */
            [[nodiscard]] static bool swap_window(const window& window) {
                return SDL_GL_SwapWindow(window.get());
            }

            /**
             * @brief Make a context current
             * @param window Window to bind to
             * @param context Context to make current (nullptr to unbind)
             * @return true if successful
             */
            [[nodiscard]] static bool make_current(const window& window, const gl_context& context) {
                return SDL_GL_MakeCurrent(window.get(), context.get());
            }

            /**
             * @brief Unbind current context
             * @return true if successful
             */
            [[nodiscard]] static bool clear_current() {
                return SDL_GL_MakeCurrent(nullptr, nullptr);
            }
    };

    /**
     * @brief EGL integration utilities
     */
    class egl {
        public:
            /**
             * @brief Get EGL function pointer
             * @param proc Function name
             * @return Function pointer or nullptr
             */
            [[nodiscard]] static SDL_FunctionPointer get_proc_address(const char* proc) {
                return SDL_EGL_GetProcAddress(proc);
            }

            /**
             * @brief Type-safe get proc address
             * @tparam T Function pointer type
             * @param proc Function name
             * @return Function pointer or nullptr
             */
            template<typename T>
            [[nodiscard]] static T get_proc_address(const char* proc) {
                return reinterpret_cast <T>(get_proc_address(proc));
            }

            /**
             * @brief Get current EGL display
             * @return EGL display handle
             */
            [[nodiscard]] static SDL_EGLDisplay get_current_display() {
                return SDL_EGL_GetCurrentDisplay();
            }

            /**
             * @brief Get current EGL config
             * @return EGL config handle
             */
            [[nodiscard]] static SDL_EGLConfig get_current_config() {
                return SDL_EGL_GetCurrentConfig();
            }

            /**
             * @brief Get EGL surface for window
             * @param window Window to get surface for
             * @return EGL surface handle
             */
            [[nodiscard]] static SDL_EGLSurface get_window_surface(const window& window) {
                return SDL_EGL_GetWindowSurface(window.get());
            }

            /**
             * @brief EGL attribute callbacks wrapper
             */
            class attribute_callbacks {
                private:
                    SDL_EGLAttribArrayCallback platform_callback = nullptr;
                    SDL_EGLIntArrayCallback surface_callback = nullptr;
                    SDL_EGLIntArrayCallback context_callback = nullptr;
                    void* userdata = nullptr;

                public:
                    /**
                     * @brief Set platform attributes callback
                     */
                    attribute_callbacks& set_platform_callback(SDL_EGLAttribArrayCallback cb) {
                        platform_callback = cb;
                        return *this;
                    }

                    /**
                     * @brief Set surface attributes callback
                     */
                    attribute_callbacks& set_surface_callback(SDL_EGLIntArrayCallback cb) {
                        surface_callback = cb;
                        return *this;
                    }

                    /**
                     * @brief Set context attributes callback
                     */
                    attribute_callbacks& set_context_callback(SDL_EGLIntArrayCallback cb) {
                        context_callback = cb;
                        return *this;
                    }

                    /**
                     * @brief Set userdata for callbacks
                     */
                    attribute_callbacks& set_userdata(void* data) {
                        userdata = data;
                        return *this;
                    }

                    /**
                     * @brief Apply callbacks to SDL
                     */
                    void apply() const {
                        SDL_EGL_SetAttributeCallbacks(platform_callback, surface_callback,
                                                      context_callback, userdata);
                    }
            };

            /**
             * @brief Set EGL attribute callbacks
             * @param callbacks Callbacks configuration
             */
            static void set_attribute_callbacks(const attribute_callbacks& callbacks) {
                callbacks.apply();
            }
    };

    /**
     * @brief RAII OpenGL library loader
     */
    class gl_library {
        private:
            bool loaded;

        public:
            /**
             * @brief Load OpenGL library
             * @param path Library path (nullptr for default)
             */
            explicit gl_library(const char* path = nullptr)
                : loaded(false) {
                loaded = gl::load_library(path);
            }

            /**
             * @brief Destructor - unloads library
             */
            ~gl_library() {
                if (loaded) {
                    gl::unload_library();
                }
            }

            /**
             * @brief Check if library was loaded successfully
             */
            [[nodiscard]] bool is_loaded() const noexcept { return loaded; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_loaded(); }

            // Delete copy/move operations
            gl_library(const gl_library&) = delete;
            gl_library& operator=(const gl_library&) = delete;
            gl_library(gl_library&&) = delete;
            gl_library& operator=(gl_library&&) = delete;
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for gl_profile
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gl_profile value);

/**
 * @brief Stream input operator for gl_profile
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gl_profile& value);

/**
 * @brief Stream output operator for gl_context_flag
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gl_context_flag value);

/**
 * @brief Stream input operator for gl_context_flag
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gl_context_flag& value);

/**
 * @brief Stream output operator for gl_release_behavior
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gl_release_behavior value);

/**
 * @brief Stream input operator for gl_release_behavior
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gl_release_behavior& value);

/**
 * @brief Stream output operator for gl_reset_notification
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gl_reset_notification value);

/**
 * @brief Stream input operator for gl_reset_notification
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gl_reset_notification& value);

/**
 * @brief Stream output operator for gl_attr
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gl_attr value);

/**
 * @brief Stream input operator for gl_attr
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gl_attr& value);

}