#include <doctest/doctest.h>
#include <sdlpp/system/platform.hh>

TEST_SUITE("platform") {
    TEST_CASE("platform detection") {
        SUBCASE("get_platform returns non-empty string") {
            auto platform = sdlpp::platform::get_platform();
            CHECK_FALSE(platform.empty());
            
            // Platform should be one of the known platforms
            bool is_known = platform == "Windows" || 
                           platform == "Mac OS X" || 
                           platform == "Linux" || 
                           platform == "iOS" || 
                           platform == "Android" ||
                           platform.find("BSD") != std::string_view::npos ||
                           platform == "Emscripten";
            CHECK(is_known);
        }
        
        SUBCASE("platform checks are mutually exclusive for major platforms") {
            int platform_count = 0;
            if (sdlpp::platform::is_windows()) platform_count++;
            if (sdlpp::platform::is_macos()) platform_count++;
            if (sdlpp::platform::is_linux() && !sdlpp::platform::is_android()) platform_count++;
            if (sdlpp::platform::is_ios()) platform_count++;
            if (sdlpp::platform::is_android()) platform_count++;
            
            // Should be on exactly one major platform
            CHECK(platform_count == 1);
        }
        
        SUBCASE("apple platform check") {
            bool is_apple = sdlpp::platform::is_apple();
            bool is_any_apple = sdlpp::platform::is_macos() || 
                               sdlpp::platform::is_ios() || 
                               sdlpp::platform::is_tvos();
            
            // If on any Apple platform, is_apple should be true
            if (is_any_apple) {
                CHECK(is_apple);
            }
        }
        
        SUBCASE("unix platform check") {
            bool is_unix = sdlpp::platform::is_unix();
            
            // Linux, macOS, and BSD are Unix-like
            if (sdlpp::platform::is_linux() || 
                sdlpp::platform::is_macos() || 
                sdlpp::platform::is_bsd()) {
                CHECK(is_unix);
            }
        }
    }
    
    TEST_CASE("platform_info structure") {
        auto info = sdlpp::platform::get_platform_info();
        
        SUBCASE("basic fields") {
            CHECK_FALSE(info.name.empty());
            CHECK(info.category != sdlpp::platform::platform_category::unknown);
            
            // 64-bit check should match pointer size
            CHECK(info.is_64bit == (sizeof(void*) == 8));
        }
        
        SUBCASE("category consistency") {
            using category = sdlpp::platform::platform_category;
            
            // Category flags should match category enum
            if (info.category == category::desktop) {
                CHECK(info.is_desktop);
                CHECK_FALSE(info.is_mobile);
                CHECK_FALSE(info.is_web);
            } else if (info.category == category::mobile) {
                CHECK_FALSE(info.is_desktop);
                CHECK(info.is_mobile);
                CHECK_FALSE(info.is_web);
            } else if (info.category == category::web) {
                CHECK_FALSE(info.is_desktop);
                CHECK_FALSE(info.is_mobile);
                CHECK(info.is_web);
            }
        }
    }
    
    TEST_CASE("power information") {
        auto power = sdlpp::power::get_power_info();
        
        SUBCASE("state consistency") {
            using state = sdlpp::power_state;
            
            if (power.state == state::on_battery) {
                CHECK(power.is_on_battery());
                CHECK_FALSE(power.is_plugged_in());
                CHECK(power.has_battery());
            } else if (power.state == state::no_battery) {
                CHECK_FALSE(power.is_on_battery());
                CHECK(power.is_plugged_in());
                CHECK_FALSE(power.has_battery());
            } else if (power.state == state::charging || power.state == state::charged) {
                CHECK_FALSE(power.is_on_battery());
                CHECK(power.is_plugged_in());
                CHECK(power.has_battery());
            }
        }
        
        SUBCASE("battery info validity") {
            if (power.has_battery()) {
                // If we have battery info, percentages should be valid
                if (power.percent_left >= 0) {
                    CHECK(power.percent_left <= 100);
                }
            }
        }
    }
    
    TEST_CASE("directories") {
        SUBCASE("base path") {
            auto base = sdlpp::directories::get_base_path();
            // Base path might be empty on some platforms
            if (!base.empty()) {
                // SDL typically returns paths with trailing separators
                auto base_str = base.string();
                if (!base_str.empty()) {
                    char last = base_str.back();
                    bool has_separator = last == '/' || last == '\\';
                    CHECK(has_separator);
                }
            }
        }
        
        SUBCASE("pref path") {
            auto pref = sdlpp::directories::get_pref_path("TestOrg", "TestApp");
            // Pref path should not be empty and should end with separator
            if (!pref.empty()) {
                auto pref_str = pref.string();
                if (!pref_str.empty()) {
                    char last = pref_str.back();
                    bool has_separator = last == '/' || last == '\\';
                    CHECK(has_separator);
                }
            }
        }
        
        SUBCASE("user folders") {
            // At least home folder should exist on most platforms
            auto home = sdlpp::directories::get_home_folder();
            // Home might be empty on some embedded platforms
            if (!home.empty()) {
                // Should be an absolute path
                CHECK(home.is_absolute());
            }
        }
    }
    
    TEST_CASE("environment variables") {
        SUBCASE("set and get") {
            const std::string var_name = "SDLPP_TEST_VAR_PLATFORM";
            const std::string var_value = "test_value_12345";
            
            // Set variable
            bool set_ok = sdlpp::environment::set_env(var_name, var_value);
            CHECK(set_ok);
            
            // Get it back
            auto value = sdlpp::environment::get_env(var_name);
            CHECK(value == var_value);
            
            // Unset it
            bool unset_ok = sdlpp::environment::unset_env(var_name);
            CHECK(unset_ok);
            
            // Should be empty now
            auto empty = sdlpp::environment::get_env(var_name);
            CHECK(empty.empty());
        }
        
        SUBCASE("overwrite behavior") {
            const std::string var_name = "SDLPP_TEST_VAR_OVERWRITE";
            const std::string value1 = "first_value";
            const std::string value2 = "second_value";
            
            // Set initial value
            (void)sdlpp::environment::set_env(var_name, value1);
            CHECK(sdlpp::environment::get_env(var_name) == value1);
            
            // Try to set without overwrite
            (void)sdlpp::environment::set_env(var_name, value2, false);
            CHECK(sdlpp::environment::get_env(var_name) == value1); // Should not change
            
            // Set with overwrite
            (void)sdlpp::environment::set_env(var_name, value2, true);
            CHECK(sdlpp::environment::get_env(var_name) == value2);
            
            // Clean up
            (void)sdlpp::environment::unset_env(var_name);
        }
    }
}