//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file log.hh
 * @brief Modern C++ wrapper for SDL3 logging system
 * 
 * This header provides a type-safe, stream-based logging interface for SDL3
 * with automatic source location tracking and parameter pack support.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <failsafe/detail/string_utils.hh>
#include <sstream>
#include <string>
#include <functional>
#include <memory>
#include <source_location>
#include <iostream>
#include <utility>

namespace sdlpp {
    /**
     * @brief Log priority levels
     */
    enum class log_priority {
        invalid = SDL_LOG_PRIORITY_INVALID,
        trace = SDL_LOG_PRIORITY_TRACE,
        verbose = SDL_LOG_PRIORITY_VERBOSE,
        debug = SDL_LOG_PRIORITY_DEBUG,
        info = SDL_LOG_PRIORITY_INFO,
        warn = SDL_LOG_PRIORITY_WARN,
        error = SDL_LOG_PRIORITY_ERROR,
        critical = SDL_LOG_PRIORITY_CRITICAL
    };

    /**
     * @brief Log categories
     */
    enum class log_category {
        application = SDL_LOG_CATEGORY_APPLICATION,
        error = SDL_LOG_CATEGORY_ERROR,
        assert = SDL_LOG_CATEGORY_ASSERT,
        system = SDL_LOG_CATEGORY_SYSTEM,
        audio = SDL_LOG_CATEGORY_AUDIO,
        video = SDL_LOG_CATEGORY_VIDEO,
        render = SDL_LOG_CATEGORY_RENDER,
        input = SDL_LOG_CATEGORY_INPUT,
        test = SDL_LOG_CATEGORY_TEST,
        gpu = SDL_LOG_CATEGORY_GPU,
        custom = SDL_LOG_CATEGORY_CUSTOM
    };

    /**
     * @brief Convert enum to SDL type
     */
    inline SDL_LogPriority to_sdl_priority(log_priority priority) {
        return static_cast <SDL_LogPriority>(priority);
    }

    /**
     * @brief Convert enum to SDL type
     */
    inline int to_sdl_category(log_category category) {
        return static_cast <int>(category);
    }

    // Note: string building utilities (append_to_stream, build_message) are now in detail/string_utils.hh
    // They provide enhanced type support including filesystem::path, chrono, optional, variant

    /**
     * @brief Log output callback type
     */
    using log_output_function = std::function <void(int category, log_priority priority, const std::string& message)>;

    /**
     * @brief RAII wrapper for custom log output handler
     */
    class log_output_guard {
        private:
            SDL_LogOutputFunction old_callback;
            void* old_userdata;

        public:
            log_output_guard() {
                SDL_GetLogOutputFunction(&old_callback, &old_userdata);
            }

            ~log_output_guard() {
                SDL_SetLogOutputFunction(old_callback, old_userdata);
            }

            // Non-copyable, non-movable
            log_output_guard(const log_output_guard&) = delete;
            log_output_guard& operator=(const log_output_guard&) = delete;
            log_output_guard(log_output_guard&&) = delete;
            log_output_guard& operator=(log_output_guard&&) = delete;
    };

    /**
     * @brief Logger class with source location tracking
     *
     * This class provides a modern C++ logging interface that automatically
     * tracks source location and builds messages from parameter packs.
     */
    class logger {
        private:
            struct log_context {
                int category;
                log_priority priority;
                std::source_location location;

                log_context(int cat, log_priority prio, std::source_location loc)
                    : category(cat), priority(prio), location(loc) {
                }
            };

            /**
             * @brief Format source location for logging
             */
            static std::string format_location(const std::source_location& loc) {
                std::ostringstream oss;
                oss << "[" << loc.file_name() << ":" << loc.line()
                    << " " << loc.function_name() << "] ";
                return oss.str();
            }

            /**
             * @brief Internal logging function
             */
            static void log_internal(const log_context& ctx, const std::string& message) {
                std::string full_message = format_location(ctx.location) + message;
                SDL_LogMessage(ctx.category, to_sdl_priority(ctx.priority), "%s", full_message.c_str());
            }

        public:
            /**
             * @brief Log with specific category and priority
             * @param category Log category
             * @param priority Log priority
             * @param args Message components to concatenate
             * @param loc Source location (automatically captured)
             */
            template<typename... Args>
            static void log(int category, log_priority priority,
                            const std::source_location& loc, Args&&... args) {
                log_context ctx(category, priority, loc);
                std::string message = failsafe::detail::build_message(std::forward <Args>(args)...);
                log_internal(ctx, message);
            }

            /**
             * @brief Log with enum category
             */
            template<typename... Args>
            static void log(log_category category, log_priority priority,
                            const std::source_location& loc, Args&&... args) {
                log(to_sdl_category(category), priority, loc, std::forward <Args>(args)...);
            }

            // Convenience methods for each priority level

