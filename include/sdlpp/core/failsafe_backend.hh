//
// Created by igor on 7/20/25.
//

#pragma once

/**
 * @file failsafe_backend.hh
 * @brief SDL++ logger backend for failsafe library
 * 
 * This header provides a logger backend implementation that bridges
 * the failsafe logging system with SDL++ logging infrastructure.
 */

#include <sdlpp/core/log.hh>
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

namespace sdlpp {
    /**
     * @brief Logger backend for failsafe library that outputs to SDL++ logging system
     *
     * This class provides a configurable logger backend that can be used with
     * the failsafe library. It supports:
     * - Mapping failsafe log levels to SDL++ log priorities
     * - Category-based logging with custom category mapping
     * - Optional timestamps, thread IDs, and color output
     * - Thread-safe operation
     */
    class failsafe_backend {
        public:
            /**
             * @brief Configuration options for the backend
             */
            struct config {
                bool show_timestamp = true;      ///< Include timestamp in output
                bool show_thread_id = true;      ///< Include thread ID in output
                bool show_file_line = true;      ///< Include file:line information
                bool use_colors = false;         ///< Use ANSI colors (not applicable for SDL logging)
                std::string timestamp_format = "%Y-%m-%d %H:%M:%S"; ///< strftime format for timestamps
            };

            /**
             * @brief Default constructor with default configuration
             */
            failsafe_backend() = default;

            /**
             * @brief Constructor with custom configuration
             * @param cfg Configuration options
             */
            explicit failsafe_backend(const config& cfg) : config_(cfg) {
            }

            /**
             * @brief Set configuration
             * @param cfg New configuration
             */
            void set_config(const config& cfg) {
                std::lock_guard<std::mutex> lock(mutex_);
                config_ = cfg;
            }

            /**
             * @brief Get current configuration
             * @return Current configuration
             */
            config get_config() const {
                std::lock_guard<std::mutex> lock(mutex_);
                return config_;
            }

            /**
             * @brief Map a failsafe category to an SDL++ log category
             * @param failsafe_category Category name from failsafe
             * @param sdl_category SDL++ log category to map to
             */
            void map_category(const std::string& failsafe_category, int sdl_category) {
                std::lock_guard<std::mutex> lock(mutex_);
                category_map_[failsafe_category] = sdl_category;
            }

            /**
             * @brief Map a failsafe category to an SDL++ log category
             * @param failsafe_category Category name from failsafe
             * @param sdl_category SDL++ log category enum to map to
             */
            void map_category(const std::string& failsafe_category, log_category sdl_category) {
                map_category(failsafe_category, to_sdl_category(sdl_category));
            }

            /**
             * @brief Clear all category mappings
             */
            void clear_category_mappings() {
                std::lock_guard<std::mutex> lock(mutex_);
                category_map_.clear();
            }

            /**
             * @brief Set default SDL++ category for unmapped failsafe categories
             * @param category Default category (defaults to application)
             */
            void set_default_category(int category) {
                std::lock_guard<std::mutex> lock(mutex_);
                default_category_ = category;
            }

            /**
             * @brief Set default SDL++ category for unmapped failsafe categories
             * @param category Default category enum
             */
            void set_default_category(log_category category) {
                set_default_category(to_sdl_category(category));
            }

            /**
             * @brief Get the logger function object for failsafe
             * @return Function object that can be passed to failsafe's set_logger()
             */
            auto get_logger() {
                return [this](int level, const char* category, const char* file, int line,
                              const std::string& message) {
                    log(level, category, file, line, message);
                };
            }

            /**
             * @brief Convenience method to create and configure a backend
             * @param cfg Configuration options
             * @return Configured backend instance
             */
            static failsafe_backend create(const config& cfg) {
                return failsafe_backend(cfg);
            }
            
            /**
             * @brief Convenience method to create backend with default config
             * @return Backend with default configuration
             */
            static failsafe_backend create() {
                return failsafe_backend();
            }

