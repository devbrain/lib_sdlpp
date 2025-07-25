//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <vector>
#include <string>
#include <sstream>
#include <regex>

#include "../include/sdlpp/core/log.hh"

using namespace sdlpp;

// Test helper to capture log output
struct log_capture {
    struct log_entry {
        int category;
        log_priority priority;
        std::string message;
    };
    
    std::vector<log_entry> entries;
    
    log_capture() {
        log_config::set_output_function([this](int category, log_priority priority, const std::string& message) {
            entries.push_back({category, priority, message});
        });
    }
    
    ~log_capture() {
        log_config::set_output_function(nullptr);
    }
    
    void clear() {
        entries.clear();
    }
    
    bool has_message_containing(const std::string& text) const {
        return std::any_of(entries.begin(), entries.end(),
            [&text](const log_entry& entry) {
                return entry.message.find(text) != std::string::npos;
            });
    }
    
    bool has_message_matching(const std::string& pattern) const {
        std::regex re(pattern);
        return std::any_of(entries.begin(), entries.end(),
            [&re](const log_entry& entry) {
                return std::regex_search(entry.message, re);
            });
    }
};

TEST_SUITE("log") {
    
    TEST_CASE("basic logging") {
        log_capture capture;
        
        SUBCASE("simple message") {
            SDLPP_LOG_APP("Hello, World!");
            REQUIRE(capture.entries.size() == 1);
            CHECK(capture.entries[0].category == static_cast<int>(log_category::application));
            CHECK(capture.entries[0].priority == log_priority::info);
            CHECK(capture.has_message_containing("Hello, World!"));
        }
        
        SUBCASE("multiple arguments") {
            SDLPP_LOG_APP("Value:", 42, "Status:", true);
            REQUIRE(capture.entries.size() == 1);
            CHECK(capture.has_message_containing("Value: 42 Status: true"));
        }
        
        SUBCASE("different types") {
            double pi = 3.14159;
            const char* str = "test";
            void* ptr = nullptr;
            
            SDLPP_LOG_APP("Pi:", pi, "String:", str, "Pointer:", ptr);
            REQUIRE(capture.entries.size() == 1);
            CHECK(capture.has_message_containing("Pi: 3.14159"));
            CHECK(capture.has_message_containing("String: test"));
            CHECK(capture.has_message_containing("Pointer: nullptr"));
        }
    }
    
    TEST_CASE("priority levels") {
        log_capture capture;
        
        // Set all priorities to trace to capture everything
        log_config::set_all_priorities(log_priority::trace);
        
        SUBCASE("all priority levels") {
            SDLPP_LOG_TRACE(log_category::application, "Trace message");
            SDLPP_LOG_VERBOSE(log_category::application, "Verbose message");
            SDLPP_LOG_DEBUG(log_category::application, "Debug message");
            SDLPP_LOG_INFO(log_category::application, "Info message");
            SDLPP_LOG_WARN(log_category::application, "Warn message");
            SDLPP_LOG_ERROR(log_category::application, "Error message");
            SDLPP_LOG_CRITICAL(log_category::application, "Critical message");
            
            REQUIRE(capture.entries.size() == 7);
            
            CHECK(capture.entries[0].priority == log_priority::trace);
            CHECK(capture.entries[1].priority == log_priority::verbose);
            CHECK(capture.entries[2].priority == log_priority::debug);
            CHECK(capture.entries[3].priority == log_priority::info);
            CHECK(capture.entries[4].priority == log_priority::warn);
            CHECK(capture.entries[5].priority == log_priority::error);
            CHECK(capture.entries[6].priority == log_priority::critical);
        }
        
        SUBCASE("priority filtering") {
            // Set to only show warnings and above
            log_config::set_priority(log_category::application, log_priority::warn);
            
            SDLPP_LOG_DEBUG(log_category::application, "Debug - should not appear");
            SDLPP_LOG_INFO(log_category::application, "Info - should not appear");
            SDLPP_LOG_WARN(log_category::application, "Warning - should appear");
            SDLPP_LOG_ERROR(log_category::application, "Error - should appear");
            
            REQUIRE(capture.entries.size() == 2);
            CHECK(capture.has_message_containing("Warning - should appear"));
            CHECK(capture.has_message_containing("Error - should appear"));
        }
        
        // Reset priorities
        log_config::reset_priorities();
    }
    
    TEST_CASE("categories") {
        log_capture capture;
        log_config::set_all_priorities(log_priority::trace);
        
        SUBCASE("different categories") {
            SDLPP_LOG_INFO(log_category::application, "App message");
            SDLPP_LOG_INFO(log_category::audio, "Audio message");
            SDLPP_LOG_INFO(log_category::video, "Video message");
            SDLPP_LOG_INFO(log_category::render, "Render message");
            
            REQUIRE(capture.entries.size() == 4);
            CHECK(capture.entries[0].category == static_cast<int>(log_category::application));
            CHECK(capture.entries[1].category == static_cast<int>(log_category::audio));
            CHECK(capture.entries[2].category == static_cast<int>(log_category::video));
            CHECK(capture.entries[3].category == static_cast<int>(log_category::render));
        }
        
        SUBCASE("custom category") {
            const int CUSTOM_CATEGORY = static_cast<int>(log_category::custom) + 1;
            SDLPP_LOG_INFO(CUSTOM_CATEGORY, "Custom category message");
            
            REQUIRE(capture.entries.size() == 1);
            CHECK(capture.entries[0].category == CUSTOM_CATEGORY);
        }
        
        log_config::reset_priorities();
    }
    
    TEST_CASE("source location") {
        log_capture capture;
        
        SUBCASE("location included in message") {
            SDLPP_LOG_APP("Test message");
            
            REQUIRE(capture.entries.size() == 1);
            // Check that the message contains file info
            CHECK(capture.has_message_containing("test_log.cc"));
            // The location format may have spaces in function names
            CHECK(capture.has_message_matching(R"(\[.+:\d+ .+\])"));  // [file:line function]
        }
        
        SUBCASE("manual location") {
            auto loc = std::source_location::current();
            logger::app_info(loc, "Manual location");
            
            REQUIRE(capture.entries.size() == 1);
            CHECK(capture.has_message_containing(std::to_string(loc.line())));
        }
    }
    
    TEST_CASE("log configuration") {
        SUBCASE("priority management") {
            // Set and get priority
            log_config::set_priority(log_category::audio, log_priority::debug);
            CHECK(log_config::get_priority(log_category::audio) == log_priority::debug);
            
            // Set all priorities
            log_config::set_all_priorities(log_priority::error);
            CHECK(log_config::get_priority(log_category::application) == log_priority::error);
            CHECK(log_config::get_priority(log_category::video) == log_priority::error);
            
            // Reset
            log_config::reset_priorities();
        }
        
        SUBCASE("priority prefix") {
            log_capture capture;
            
            // Set custom prefix for warnings
            CHECK(log_config::set_priority_prefix(log_priority::warn, "[ALERT] "));
            
            SDLPP_LOG_WARN(log_category::application, "Warning message");
            
            REQUIRE(capture.entries.size() == 1);
            // SDL internally prepends the prefix, but we're using custom output
            
            // Reset prefix
            CHECK(log_config::set_priority_prefix(log_priority::warn, ""));
        }
    }
    
    TEST_CASE("custom output function") {
        SUBCASE("temporary custom output") {
            std::vector<std::string> captured;
            
            {
                auto guard = log_config::scoped_output_function(
                    [&captured]([[maybe_unused]] int category, [[maybe_unused]] log_priority priority, const std::string& message) {
                        captured.push_back(message);
                    });
                
                SDLPP_LOG_APP("Message 1");
                SDLPP_LOG_APP("Message 2");
            }
            
            // After guard destruction, output should be restored
            SDLPP_LOG_APP("Message 3"); // Should not be captured
            
            REQUIRE(captured.size() == 2);
            CHECK(captured[0].find("Message 1") != std::string::npos);
            CHECK(captured[1].find("Message 2") != std::string::npos);
        }
    }
    
    TEST_CASE("edge cases") {
        log_capture capture;
        
        SUBCASE("empty message") {
            SDLPP_LOG_APP();
            REQUIRE(capture.entries.size() == 1);
            // Should still have location info
            CHECK(capture.has_message_containing("test_log.cc"));
        }
        
        SUBCASE("special characters") {
            SDLPP_LOG_APP("Special: \t\n\r", "Quoted: \"text\"", "Percent: %d %s");
            REQUIRE(capture.entries.size() == 1);
            CHECK(capture.has_message_containing("Special: \t\n\r"));
            CHECK(capture.has_message_containing("Quoted: \"text\""));
            CHECK(capture.has_message_containing("Percent: %d %s"));
        }
        
        SUBCASE("very long message") {
            std::string long_str(1000, 'x');
            SDLPP_LOG_APP("Long:", long_str);
            
            REQUIRE(capture.entries.size() == 1);
            CHECK(capture.has_message_containing(long_str));
        }
    }
    
    TEST_CASE("convenience methods") {
        log_capture capture;
        log_config::set_all_priorities(log_priority::trace);
        
        SUBCASE("app shortcuts") {
            SDLPP_LOG_APP("Info message");
            SDLPP_LOG_APP_DEBUG("Debug message");
            SDLPP_LOG_APP_WARN("Warning message");
            SDLPP_LOG_APP_ERROR("Error message");
            
            REQUIRE(capture.entries.size() == 4);
            
            // All should be application category
            for (const auto& entry : capture.entries) {
                CHECK(entry.category == static_cast<int>(log_category::application));
            }
            
            CHECK(capture.entries[0].priority == log_priority::info);
            CHECK(capture.entries[1].priority == log_priority::debug);
            CHECK(capture.entries[2].priority == log_priority::warn);
            CHECK(capture.entries[3].priority == log_priority::error);
        }
        
        log_config::reset_priorities();
    }
    
    TEST_CASE("stream-like behavior") {
        log_capture capture;
        
        SUBCASE("simple types") {
            int x = 10, y = 20;
            SDLPP_LOG_APP("Point: (", x, ",", y, ") Distance:", 15.5);
            
            REQUIRE(capture.entries.size() == 1);
            CHECK(capture.has_message_containing("Point: ( 10 , 20 ) Distance: 15.5"));
        }
    }
}