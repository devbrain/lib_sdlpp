#pragma once

/**
 * @file platform.hh
 * @brief Platform detection and system information utilities
 * 
 * This header provides access to platform-specific information and
 * compile-time platform detection macros.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/system/power_state.hh>
#include <string>
#include <string_view>
#include <filesystem>

namespace sdlpp {
    /**
     * @brief Platform identification and information
     */
    namespace platform {
        /**
         * @brief Get the name of the platform
         *
         * @return Platform name (e.g., "Windows", "Mac OS X", "Linux", "iOS", "Android")
         */
        [[nodiscard]] inline std::string get_platform() noexcept {
            const char* platform = SDL_GetPlatform();
            return platform ? platform : "";
        }

        /**
         * @brief Check if running on Windows
         * @return true if running on Windows
         */
        [[nodiscard]] inline constexpr bool is_windows() noexcept {
#ifdef SDL_PLATFORM_WINDOWS
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on macOS
         * @return true if running on macOS
         */
        [[nodiscard]] inline constexpr bool is_macos() noexcept {
#ifdef SDL_PLATFORM_MACOS
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on Linux
         * @return true if running on Linux
         */
        [[nodiscard]] inline constexpr bool is_linux() noexcept {
#ifdef SDL_PLATFORM_LINUX
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on Android
         * @return true if running on Android
         */
        [[nodiscard]] inline constexpr bool is_android() noexcept {
#ifdef SDL_PLATFORM_ANDROID
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on iOS
         * @return true if running on iOS
         */
        [[nodiscard]] inline constexpr bool is_ios() noexcept {
#ifdef SDL_PLATFORM_IOS
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on tvOS
         * @return true if running on tvOS
         */
        [[nodiscard]] inline constexpr bool is_tvos() noexcept {
#ifdef SDL_PLATFORM_TVOS
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on any Apple platform
         * @return true if running on macOS, iOS, or tvOS
         */
        [[nodiscard]] inline constexpr bool is_apple() noexcept {
#ifdef SDL_PLATFORM_APPLE
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on BSD
         * @return true if running on BSD
         */
        [[nodiscard]] inline constexpr bool is_bsd() noexcept {
#if defined(SDL_PLATFORM_FREEBSD) || defined(SDL_PLATFORM_NETBSD) || \
            defined(SDL_PLATFORM_OPENBSD) || defined(SDL_PLATFORM_BSD)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on Unix-like system
         * @return true if running on Unix-like system
         */
        [[nodiscard]] inline constexpr bool is_unix() noexcept {
#if defined(SDL_PLATFORM_UNIX) || defined(SDL_PLATFORM_LINUX) || \
    defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS) || \
    defined(SDL_PLATFORM_FREEBSD) || defined(SDL_PLATFORM_NETBSD) || \
    defined(SDL_PLATFORM_OPENBSD)
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on Emscripten (WebAssembly)
         * @return true if running on Emscripten
         */
        [[nodiscard]] inline constexpr bool is_emscripten() noexcept {
#ifdef SDL_PLATFORM_EMSCRIPTEN
            return true;
#else
            return false;
#endif
        }

        /**
         * @brief Check if running on a tablet device
         * @return true if running on a tablet
         */
        [[nodiscard]] inline bool is_tablet() noexcept {
            return SDL_IsTablet();
        }

        /**
         * @brief Check if running on a TV device
         * @return true if running on a TV
         */
        [[nodiscard]] inline bool is_tv() noexcept {
            return SDL_IsTV();
        }

        /**
         * @brief Check if running on a Chromebook
         * @return true if running on a Chromebook
         * @note This function may not be available in all SDL3 versions
         */
        [[nodiscard]] inline bool is_chromebook() noexcept {
#ifdef SDL_HAS_CHROMEBOOK_DETECTION
            return SDL_IsChromebook();
#else
            return false;
#endif
        }

        /**
         * @brief Check if running in Samsung DeX mode
         * @return true if running in DeX mode
         * @note This function may not be available in all SDL3 versions
         */
        [[nodiscard]] inline bool is_dex_mode() noexcept {
#ifdef SDL_HAS_DEX_DETECTION
            return SDL_IsDeXMode();
#else
            return false;
#endif
        }

        /**
         * @brief Platform categories
         */
        enum class platform_category {
            desktop, ///< Desktop platforms (Windows, macOS, Linux)
            mobile, ///< Mobile platforms (iOS, Android)
            web, ///< Web platforms (Emscripten)
            console, ///< Console platforms
            embedded, ///< Embedded platforms
            unknown ///< Unknown platform
        };

        /**
         * @brief Get the platform category
         * @return Platform category
         */
        [[nodiscard]] inline platform_category get_platform_category() noexcept {
            if (is_windows() || is_macos() || (is_linux() && !is_android())) {
                return platform_category::desktop;
            }
            if (is_ios() || is_android()) {
                return platform_category::mobile;
            }
            if (is_emscripten()) {
                return platform_category::web;
            }
            return platform_category::unknown;
        }

        /**
         * @brief Platform information structure
         */
        struct platform_info {
            std::string_view name;
            platform_category category;
            bool is_64bit;
            bool is_big_endian;
            bool is_desktop;
            bool is_mobile;
            bool is_web;
        };

        /**
         * @brief Get comprehensive platform information
         * @return Platform information structure
         */
        [[nodiscard]] inline platform_info get_platform_info() noexcept {
            auto category = get_platform_category();
            return platform_info{
                .name = get_platform(),
                .category = category,
                .is_64bit = sizeof(void*) == 8,
                .is_big_endian = SDL_BYTEORDER == SDL_BIG_ENDIAN,
                .is_desktop = category == platform_category::desktop,
                .is_mobile = category == platform_category::mobile,
                .is_web = category == platform_category::web
            };
        }
    }

    /**
     * @brief Power management information
     */
    namespace power {

        /**
         * @brief Power information structure
         */
        struct power_info {
            power_state state{power_state::unknown};
            int seconds_left{-1}; ///< Seconds of battery life left (-1 if unknown)
            int percent_left{-1}; ///< Percentage of battery life left (-1 if unknown)

            [[nodiscard]] bool is_on_battery() const noexcept {
                return state == power_state::on_battery;
            }

            [[nodiscard]] bool is_plugged_in() const noexcept {
                return state == power_state::no_battery ||
                       state == power_state::charging ||
                       state == power_state::charged;
            }

            [[nodiscard]] bool has_battery() const noexcept {
                return state != power_state::no_battery && state != power_state::unknown;
            }
        };

        /**
         * @brief Get current power information
         * @return Power information structure
         */
        [[nodiscard]] inline power_info get_power_info() noexcept {
            power_info info;
            info.state = static_cast<power_state>(
                SDL_GetPowerInfo(&info.seconds_left, &info.percent_left)
            );
            return info;
        }
    }

    /**
     * @brief System directories
     */
    namespace directories {
        /**
         * @brief User folder types
         */
        enum class folder_type {
            home = SDL_FOLDER_HOME, ///< The home folder for the current user
            desktop = SDL_FOLDER_DESKTOP, ///< The desktop folder for the current user
            documents = SDL_FOLDER_DOCUMENTS, ///< The documents folder for the current user
            downloads = SDL_FOLDER_DOWNLOADS, ///< The downloads folder for the current user
            music = SDL_FOLDER_MUSIC, ///< The music folder for the current user
            pictures = SDL_FOLDER_PICTURES, ///< The pictures folder for the current user
            publicshare = SDL_FOLDER_PUBLICSHARE, ///< The public share folder for the current user
            savedgames = SDL_FOLDER_SAVEDGAMES, ///< The saved games folder for the current user
            screenshots = SDL_FOLDER_SCREENSHOTS, ///< The screenshots folder for the current user
            templates = SDL_FOLDER_TEMPLATES, ///< The templates folder for the current user
            videos = SDL_FOLDER_VIDEOS ///< The videos folder for the current user
        };

        /**
         * @brief Get the path where the application was launched from
         * @return Base path with trailing separator, or empty on error
         * @note The returned path is owned by SDL and should not be freed
         */
        [[nodiscard]] inline std::filesystem::path get_base_path() noexcept {
            const char* path = SDL_GetBasePath();
            return path ? std::filesystem::path(path) : std::filesystem::path{};
        }

        /**
         * @brief Get the user-and-app-specific path for writing preferences
         * @param org Organization name
         * @param app Application name
         * @return Preferences path with trailing separator, or empty on error
         * @note The returned path is owned by the caller and must be freed with SDL_free
         */
        [[nodiscard]] inline std::filesystem::path get_pref_path(const std::string& org,
                                                                 const std::string& app) noexcept {
            char* path = SDL_GetPrefPath(org.c_str(), app.c_str());
            if (!path) return {};
            std::filesystem::path result(path);
            SDL_free(path);
            return result;
        }

        /**
         * @brief Get a specific user folder
         * @param folder The type of folder to retrieve
         * @return Path to the folder, or empty on error
         */
        [[nodiscard]] inline std::filesystem::path get_user_folder(folder_type folder) noexcept {
            const char* path = SDL_GetUserFolder(static_cast <SDL_Folder>(folder));
            return path ? std::filesystem::path(path) : std::filesystem::path{};
        }

        // Convenience functions for common folders
        [[nodiscard]] inline std::filesystem::path get_home_folder() noexcept {
            return get_user_folder(folder_type::home);
        }

        [[nodiscard]] inline std::filesystem::path get_desktop_folder() noexcept {
            return get_user_folder(folder_type::desktop);
        }

        [[nodiscard]] inline std::filesystem::path get_documents_folder() noexcept {
            return get_user_folder(folder_type::documents);
        }

        [[nodiscard]] inline std::filesystem::path get_downloads_folder() noexcept {
            return get_user_folder(folder_type::downloads);
        }

        [[nodiscard]] inline std::filesystem::path get_music_folder() noexcept {
            return get_user_folder(folder_type::music);
        }

        [[nodiscard]] inline std::filesystem::path get_pictures_folder() noexcept {
            return get_user_folder(folder_type::pictures);
        }

        [[nodiscard]] inline std::filesystem::path get_publicshare_folder() noexcept {
            return get_user_folder(folder_type::publicshare);
        }

        [[nodiscard]] inline std::filesystem::path get_saved_games_folder() noexcept {
            return get_user_folder(folder_type::savedgames);
        }

        [[nodiscard]] inline std::filesystem::path get_screenshots_folder() noexcept {
            return get_user_folder(folder_type::screenshots);
        }

        [[nodiscard]] inline std::filesystem::path get_templates_folder() noexcept {
            return get_user_folder(folder_type::templates);
        }

        [[nodiscard]] inline std::filesystem::path get_videos_folder() noexcept {
            return get_user_folder(folder_type::videos);
        }
    }

    /**
     * @brief Environment utilities
     */
    namespace environment {
        /**
         * @brief Get an environment variable
         * @param name Variable name
         * @return Variable value, or empty if not found
         */
        [[nodiscard]] inline std::string get_env(const std::string& name) noexcept {
            const char* value = SDL_getenv(name.c_str());
            return value ? std::string(value) : std::string{};
        }

        /**
         * @brief Set an environment variable
         * @param name Variable name
         * @param value Variable value
         * @param overwrite Whether to overwrite if already exists
         * @return true on success
         * @note This uses SDL_setenv_unsafe which is not thread-safe
         */
        [[nodiscard]] inline bool set_env(const std::string& name,
                                          const std::string& value,
                                          bool overwrite = true) noexcept {
            return SDL_setenv_unsafe(name.c_str(), value.c_str(), overwrite ? 1 : 0) == 0;
        }

        /**
         * @brief Unset an environment variable
         * @param name Variable name
         * @return true on success
         * @note This uses SDL_unsetenv_unsafe which is not thread-safe
         */
        [[nodiscard]] inline bool unset_env(const std::string& name) noexcept {
            return SDL_unsetenv_unsafe(name.c_str()) == 0;
        }
    }

    /**
     * @brief Android-specific functionality
     * @note These functions are only available on Android
     */
    namespace android {
        /**
         * @brief Android external storage states
         */
        enum class external_storage_state : Uint32 {
            read = 0x01, ///< External storage is readable
            write = 0x02 ///< External storage is writable
        };

#ifdef SDL_PLATFORM_ANDROID

    /**
     * @brief Get the Android SDK version
     * @return SDK version number
     */
    [[nodiscard]] inline int get_sdk_version() noexcept {
        return SDL_GetAndroidSDKVersion();
    }

    /**
     * @brief Get the path to internal storage
     * @return Path to internal storage
     */
    [[nodiscard]] inline std::filesystem::path get_internal_storage_path() noexcept {
        const char* path = SDL_GetAndroidInternalStoragePath();
        return path ? std::filesystem::path(path) : std::filesystem::path{};
    }

    /**
     * @brief Get the path to external storage
     * @return Path to external storage
     */
    [[nodiscard]] inline std::filesystem::path get_external_storage_path() noexcept {
        const char* path = SDL_GetAndroidExternalStoragePath();
        return path ? std::filesystem::path(path) : std::filesystem::path{};
    }

    /**
     * @brief Get the path to the cache directory
     * @return Path to cache directory
     */
    [[nodiscard]] inline std::filesystem::path get_cache_path() noexcept {
        const char* path = SDL_GetAndroidCachePath();
        return path ? std::filesystem::path(path) : std::filesystem::path{};
    }

    /**
     * @brief Get the external storage state
     * @return Bitmask of external storage states
     */
    [[nodiscard]] inline Uint32 get_external_storage_state() noexcept {
        return SDL_GetAndroidExternalStorageState();
    }

    /**
     * @brief Check if external storage is readable
     * @return true if external storage can be read
     */
    [[nodiscard]] inline bool is_external_storage_readable() noexcept {
        return (get_external_storage_state() & static_cast<Uint32>(external_storage_state::read)) != 0;
    }

    /**
     * @brief Check if external storage is writable
     * @return true if external storage can be written
     */
    [[nodiscard]] inline bool is_external_storage_writable() noexcept {
        return (get_external_storage_state() & static_cast<Uint32>(external_storage_state::write)) != 0;
    }

    /**
     * @brief Request an Android permission
     * @param permission Permission string (e.g., "android.permission.CAMERA")
     * @return true if permission request was initiated
     * @note The actual permission grant happens asynchronously
     */
    [[nodiscard]] inline bool request_permission(const std::string& permission) noexcept {
        return SDL_RequestAndroidPermission(permission.c_str());
    }

    /**
     * @brief Send the Android back button event
     * @note This is useful when you want to override back button behavior
     */
    inline void send_back_button() noexcept {
        SDL_SendAndroidBackButton();
    }

    /**
     * @brief Show an Android toast message
     * @param message Message to display
     * @param duration Duration in milliseconds (0 for short, 1 for long)
     * @param gravity Gravity flags (combination of top/bottom/left/right/center)
     * @param x_offset X offset from gravity position
     * @param y_offset Y offset from gravity position
     * @return true on success
     */
    [[nodiscard]] inline bool show_toast(const std::string& message,
                                        int duration = 0,
                                        int gravity = -1,
                                        int x_offset = 0,
                                        int y_offset = 0) noexcept {
        return SDL_ShowAndroidToast(message.c_str(), duration, gravity, x_offset, y_offset);
    }

    /**
     * @brief Get the Android activity object
     * @return JNI jobject for the activity (void* for type safety)
     * @note This is for advanced JNI usage
     */
    [[nodiscard]] inline void* get_activity() noexcept {
        return SDL_GetAndroidActivity();
    }

    /**
     * @brief Get the JNI environment
     * @return JNI environment pointer (void* for type safety)
     * @note This is for advanced JNI usage
     */
    [[nodiscard]] inline void* get_jni_env() noexcept {
        return SDL_GetAndroidJNIEnv();
    }

    /**
     * @brief Send a message to the Android activity
     * @param command Command identifier
     * @param param Parameter value
     * @return Result value from the activity
     * @note This is for custom activity communication
     */
    [[nodiscard]] inline int send_message(int command, int param) noexcept {
        return SDL_SendAndroidMessage(command, param);
    }

#else // Not Android - provide stubs that return sensible defaults

        [[nodiscard]] inline int get_sdk_version() noexcept { return 0; }
        [[nodiscard]] inline std::filesystem::path get_internal_storage_path() noexcept { return {}; }
        [[nodiscard]] inline std::filesystem::path get_external_storage_path() noexcept { return {}; }
        [[nodiscard]] inline std::filesystem::path get_cache_path() noexcept { return {}; }
        [[nodiscard]] inline Uint32 get_external_storage_state() noexcept { return 0; }
        [[nodiscard]] inline bool is_external_storage_readable() noexcept { return false; }
        [[nodiscard]] inline bool is_external_storage_writable() noexcept { return false; }
        [[nodiscard]] inline bool request_permission(const std::string&) noexcept { return false; }

        inline void send_back_button() noexcept {
        }

        [[nodiscard]] inline bool show_toast(const std::string&, int = 0, int = -1, int = 0, int = 0) noexcept {
            return false;
        }

        [[nodiscard]] inline void* get_activity() noexcept { return nullptr; }
        [[nodiscard]] inline void* get_jni_env() noexcept { return nullptr; }
        [[nodiscard]] inline int send_message(int, int) noexcept { return 0; }

#endif // SDL_PLATFORM_ANDROID
    }

    /**
     * @brief iOS-specific functionality
     * @note These functions are only available on iOS
     */
    namespace ios {
#ifdef SDL_PLATFORM_IOS

    /**
     * @brief iOS animation callback function type
     */
    using animation_callback = void(*)(void*);

    /**
     * @brief Set the animation callback
     * @param window Window to animate
     * @param interval Frame interval (e.g., 1 for 60fps, 2 for 30fps)
     * @param callback Animation callback function
     * @param userdata User data passed to callback
     * @return true on success
     */
    [[nodiscard]] inline bool set_animation_callback(SDL_Window* window,
                                                    int interval,
                                                    animation_callback callback,
                                                    void* userdata) noexcept {
        return SDL_SetiOSAnimationCallback(window, interval, callback, userdata);
    }

    /**
     * @brief Set whether the iOS event pump is enabled
     * @param enabled true to enable event pump
     */
    inline void set_event_pump(bool enabled) noexcept {
        SDL_SetiOSEventPump(enabled);
    }

#else // Not iOS - provide stubs

        using animation_callback = void(*)(void*);

        [[nodiscard]] inline bool set_animation_callback(SDL_Window*, int, animation_callback, void*) noexcept {
            return false;
        }

        inline void set_event_pump(bool) noexcept {
        }

#endif // SDL_PLATFORM_IOS
    }

    /**
     * @brief Linux-specific functionality
     * @note These functions are only available on Linux
     */
    namespace linux_platform {
#ifdef SDL_PLATFORM_LINUX

        /**
         * @brief Set thread priority on Linux
         * @param thread_id Thread ID (pthread_t)
         * @param priority Priority value
         * @return true on success
         */
        [[nodiscard]] inline bool set_thread_priority(Sint64 thread_id, int priority) noexcept {
            return SDL_SetLinuxThreadPriority(thread_id, priority);
        }

        /**
         * @brief Set thread priority and scheduling policy on Linux
         * @param thread_id Thread ID (pthread_t)
         * @param policy Scheduling policy (e.g., SCHED_FIFO, SCHED_RR)
         * @param priority Priority value
         * @return true on success
         */
        [[nodiscard]] inline bool set_thread_priority_and_policy(Sint64 thread_id,
                                                                 int policy,
                                                                 int priority) noexcept {
            return SDL_SetLinuxThreadPriorityAndPolicy(thread_id, policy, priority);
        }

#else // Not Linux - provide stubs

    [[nodiscard]] inline bool set_thread_priority(Sint64, int) noexcept { return false; }
    [[nodiscard]] inline bool set_thread_priority_and_policy(Sint64, int, int) noexcept { return false; }

#endif // SDL_PLATFORM_LINUX
    }

    /**
     * @brief Windows-specific functionality
     * @note These functions are only available on Windows
     */
    namespace windows {
#ifdef SDL_PLATFORM_WINDOWS

    /**
     * @brief Windows message hook function type
     * @note The callback receives a pointer to the Win32 MSG structure
     * @return true to continue processing the message, false to drop it
     */
    using message_hook = SDL_WindowsMessageHook;

    /**
     * @brief Set Windows message hook
     * @param callback Message hook callback (receives userdata and MSG*)
     * @param userdata User data passed to callback
     */
    inline void set_message_hook(message_hook callback, void* userdata) noexcept {
        SDL_SetWindowsMessageHook(callback, userdata);
    }

#else // Not Windows - provide stub

        using message_hook = bool(*)(void*, void*);

        inline void set_message_hook(message_hook, void*) noexcept {
        }

#endif // SDL_PLATFORM_WINDOWS
    }

    /**
     * @brief X11-specific functionality
     * @note These functions are only available on X11 systems
     */
    namespace x11 {
#if defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_UNIX)

        /**
         * @brief X11 event hook function type
         * @note The xevent parameter is actually XEvent* on X11 systems
         */
        using event_hook = SDL_X11EventHook;

        /**
         * @brief Set X11 event hook
         * @param callback Event hook callback
         * @param userdata User data passed to callback
         */
        inline void set_event_hook(event_hook callback, void* userdata) noexcept {
            SDL_SetX11EventHook(callback, userdata);
        }

#else // Not X11 - provide stub

        using event_hook = bool(*)(void*, void*);
        inline void set_event_hook(event_hook, void*) noexcept {}

#endif // X11 platforms
    } // namespace x11
} // namespace sdlpp

// Stream operators for enums
#include <iosfwd>

namespace sdlpp::android {
/**
 * @brief Stream output operator for external_storage_state
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, external_storage_state value);

/**
 * @brief Stream input operator for external_storage_state
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, external_storage_state& value);

} // namespace sdlpp::android

namespace sdlpp::directories {
/**
 * @brief Stream output operator for folder_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, folder_type value);

/**
 * @brief Stream input operator for folder_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, folder_type& value);

} // namespace sdlpp::directories

namespace sdlpp::platform {
/**
 * @brief Stream output operator for platform_category
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, platform_category value);

/**
 * @brief Stream input operator for platform_category
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, platform_category& value);

} // namespace sdlpp::platform