        private:
            /**
             * @brief Convert failsafe log level to SDL++ log priority
             * @param level Failsafe log level
             * @return Corresponding SDL++ log priority
             */
            static log_priority map_level(int level) {
                // Failsafe levels are defined as:
                // LOGGER_LEVEL_TRACE = 0
                // LOGGER_LEVEL_DEBUG = 1
                // LOGGER_LEVEL_INFO  = 2
                // LOGGER_LEVEL_WARN  = 3
                // LOGGER_LEVEL_ERROR = 4
                // LOGGER_LEVEL_FATAL = 5
                switch (level) {
                    case 0:  // LOGGER_LEVEL_TRACE
                        return log_priority::trace;
                    case 1:  // LOGGER_LEVEL_DEBUG
                        return log_priority::debug;
                    case 2:  // LOGGER_LEVEL_INFO
                        return log_priority::info;
                    case 3:  // LOGGER_LEVEL_WARN
                        return log_priority::warn;
                    case 4:  // LOGGER_LEVEL_ERROR
                        return log_priority::error;
                    case 5:  // LOGGER_LEVEL_FATAL
                        return log_priority::critical;
                    default:
                        // Unknown levels default to info
                        return log_priority::info;
                }
            }

            /**
             * @brief Get SDL category for a failsafe category
             * @param category Failsafe category name
             * @return SDL category ID
             */
            int get_sdl_category(const char* category) const {
                if (!category) {
                    return default_category_;
                }

                std::lock_guard<std::mutex> lock(mutex_);
                auto it = category_map_.find(category);
                if (it != category_map_.end()) {
                    return it->second;
                }
                return default_category_;
            }

            /**
             * @brief Format timestamp
             * @return Formatted timestamp string
             */
            std::string format_timestamp() const {
                auto now = std::chrono::system_clock::now();
                auto time_t_val = std::chrono::system_clock::to_time_t(now);
                std::tm tm{};
#if defined(_MSC_VER)
                localtime_s(&tm, &time_t_val);
#else
                localtime_r(&time_t_val, &tm);
#endif

                std::ostringstream oss;
                oss << std::put_time(&tm, config_.timestamp_format.c_str());
                
                // Add milliseconds
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;
                oss << "." << std::setfill('0') << std::setw(3) << ms.count();
                
                return oss.str();
            }

            /**
             * @brief Get current thread ID as string
             * @return Thread ID string
             */
            static std::string get_thread_id() {
                std::ostringstream oss;
                oss << std::this_thread::get_id();
                return oss.str();
            }

            /**
             * @brief Main logging function
             * @param level Failsafe log level
             * @param category Category name
             * @param file Source file name
             * @param line Line number
             * @param message Log message
             */
            void log(int level, const char* category, const char* file, int line,
                     const std::string& message) {
                std::ostringstream oss;

                // Build the log message
                if (config_.show_timestamp) {
                    oss << "[" << format_timestamp() << "] ";
                }

                if (config_.show_thread_id) {
                    oss << "[" << get_thread_id() << "] ";
                }

                if (category) {
                    oss << "[" << category << "] ";
                }

                if (config_.show_file_line && file) {
                    oss << "[" << file << ":" << line << "] ";
                }

                oss << message;

                // Log to SDL++
                auto priority = map_level(level);
                auto sdl_category = get_sdl_category(category);
                
                // Use SDL logging directly to avoid double source location info
                SDL_LogMessage(sdl_category, to_sdl_priority(priority), "%s", oss.str().c_str());
            }

        private:
            mutable std::mutex mutex_;
            config config_;
            std::unordered_map<std::string, int> category_map_;
            int default_category_ = to_sdl_category(log_category::application);
    };

    /**
     * @brief Convenience function to create a default SDL++ backend for failsafe
     * @param show_timestamp Whether to include timestamps
     * @param show_thread_id Whether to include thread IDs
     * @return Logger function object ready to use with failsafe
     */
    inline auto create_failsafe_sdl_backend(bool show_timestamp = true, 
                                           bool show_thread_id = true) {
        failsafe_backend::config cfg;
        cfg.show_timestamp = show_timestamp;
        cfg.show_thread_id = show_thread_id;
        cfg.show_file_line = true;
        cfg.use_colors = false;  // SDL logging handles its own formatting
        
        static failsafe_backend backend(cfg);
        return backend.get_logger();
    }

} // namespace sdlpp