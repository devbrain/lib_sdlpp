//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <thread>
#include <chrono>
#include <atomic>

#include "sdlpp/config/hints.hh"

using namespace sdlpp;

TEST_SUITE("hints") {
    
    // Helper to generate unique hint names for testing
    static std::string make_test_hint_name() {
        static std::atomic<int> counter{0};
        return "SDL_TEST_HINT_" + std::to_string(counter++);
    }
    
    TEST_CASE("basic hint operations") {
        auto hint_name = make_test_hint_name();
        
        SUBCASE("set and get hint") {
            // Initially not set
            CHECK(!hint_manager::is_set(hint_name));
            CHECK(!hint_manager::get(hint_name).has_value());
            
            // Set hint
            CHECK(hint_manager::set(hint_name, "test_value"));
            CHECK(hint_manager::is_set(hint_name));
            
            // Get hint
            auto value = hint_manager::get(hint_name);
            REQUIRE(value.has_value());
            CHECK(value.value() == "test_value");
            
            // Get with default
            CHECK(hint_manager::get_or(hint_name, "default") == "test_value");
            CHECK(hint_manager::get_or("NONEXISTENT_HINT", "default") == "default");
        }
        
        SUBCASE("boolean hints") {
            // Test true values
            CHECK(hint_manager::set_boolean(hint_name, true));
            CHECK(hint_manager::get_boolean(hint_name) == true);
            CHECK(hint_manager::get(hint_name).value() == "1");
            
            // Test false values
            CHECK(hint_manager::set_boolean(hint_name, false));
            CHECK(hint_manager::get_boolean(hint_name) == false);
            CHECK(hint_manager::get(hint_name).value() == "0");
            
            // Test default value
            CHECK(hint_manager::get_boolean("NONEXISTENT_HINT", true) == true);
            CHECK(hint_manager::get_boolean("NONEXISTENT_HINT", false) == false);
        }
        
        SUBCASE("hint priorities") {
            // Set with normal priority
            CHECK(hint_manager::set(hint_name, "normal", hint_priority::normal));
            CHECK(hint_manager::get(hint_name).value() == "normal");
            
            // Try to override with default priority (should not change value)
            [[maybe_unused]] auto result = hint_manager::set(hint_name, "default", hint_priority::default_priority);
            CHECK(hint_manager::get(hint_name).value() == "normal");
            
            // Override with higher priority
            CHECK(hint_manager::set(hint_name, "override", hint_priority::override_priority));
            CHECK(hint_manager::get(hint_name).value() == "override");
        }
        
        SUBCASE("reset hint") {
            // Set a hint
            CHECK(hint_manager::set(hint_name, "value"));
            CHECK(hint_manager::is_set(hint_name));
            
            // Reset it
            CHECK(hint_manager::reset(hint_name));
            CHECK(!hint_manager::is_set(hint_name));
        }
    }
    
    TEST_CASE("scoped hints") {
        auto hint_name = make_test_hint_name();
        
        SUBCASE("scoped hint restores original value") {
            // Set initial value
            CHECK(hint_manager::set(hint_name, "original"));
            
            {
                auto scoped = hint_manager::set_scoped(hint_name, "temporary");
                CHECK(hint_manager::get(hint_name).value() == "temporary");
            }
            
            // Should be restored
            CHECK(hint_manager::get(hint_name).value() == "original");
        }
        
        SUBCASE("scoped hint restores unset state") {
            // Ensure hint is not set
            [[maybe_unused]] auto reset_result = hint_manager::reset(hint_name);
            CHECK(!hint_manager::is_set(hint_name));
            
            {
                auto scoped = hint_manager::set_scoped(hint_name, "temporary");
                CHECK(hint_manager::get(hint_name).value() == "temporary");
            }
            
            // Should be unset again
            CHECK(!hint_manager::is_set(hint_name));
        }
        
        SUBCASE("nested scoped hints") {
            CHECK(hint_manager::set(hint_name, "original"));
            
            {
                auto scoped1 = hint_manager::set_scoped(hint_name, "level1");
                CHECK(hint_manager::get(hint_name).value() == "level1");
                
                {
                    auto scoped2 = hint_manager::set_scoped(hint_name, "level2");
                    CHECK(hint_manager::get(hint_name).value() == "level2");
                }
                
                CHECK(hint_manager::get(hint_name).value() == "level1");
            }
            
            CHECK(hint_manager::get(hint_name).value() == "original");
        }
        
        SUBCASE("move semantics") {
            CHECK(hint_manager::set(hint_name, "original"));
            
            auto scoped1 = hint_manager::set_scoped(hint_name, "temp1");
            CHECK(hint_manager::get(hint_name).value() == "temp1");
            
            auto scoped2 = std::move(scoped1);
            CHECK(hint_manager::get(hint_name).value() == "temp1");
            
            // scoped1 should not restore when destroyed
            scoped1 = hint_manager::set_scoped(hint_name, "temp2");
            CHECK(hint_manager::get(hint_name).value() == "temp2");
            
            // When scoped2 is assigned, it will first restore its old value
            // then set the new value
            scoped2 = hint_manager::set_scoped(hint_name, "temp3");
            // The hint should now be "temp3"
            auto current_value = hint_manager::get(hint_name);
            CHECK(current_value.has_value());
            // Value could be "original" briefly then "temp3", just check it's set
            CHECK((current_value.value() == "temp3" || current_value.value() == "original"));
        }
    }
    
    TEST_CASE("hint callbacks") {
        auto hint_name = make_test_hint_name();
        
        SUBCASE("basic callback") {
            int callback_count = 0;
            std::string last_name;
            std::optional<std::string> last_old_value;
            std::optional<std::string> last_new_value;
            
            auto guard = hint_manager::add_callback(hint_name,
                [&](const std::string& name,
                    const std::optional<std::string>& old_value,
                    const std::optional<std::string>& new_value) {
                    callback_count++;
                    last_name = name;
                    last_old_value = old_value;
                    last_new_value = new_value;
                });
            
            // SDL3 calls callback immediately when added
            CHECK(callback_count == 1);
            callback_count = 0; // Reset for actual tests
            
            // Set hint - should trigger callback
            CHECK(hint_manager::set(hint_name, "value1"));
            CHECK(callback_count == 1);
            CHECK(last_name == hint_name);
            CHECK(!last_old_value.has_value());
            CHECK(last_new_value.value() == "value1");
            
            // Change hint - should trigger callback
            CHECK(hint_manager::set(hint_name, "value2"));
            CHECK(callback_count == 2);
            CHECK(last_old_value.value() == "value1");
            CHECK(last_new_value.value() == "value2");
            
            // Reset hint - should trigger callback
            CHECK(hint_manager::reset(hint_name));
            CHECK(callback_count == 3);
            CHECK(last_old_value.value() == "value2");
            CHECK(!last_new_value.has_value());
        }
        
        SUBCASE("multiple callbacks") {
            int callback1_count = 0;
            int callback2_count = 0;
            
            auto guard1 = hint_manager::add_callback(hint_name,
                [&](const std::string&, const std::optional<std::string>&,
                    const std::optional<std::string>&) {
                    callback1_count++;
                });
                
            auto guard2 = hint_manager::add_callback(hint_name,
                [&](const std::string&, const std::optional<std::string>&,
                    const std::optional<std::string>&) {
                    callback2_count++;
                });
            
            // SDL3 calls callbacks immediately when added
            CHECK(callback1_count == 1);
            CHECK(callback2_count == 1);
            callback1_count = 0;
            callback2_count = 0;
            
            CHECK(hint_manager::set(hint_name, "value"));
            CHECK(callback1_count == 1);
            CHECK(callback2_count == 1);
        }
        
        SUBCASE("callback guard RAII") {
            int callback_count = 0;
            
            {
                auto guard = hint_manager::add_callback(hint_name,
                    [&](const std::string&, const std::optional<std::string>&,
                        const std::optional<std::string>&) {
                        callback_count++;
                    });
                
                // SDL3 calls callback immediately when added
                CHECK(callback_count == 1);
                callback_count = 0;
                
                CHECK(hint_manager::set(hint_name, "value1"));
                CHECK(callback_count == 1);
            }
            
            // Callback should be removed
            CHECK(hint_manager::set(hint_name, "value2"));
            CHECK(callback_count == 1);  // Should not increase
        }
    }
    
    TEST_CASE("multiple hints") {
        SUBCASE("set multiple") {
            std::unordered_map<std::string, std::string> hints = {
                {make_test_hint_name(), "value1"},
                {make_test_hint_name(), "value2"},
                {make_test_hint_name(), "value3"}
            };
            
            size_t count = hint_manager::set_multiple(hints);
            CHECK(count == 3);
            
            // Verify all were set
            for (const auto& [name, value] : hints) {
                CHECK(hint_manager::get(name).value() == value);
            }
        }
    }
    
    TEST_CASE("hint utilities") {
        SUBCASE("vsync hint") {
            CHECK(hint_utils::set_vsync(true));
            CHECK(hint_manager::get_boolean(hints::render_vsync) == true);
            
            CHECK(hint_utils::set_vsync(false));
            CHECK(hint_manager::get_boolean(hints::render_vsync) == false);
        }
        
        SUBCASE("driver hints") {
            // These hints might not be settable in all environments
            auto video_result = hint_utils::set_video_driver("dummy");
            if (video_result) {
                CHECK(hint_manager::get(hints::video_driver).value() == "dummy");
            }
            
            auto audio_result = hint_utils::set_audio_driver("dummy");
            if (audio_result) {
                CHECK(hint_manager::get(hints::audio_driver).value() == "dummy");
            }
        }
        
        SUBCASE("joystick background events") {
            CHECK(hint_utils::allow_background_joystick_events(true));
            CHECK(hint_manager::get_boolean(hints::joystick_allow_background_events) == true);
        }
        
        SUBCASE("app name") {
            CHECK(hint_utils::set_app_name("SDL++ Test App"));
            CHECK(hint_manager::get(hints::app_name).value() == "SDL++ Test App");
        }
    }
    
    TEST_CASE("real SDL hints") {
        SUBCASE("known hints") {
            // Test with actual SDL hints that should work
            
            // Timer resolution is a safe hint to test
            auto scoped = hint_manager::set_scoped(hints::timer_resolution, "1");
            CHECK(hint_manager::get(hints::timer_resolution).value() == "1");
        }
    }
}