            template<typename... Args>
            static void trace(int category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::trace, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void trace(log_category category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::trace, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void verbose(int category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::verbose, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void verbose(log_category category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::verbose, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void debug(int category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::debug, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void debug(log_category category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::debug, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void info(int category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::info, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void info(log_category category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::info, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void warn(int category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::warn, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void warn(log_category category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::warn, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void error(int category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::error, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void error(log_category category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::error, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void critical(int category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::critical, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void critical(log_category category, const std::source_location& loc, Args&&... args) {
                log(category, log_priority::critical, loc, std::forward <Args>(args)...);
            }

            // Application category shortcuts (most common)

            template<typename... Args>
            static void app_info(const std::source_location& loc, Args&&... args) {
                info(log_category::application, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void app_warn(const std::source_location& loc, Args&&... args) {
                warn(log_category::application, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void app_error(const std::source_location& loc, Args&&... args) {
                error(log_category::application, loc, std::forward <Args>(args)...);
            }

            template<typename... Args>
            static void app_debug(const std::source_location& loc, Args&&... args) {
                debug(log_category::application, loc, std::forward <Args>(args)...);
            }
    };

    /**
     * @brief Log configuration utilities
     */
    class log_config {
        public:
            /**
             * @brief Set priority for all categories
             */
            static void set_all_priorities(log_priority priority) {
                SDL_SetLogPriorities(to_sdl_priority(priority));
            }

            /**
             * @brief Set priority for a specific category
             */
            static void set_priority(int category, log_priority priority) {
                SDL_SetLogPriority(category, to_sdl_priority(priority));
            }

            /**
             * @brief Set priority for a specific category
             */
            static void set_priority(log_category category, log_priority priority) {
                SDL_SetLogPriority(to_sdl_category(category), to_sdl_priority(priority));
            }

            /**
             * @brief Get priority for a category
             */
            static log_priority get_priority(int category) {
                return static_cast <log_priority>(SDL_GetLogPriority(category));
            }

            /**
             * @brief Get priority for a category
             */
            static log_priority get_priority(log_category category) {
                return get_priority(to_sdl_category(category));
            }

            /**
             * @brief Reset all priorities to defaults
             */
            static void reset_priorities() {
                SDL_ResetLogPriorities();
            }

            /**
             * @brief Set prefix for a priority level
             */
            static bool set_priority_prefix(log_priority priority, const std::string& prefix) {
                return SDL_SetLogPriorityPrefix(to_sdl_priority(priority),
                                                prefix.empty() ? nullptr : prefix.c_str());
            }

            /**
             * @brief Set custom log output function
             */
            static void set_output_function(log_output_function func) {
                struct callback_data {
                    log_output_function func;

                    static void sdl_callback(void* userdata, int category,
                                             SDL_LogPriority priority, const char* message) {
                        auto* data = static_cast <callback_data*>(userdata);
                        if (data && data->func) {
                            data->func(category, static_cast <log_priority>(priority), message);
                        }
                    }
                };

                static std::unique_ptr <callback_data> data;

                if (func) {
                    data = std::make_unique <callback_data>(callback_data{std::move(func)});
                    SDL_SetLogOutputFunction(callback_data::sdl_callback, data.get());
                } else {
                    SDL_SetLogOutputFunction(nullptr, nullptr);
                    data.reset();
                }
            }

            /**
             * @brief Get the default log output function
             */
            static SDL_LogOutputFunction get_default_output_function() {
                return SDL_GetDefaultLogOutputFunction();
            }

            /**
             * @brief Create a scoped log output handler
             * Note: Returns a unique_ptr to work around non-movable guard
             */
            static std::unique_ptr <log_output_guard> scoped_output_function(log_output_function func) {
                auto guard = std::make_unique <log_output_guard>();
                set_output_function(std::move(func));
                return guard;
            }
    };

    /**
     * @brief Convenience macros for logging with automatic source location
     *
     * These macros automatically capture the source location at the call site.
     */
#define SDLPP_LOG(category, priority, ...) \
        ::sdlpp::logger::log(category, priority, std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_TRACE(category, ...) \
        ::sdlpp::logger::trace(category, std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_VERBOSE(category, ...) \
        ::sdlpp::logger::verbose(category, std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_DEBUG(category, ...) \
        ::sdlpp::logger::debug(category, std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_INFO(category, ...) \
        ::sdlpp::logger::info(category, std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_WARN(category, ...) \
        ::sdlpp::logger::warn(category, std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_ERROR(category, ...) \
        ::sdlpp::logger::error(category, std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_CRITICAL(category, ...) \
        ::sdlpp::logger::critical(category, std::source_location::current(), __VA_ARGS__)

    // Application category shortcuts
#define SDLPP_LOG_APP(...) \
        ::sdlpp::logger::app_info(std::source_location::current() __VA_OPT__(,) __VA_ARGS__)

#define SDLPP_LOG_APP_DEBUG(...) \
        ::sdlpp::logger::app_debug(std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_APP_WARN(...) \
        ::sdlpp::logger::app_warn(std::source_location::current(), __VA_ARGS__)

#define SDLPP_LOG_APP_ERROR(...) \
        ::sdlpp::logger::app_error(std::source_location::current(), __VA_ARGS__)
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for log_priority
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, log_priority value);

/**
 * @brief Stream input operator for log_priority
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, log_priority& value);

/**
 * @brief Stream output operator for log_category
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, log_category value);

/**
 * @brief Stream input operator for log_category
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, log_category& value);

}