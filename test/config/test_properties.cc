//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <thread>
#include <sstream>
#include <cmath>

#include "sdlpp/config/properties.hh"

using namespace sdlpp;

TEST_SUITE("properties") {
    
    TEST_CASE("basic property operations") {
        auto props_result = properties::create();
        REQUIRE(props_result.has_value());
        auto& props = *props_result;
        
        SUBCASE("string properties") {
            CHECK(props.set_string("name", "Test Name"));
            CHECK(props.has("name"));
            CHECK(props.get_string("name") == "Test Name");
            CHECK(props.get_string("nonexistent", "default") == "default");
            
            // Overwrite
            CHECK(props.set_string("name", "New Name"));
            CHECK(props.get_string("name") == "New Name");
        }
        
        SUBCASE("number properties") {
            CHECK(props.set_number("score", 12345));
            CHECK(props.get_number("score") == 12345);
            CHECK(props.get_number("nonexistent", 999) == 999);
            
            // Negative numbers
            CHECK(props.set_number("delta", -100));
            CHECK(props.get_number("delta") == -100);
            
            // Large numbers
            CHECK(props.set_number("large", INT64_MAX));
            CHECK(props.get_number("large") == INT64_MAX);
        }
        
        SUBCASE("float properties") {
            CHECK(props.set_float("pi", 3.14159f));
            CHECK(props.get_float("pi") == doctest::Approx(3.14159f));
            CHECK(props.get_float("nonexistent", 1.0f) == 1.0f);
            
            // Special values
            CHECK(props.set_float("inf", INFINITY));
            CHECK(std::isinf(props.get_float("inf")));
        }
        
        SUBCASE("boolean properties") {
            CHECK(props.set_boolean("enabled", true));
            CHECK(props.get_boolean("enabled") == true);
            CHECK(props.get_boolean("nonexistent", true) == true);
            
            CHECK(props.set_boolean("enabled", false));
            CHECK(props.get_boolean("enabled") == false);
        }
        
        SUBCASE("pointer properties") {
            int value = 42;
            CHECK(props.set_pointer("ptr", &value));
            CHECK(props.get_pointer("ptr") == &value);
            CHECK(props.get_pointer("nonexistent") == nullptr);
            
            // Null pointer
            CHECK(props.set_pointer("null", nullptr));
            CHECK(props.get_pointer("null") == nullptr);
        }
        
        SUBCASE("generic set method") {
            CHECK(props.set("str", std::string("Hello")));
            CHECK(props.get_string("str") == "Hello");
            
            CHECK(props.set("num", 42));
            CHECK(props.get_number("num") == 42);
            
            CHECK(props.set("float", 3.14));
            CHECK(props.get_float("float") == doctest::Approx(3.14f));
            
            CHECK(props.set("bool", true));
            CHECK(props.get_boolean("bool") == true);
            
            CHECK(props.set("cstr", std::string("C String")));
            CHECK(props.get_string("cstr") == "C String");
        }
    }
    
    TEST_CASE("property lifecycle") {
        auto props_result = properties::create();
        REQUIRE(props_result.has_value());
        auto& props = *props_result;
        
        SUBCASE("clear property") {
            CHECK(props.set_string("temp", "value"));
            CHECK(props.has("temp"));
            
            CHECK(props.clear("temp"));
            CHECK(!props.has("temp"));
            CHECK(props.get_string("temp") == "");
        }
        
        SUBCASE("overwrite different types") {
            // Set as string
            CHECK(props.set_string("multi", "text"));
            CHECK(props.get_string("multi") == "text");
            
            // Overwrite as number
            CHECK(props.set_number("multi", 123));
            CHECK(props.get_number("multi") == 123);
            
            // Overwrite as boolean
            CHECK(props.set_boolean("multi", true));
            CHECK(props.get_boolean("multi") == true);
        }
        
        SUBCASE("property type detection") {
            CHECK(props.set_string("str_prop", "value"));
            CHECK(props.get_type("str_prop") == SDL_PROPERTY_TYPE_STRING);
            
            CHECK(props.set_number("num_prop", 42));
            CHECK(props.get_type("num_prop") == SDL_PROPERTY_TYPE_NUMBER);
            
            CHECK(props.set_float("float_prop", 3.14f));
            CHECK(props.get_type("float_prop") == SDL_PROPERTY_TYPE_FLOAT);
            
            CHECK(props.set_boolean("bool_prop", true));
            CHECK(props.get_type("bool_prop") == SDL_PROPERTY_TYPE_BOOLEAN);
            
            int value = 0;
            CHECK(props.set_pointer("ptr_prop", &value));
            CHECK(props.get_type("ptr_prop") == SDL_PROPERTY_TYPE_POINTER);
            
            CHECK(props.get_type("nonexistent") == SDL_PROPERTY_TYPE_INVALID);
        }
    }
    
    TEST_CASE("pointer with cleanup") {
        auto props_result = properties::create();
        REQUIRE(props_result.has_value());
        auto& props = *props_result;
        
        bool cleanup_called = false;
        void* cleanup_value = nullptr;
        void* cleanup_userdata = nullptr;
        
        auto* data = new int(42);
        
        CHECK(props.set_pointer_with_cleanup("managed", data,
            [&cleanup_called, &cleanup_value, &cleanup_userdata](void* userdata, void* value) {
                cleanup_called = true;
                cleanup_value = value;
                cleanup_userdata = userdata;
                delete static_cast<int*>(value);
            }, &props));
        
        CHECK(props.get_pointer("managed") == data);
        
        // Clear should trigger cleanup
        CHECK(props.clear("managed"));
        CHECK(cleanup_called);
        CHECK(cleanup_value == data);
        CHECK(cleanup_userdata == &props);
    }
    
    TEST_CASE("property enumeration") {
        auto props_result = properties::create();
        REQUIRE(props_result.has_value());
        auto& props = *props_result;
        
        // Add some properties
        CHECK(props.set_string("prop1", "value1"));
        CHECK(props.set_number("prop2", 42));
        CHECK(props.set_float("prop3", 3.14f));
        
        SUBCASE("enumerate with callback") {
            std::vector<std::string> names;
            CHECK(props.enumerate([&names](const std::string& name) {
                names.push_back(name);
            }));
            
            CHECK(names.size() == 3);
            CHECK(std::find(names.begin(), names.end(), "prop1") != names.end());
            CHECK(std::find(names.begin(), names.end(), "prop2") != names.end());
            CHECK(std::find(names.begin(), names.end(), "prop3") != names.end());
        }
        
        SUBCASE("get all names") {
            auto names = props.get_names();
            CHECK(names.size() == 3);
            CHECK(std::find(names.begin(), names.end(), "prop1") != names.end());
            CHECK(std::find(names.begin(), names.end(), "prop2") != names.end());
            CHECK(std::find(names.begin(), names.end(), "prop3") != names.end());
        }
    }
    
    TEST_CASE("thread safety") {
        auto props_result = properties::create();
        REQUIRE(props_result.has_value());
        auto& props = *props_result;
        
        SUBCASE("manual lock/unlock") {
            CHECK(props.lock());
            
            // Perform operations while locked
            CHECK(props.set_number("counter", 0));
            
            props.unlock();
        }
        
        SUBCASE("RAII lock guard") {
            {
                properties::lock_guard lock(props);
                CHECK(lock.is_locked());
                
                // Operations within lock
                CHECK(props.set_string("protected", "value"));
            }
            // Automatically unlocked
            
            CHECK(props.get_string("protected") == "value");
        }
        
        SUBCASE("concurrent access") {
            const int num_threads = 4;
            constexpr int iterations = 100;

            CHECK(props.set_number("shared_counter", 0));

            std::vector<std::thread> threads;
            for (int i = 0; i < num_threads; ++i) {
                threads.emplace_back([&props]() {
                    for (int j = 0; j < iterations; ++j) {
                        properties::lock_guard lock(props);
                        auto current = props.get_number("shared_counter");
                        [[maybe_unused]] auto result = props.set_number("shared_counter", current + 1);
                    }
                });
            }
            
            for (auto& t : threads) {
                t.join();
            }
            
            CHECK(props.get_number("shared_counter") == num_threads * iterations);
        }
    }
    
    TEST_CASE("property builder") {
        SUBCASE("build with various types") {
            auto props_result = property_builder()
                .add("name", "Player One")
                .add("level", 50)
                .add("health", 100.0f)
                .add("alive", true)
                .add("team", "red")
                .build();
            
            REQUIRE(props_result.has_value());
            auto& props = *props_result;
            
            CHECK(props.get_string("name") == "Player One");
            CHECK(props.get_number("level") == 50);
            CHECK(props.get_float("health") == 100.0f);
            CHECK(props.get_boolean("alive") == true);
            CHECK(props.get_string("team") == "red");
        }
        
        SUBCASE("empty builder") {
            auto props_result = property_builder().build();
            REQUIRE(props_result.has_value());
            CHECK(props_result->get_names().empty());
        }
    }
    
    TEST_CASE("property accessor") {
        auto props_result = properties::create();
        REQUIRE(props_result.has_value());
        auto& props = *props_result;
        
        SUBCASE("string accessor") {
            property_accessor<std::string> name(props, "name", "Unknown");
            
            CHECK(!name.exists());
            CHECK(static_cast<std::string>(name) == "Unknown");
            
            name = "Test User";
            CHECK(name.exists());
            CHECK(static_cast<std::string>(name) == "Test User");
            
            CHECK(name.clear());
            CHECK(!name.exists());
        }
        
        SUBCASE("numeric accessor") {
            property_accessor<int> score(props, "score", 0);
            
            score = 1000;
            CHECK(static_cast<int>(score) == 1000);
            
            // Increment
            score = score + 500;
            CHECK(static_cast<int>(score) == 1500);
        }
        
        SUBCASE("boolean accessor") {
            property_accessor<bool> enabled(props, "enabled", false);
            
            CHECK(static_cast<bool>(enabled) == false);
            
            enabled = true;
            CHECK(static_cast<bool>(enabled) == true);
        }
        
        SUBCASE("float accessor") {
            property_accessor<float> progress(props, "progress", 0.0f);
            
            progress = 0.75f;
            CHECK(static_cast<float>(progress) == doctest::Approx(0.75f));
        }
    }
    
    TEST_CASE("global properties") {
        auto global = properties::get_global();
        
        // Global properties should be valid
        CHECK(global.is_valid());
        
        // Should be able to set and get
        CHECK(global.set_string("test_global", "value"));
        CHECK(global.get_string("test_global") == "value");
        
        // Clean up
        CHECK(global.clear("test_global"));
    }
    
    TEST_CASE("move semantics") {
        auto props1_result = properties::create();
        REQUIRE(props1_result.has_value());
        auto props1 = std::move(*props1_result);
        
        CHECK(props1.set_string("data", "value"));
        
        // Move construction
        properties props2(std::move(props1));
        CHECK(!props1.is_valid());
        CHECK(props2.is_valid());
        CHECK(props2.get_string("data") == "value");
        
        // Move assignment
        auto props3_result = properties::create();
        REQUIRE(props3_result.has_value());
        *props3_result = std::move(props2);
        CHECK(!props2.is_valid());
        CHECK(props3_result->is_valid());
        CHECK(props3_result->get_string("data") == "value");
    }
    
    TEST_CASE("edge cases") {
        auto props_result = properties::create();
        REQUIRE(props_result.has_value());
        auto& props = *props_result;
        
        SUBCASE("empty property names") {
            // SDL3 might not allow empty property names
            auto result = props.set_string("", "empty name");
            if (!result) {
                // Expected behavior - empty names not allowed
                CHECK(!props.has(""));
                CHECK(props.get_string("") == "");
            } else {
                CHECK(props.has(""));
                CHECK(props.get_string("") == "empty name");
            }
        }
        
        SUBCASE("very long property names") {
            std::string long_name(1000, 'x');
            CHECK(props.set_number(long_name, 42));
            CHECK(props.get_number(long_name) == 42);
        }
        
        SUBCASE("special characters in names") {
            CHECK(props.set_string("prop.with.dots", "dots"));
            CHECK(props.get_string("prop.with.dots") == "dots");
            
            CHECK(props.set_string("prop/with/slashes", "slashes"));
            CHECK(props.get_string("prop/with/slashes") == "slashes");
            
            CHECK(props.set_string("prop with spaces", "spaces"));
            CHECK(props.get_string("prop with spaces") == "spaces");
        }
    }
}