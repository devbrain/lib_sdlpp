//
// Created by igor on 7/20/25.
//

/**
 * @file example_failsafe_logger.cc
 * @brief Example demonstrating failsafe library integration with SDL++ logging
 *
 * This example shows how to:
 * - Configure the SDL++ backend for failsafe
 * - Map failsafe categories to SDL++ categories
 * - Use failsafe logging with SDL++ output
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/log.hh>
#include <sdlpp/core/failsafe_backend.hh>
#include <sdlpp/app/entry_point.hh>
#include <sdlpp/app/game_application.hh>

// Include failsafe headers
#include <failsafe/failsafe.hh>

#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

// Example custom failsafe logger categories
namespace logger_categories {
    constexpr const char* NETWORK = "network";
    constexpr const char* DATABASE = "database";
    constexpr const char* BUSINESS = "business";
    constexpr const char* SECURITY = "security";
}

// Example application that uses failsafe logging
class failsafe_logger_app : public sdlpp::game_application {
protected:
    sdlpp::window_config get_window_config() override {
        return {"Failsafe Logger Example", 800, 600, sdlpp::window_flags::none, 60};
    }

    void on_ready() override {
        // Configure SDL++ logging to see all levels
        sdlpp::log_config::set_all_priorities(sdlpp::log_priority::trace);

        // Create and configure the SDL++ backend for failsafe
        sdlpp::failsafe_backend::config cfg;
        cfg.show_timestamp = true;
        cfg.show_thread_id = true;
        cfg.show_file_line = true;
        cfg.use_colors = false;  // SDL handles its own formatting

        backend_ = std::make_unique<sdlpp::failsafe_backend>(cfg);

        // Map failsafe categories to SDL++ categories
        backend_->map_category(logger_categories::NETWORK, sdlpp::log_category::system);
        backend_->map_category(logger_categories::DATABASE, sdlpp::log_category::application);
        backend_->map_category(logger_categories::BUSINESS, sdlpp::log_category::application);
        backend_->map_category(logger_categories::SECURITY, sdlpp::log_category::error);

        // Set default category for unmapped categories
        backend_->set_default_category(sdlpp::log_category::application);

        // Configure failsafe to use our SDL++ backend
        failsafe::logger::set_backend(backend_->get_logger());

        // Demonstrate different log levels with failsafe
        demonstrate_logging();
    }

    void on_update([[maybe_unused]] float delta_time) override {
        // Periodically log from the main thread
        static auto last_log = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (now - last_log > std::chrono::seconds(5)) {
            last_log = now;

            // Use failsafe logging macros
            LOG_INFO(logger_categories::BUSINESS, "Frame rendered");
            LOG_DEBUG(logger_categories::NETWORK, "Network heartbeat");
        }
    }

    void on_render(sdlpp::renderer& r) override {
        // Clear screen with dark gray
        [[maybe_unused]] auto color_result = r.set_draw_color({32, 32, 32, 255});
        [[maybe_unused]] auto clear_result = r.clear();
        r.present();
    }

    void handle_event(const sdlpp::event& e) override {
        if (e.type() == sdlpp::event_type::key_down) {
            const auto& key_event = e.key();
            switch (key_event.key) {
                case SDLK_1:
                    LOG_TRACE(logger_categories::BUSINESS, "Key 1 pressed - TRACE level");
                    break;
                case SDLK_2:
                    LOG_DEBUG(logger_categories::BUSINESS, "Key 2 pressed - DEBUG level");
                    break;
                case SDLK_3:
                    LOG_INFO(logger_categories::BUSINESS, "Key 3 pressed - INFO level");
                    break;
                case SDLK_4:
                    LOG_WARN(logger_categories::BUSINESS, "Key 4 pressed - WARNING level");
                    break;
                case SDLK_5:
                    LOG_ERROR(logger_categories::BUSINESS, "Key 5 pressed - ERROR level");
                    break;
                case SDLK_6:
                    LOG_FATAL(logger_categories::SECURITY, "Key 6 pressed - FATAL level (maps to SDL CRITICAL)");
                    break;
                case SDLK_T:
                    spawn_logging_thread();
                    break;
                case SDLK_C:
                    // Change backend configuration at runtime
                    {
                        auto backend_config = backend_->get_config();
                        backend_config.show_thread_id = !backend_config.show_thread_id;
                        backend_->set_config(backend_config);
                        LOG_INFO(logger_categories::BUSINESS,
                                "Thread ID display ",
                                backend_config.show_thread_id ? "enabled" : "disabled");
                    }
                    break;
            }
        }
    }

private:
    void demonstrate_logging() {
        std::cout << "\n=== Failsafe Logger Integration Demo ===\n";
        std::cout << "Press keys 1-6 to log at different levels\n";
        std::cout << "Press 'T' to spawn a thread that logs\n";
        std::cout << "Press 'C' to toggle thread ID display\n";
        std::cout << "Press Escape to exit\n\n";

        // Log using failsafe macros
        LOG_INFO(logger_categories::BUSINESS, "Application starting up");
        LOG_DEBUG(logger_categories::NETWORK, "Network subsystem initialized");
        LOG_INFO(logger_categories::DATABASE, "Database connection established");

        // Log with multiple arguments (failsafe supports variadic logging)
        int user_count = 42;
        double load_time = 1.234;
        LOG_INFO(logger_categories::BUSINESS,
                "System loaded with ", user_count, " users in ", load_time, " seconds");

        // Log structured data
        struct User {
            int id;
            std::string name;
        };
        User user{123, "John Doe"};
        LOG_DEBUG(logger_categories::DATABASE,
                 "User query: id=", user.id, ", name='", user.name, "'");

        // Demonstrate different severity levels
        LOG_TRACE(logger_categories::NETWORK, "Detailed network packet trace");
        LOG_DEBUG(logger_categories::NETWORK, "Network debug information");
        LOG_INFO(logger_categories::NETWORK, "Network status update");
        LOG_WARN(logger_categories::NETWORK, "Network latency warning");
        LOG_ERROR(logger_categories::NETWORK, "Network connection error");
        LOG_FATAL(logger_categories::SECURITY, "Critical security issue!");
    }

    void spawn_logging_thread() {
        std::thread worker([this]() {
            LOG_INFO(logger_categories::BUSINESS, "Worker thread started");

            for (int i = 0; i < 5; ++i) {
                LOG_DEBUG(logger_categories::BUSINESS,
                         "Worker thread iteration ", i + 1, " of 5");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            LOG_INFO(logger_categories::BUSINESS, "Worker thread finished");
        });
        worker.detach();
    }

    std::unique_ptr<sdlpp::failsafe_backend> backend_;
};

// Entry point
SDLPP_MAIN(failsafe_logger_app)
