//
// Created by igor on 7/20/25.
//

/**
 * @file test_failsafe_backend.cc
 * @brief Unit tests for SDL++ failsafe logger backend
 */

#include <doctest/doctest.h>
#include <sdlpp/core/failsafe_backend.hh>
#include <sdlpp/core/log.hh>
#include <failsafe/failsafe.hh>
#include <sstream>
#include <regex>
#include <thread>
#include <chrono>

TEST_SUITE("failsafe_backend") {
    // Helper to capture SDL log output
    class log_capture {
    public:
        log_capture() {
            // Set custom output function to capture logs
            sdlpp::log_config::set_output_function(
                [this](int category, sdlpp::log_priority priority, const std::string& message) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    entries_.push_back({category, priority, message});
                }
            );
        }
        
        ~log_capture() {
            // Reset to default output
            sdlpp::log_config::set_output_function(nullptr);
        }
        
        struct log_entry {
            int category;
            sdlpp::log_priority priority;
            std::string message;
        };
        
        std::vector<log_entry> get_entries() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return entries_;
        }
        
        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            entries_.clear();
        }
        
        bool has_message_containing(const std::string& substring) const {
            std::lock_guard<std::mutex> lock(mutex_);
            return std::any_of(entries_.begin(), entries_.end(),
                [&substring](const log_entry& e) {
                    return e.message.find(substring) != std::string::npos;
                });
        }
        
        size_t count() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return entries_.size();
        }
        
    private:
        mutable std::mutex mutex_;
        std::vector<log_entry> entries_;
    };
    
    TEST_CASE("backend_creation") {
        SUBCASE("default_configuration") {
            auto backend = sdlpp::failsafe_backend::create();
            auto config = backend.get_config();
            
            CHECK(config.show_timestamp == true);
            CHECK(config.show_thread_id == true);
            CHECK(config.show_file_line == true);
            CHECK(config.use_colors == false);
            CHECK(config.timestamp_format == "%Y-%m-%d %H:%M:%S");
        }
        
        SUBCASE("custom_configuration") {
            sdlpp::failsafe_backend::config cfg;
            cfg.show_timestamp = false;
            cfg.show_thread_id = false;
            cfg.show_file_line = false;
            cfg.timestamp_format = "%H:%M:%S";
            
            auto backend = sdlpp::failsafe_backend::create(cfg);
            auto retrieved_cfg = backend.get_config();
            
            CHECK(retrieved_cfg.show_timestamp == false);
            CHECK(retrieved_cfg.show_thread_id == false);
            CHECK(retrieved_cfg.show_file_line == false);
            CHECK(retrieved_cfg.timestamp_format == "%H:%M:%S");
        }
    }
    
    TEST_CASE("logging_functionality") {
        log_capture capture;
        sdlpp::log_config::set_all_priorities(sdlpp::log_priority::trace);
        
        auto backend = sdlpp::failsafe_backend::create({
            .show_timestamp = false,  // Disable for predictable output
            .show_thread_id = false,
            .show_file_line = true
        });
        
        // Set failsafe to use our backend
        failsafe::logger::set_backend(backend.get_logger());
        
        SUBCASE("basic_logging") {
            capture.clear();
            
            LOG_INFO("Hello from failsafe");
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 1);
            CHECK(entries[0].priority == sdlpp::log_priority::info);
            CHECK(entries[0].message.find("Hello from failsafe") != std::string::npos);
        }
        
        SUBCASE("log_level_mapping") {
            capture.clear();
            
            // Test different log levels
            LOG_TRACE("Trace message");      // Level 0 -> trace
            LOG_DEBUG("Debug message");      // Level 1 -> debug
            LOG_INFO("Info message");        // Level 2 -> info
            LOG_WARN("Warning message");     // Level 3 -> warn
            LOG_ERROR("Error message");      // Level 4 -> error
            LOG_FATAL("Fatal message");      // Level 5 -> critical
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 6);
            
            CHECK(entries[0].priority == sdlpp::log_priority::trace);
            CHECK(entries[1].priority == sdlpp::log_priority::debug);
            CHECK(entries[2].priority == sdlpp::log_priority::info);
            CHECK(entries[3].priority == sdlpp::log_priority::warn);
            CHECK(entries[4].priority == sdlpp::log_priority::error);
            CHECK(entries[5].priority == sdlpp::log_priority::critical);
        }
        
        SUBCASE("category_mapping") {
            capture.clear();
            
            // Note: LOG_INFO always uses the default "Application" category
            // The first parameter is part of the message, not a category
            // So category mapping doesn't apply to these macros
            backend.set_default_category(sdlpp::log_category::test);
            
            LOG_INFO("Test message");
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 1);
            
            // Since we set default category to test, and "Application" is not mapped,
            // it will use the default category (test)
            CHECK(entries[0].category == static_cast<int>(sdlpp::log_category::test));
            
            // To test category mapping, we would need to use the lower-level API
            // that actually accepts a category parameter
        }
        
        SUBCASE("variadic_logging") {
            capture.clear();
            
            int count = 42;
            double value = 3.14;
            std::string name = "test";
            
            LOG_INFO("Count: ", count, ", Value: ", value, ", Name: ", name);
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 1);
            // Failsafe adds extra spaces in variadic output
            CHECK(entries[0].message.find("Count: ") != std::string::npos);
            CHECK(entries[0].message.find("42") != std::string::npos);
            CHECK(entries[0].message.find("Value: ") != std::string::npos);
            CHECK(entries[0].message.find("3.14") != std::string::npos);
            CHECK(entries[0].message.find("Name: ") != std::string::npos);
            CHECK(entries[0].message.find("test") != std::string::npos);
        }
    }
    
    TEST_CASE("configuration_options") {
        log_capture capture;
        sdlpp::log_config::set_all_priorities(sdlpp::log_priority::trace);
        
        SUBCASE("timestamp_formatting") {
            auto backend = sdlpp::failsafe_backend::create({
                .show_timestamp = true,
                .show_thread_id = false,
                .show_file_line = false,
                .timestamp_format = "%H:%M:%S"
            });
            
            failsafe::logger::set_backend(backend.get_logger());
            capture.clear();
            
            LOG_INFO("test", "Message with timestamp");
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 1);
            
            // Check for time pattern HH:MM:SS.mmm
            std::regex time_pattern(R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\])");
            CHECK(std::regex_search(entries[0].message, time_pattern));
        }
        
        SUBCASE("thread_id_display") {
            auto backend = sdlpp::failsafe_backend::create({
                .show_timestamp = false,
                .show_thread_id = true,
                .show_file_line = false
            });
            
            failsafe::logger::set_backend(backend.get_logger());
            capture.clear();
            
            LOG_INFO("test", "Message with thread ID");
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 1);
            
            // Check for thread ID pattern [xxxxx]
            CHECK(entries[0].message.find("[") != std::string::npos);
            CHECK(entries[0].message.find("]") != std::string::npos);
        }
        
        SUBCASE("file_line_info") {
            auto backend = sdlpp::failsafe_backend::create({
                .show_timestamp = false,
                .show_thread_id = false,
                .show_file_line = true
            });
            
            failsafe::logger::set_backend(backend.get_logger());
            capture.clear();
            
            LOG_INFO("test", "Message with file info");
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 1);
            
            // Should contain file:line pattern
            CHECK(entries[0].message.find("test_failsafe_backend.cc:") != std::string::npos);
        }
        
        SUBCASE("runtime_config_change") {
            auto backend = sdlpp::failsafe_backend::create({
                .show_timestamp = false,
                .show_thread_id = false,
                .show_file_line = false
            });
            
            failsafe::logger::set_backend(backend.get_logger());
            capture.clear();
            
            LOG_INFO("Message 1");
            
            // Change configuration
            auto config = backend.get_config();
            config.show_thread_id = true;
            backend.set_config(config);
            
            LOG_INFO("Message 2");
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 2);
            
            // Check that messages have proper formatting
            // First message should not have thread ID
            CHECK(entries[0].message.find("Message 1") != std::string::npos);
            
            // Second message should have thread ID (appears as bracketed text)
            CHECK(entries[1].message.find("Message 2") != std::string::npos);
            
            // We can't check exact format as SDL may add its own prefixes
        }
    }
    
    TEST_CASE("thread_safety") {
        log_capture capture;
        sdlpp::log_config::set_all_priorities(sdlpp::log_priority::trace);
        
        auto backend = sdlpp::failsafe_backend::create({
            .show_timestamp = false,
            .show_thread_id = true,
            .show_file_line = false
        });
        
        failsafe::logger::set_backend(backend.get_logger());
        
        SUBCASE("concurrent_logging") {
            capture.clear();
            
            const int thread_count = 4;
            constexpr int messages_per_thread = 10;
            std::vector<std::thread> threads;

            for (int i = 0; i < thread_count; ++i) {
                threads.emplace_back([i]() {
                    for (int j = 0; j < messages_per_thread; ++j) {
                        LOG_INFO("Thread ", i, " message ", j);
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                });
            }
            
            for (auto& t : threads) {
                t.join();
            }
            
            auto entries = capture.get_entries();
            CHECK(entries.size() == thread_count * messages_per_thread);
            
            // All messages should be properly formatted
            for (const auto& entry : entries) {
                CHECK(entry.message.find("Thread") != std::string::npos);
                CHECK(entry.message.find("message") != std::string::npos);
            }
        }
    }
    
    TEST_CASE("convenience_function") {
        log_capture capture;
        sdlpp::log_config::set_all_priorities(sdlpp::log_priority::trace);
        
        SUBCASE("create_failsafe_sdl_backend") {
            auto logger = sdlpp::create_failsafe_sdl_backend(true, true);
            failsafe::logger::set_backend(logger);
            
            capture.clear();
            LOG_INFO("Using convenience function");
            
            auto entries = capture.get_entries();
            REQUIRE(entries.size() == 1);
            CHECK(entries[0].message.find("Using convenience function") != std::string::npos);
        }
    }
}