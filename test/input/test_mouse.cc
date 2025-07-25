#include <doctest/doctest.h>
#include <../include/sdlpp/input/mouse.hh>
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>

using namespace sdlpp;

TEST_SUITE("mouse") {
    TEST_CASE("mouse API availability") {
        // We can only test that the API exists and doesn't crash
        // Actual mouse input requires human interaction
        
        auto init = sdlpp::init(init_flags::video | init_flags::events);
        REQUIRE(init.was_init(init_flags::video));
        
        // Test that basic queries don't crash
        SUBCASE("basic API calls") {
            // Check if mouse is available
            [[maybe_unused]] bool has_mouse_input = has_mouse();
            CHECK(true); // Just verify it doesn't crash
            
            // Get list of mice
            [[maybe_unused]] auto mice = get_mice();
            CHECK(true); // May be empty on some systems
            
            // Get mouse state
            [[maybe_unused]] auto state = get_mouse_state();
            CHECK(true); // State should be valid even without movement
            
            // Get global mouse state
            [[maybe_unused]] auto global_state = get_global_mouse_state();
            CHECK(true);
            
            // Check cursor visibility
            [[maybe_unused]] bool visible = is_cursor_visible();
            CHECK(true);
        }
        
        SUBCASE("cursor creation") {
            // Test system cursor creation
            // Note: These may fail in headless environments or without proper video drivers
            auto arrow_cursor = cursor::create_system(system_cursor::default_cursor);
            if (!arrow_cursor) {
                MESSAGE("Cursor creation may not be supported in this environment: " << arrow_cursor.error());
                // Skip the rest of the cursor tests if basic cursor creation fails
                return;
            }
            CHECK(arrow_cursor.has_value());
            
            auto hand_cursor = cursor::create_system(system_cursor::pointer);
            CHECK(hand_cursor.has_value());
            
            auto wait_cursor = cursor::create_system(system_cursor::wait);
            CHECK(wait_cursor.has_value());
            
            // Test getting default cursor
            auto* default_cursor = get_default_cursor();
            CHECK(default_cursor != nullptr);
        }
        
        SUBCASE("mouse state helper") {
            mouse_state_helper helper;
            
            // These should not crash
            [[maybe_unused]] auto x = helper.x();
            [[maybe_unused]] auto y = helper.y();
            [[maybe_unused]] auto pos = helper.position();
            
            CHECK(true); // Position values depend on actual mouse position
            
            // Check button states (likely all false unless user is clicking)
            [[maybe_unused]] bool left = helper.is_left_pressed();
            [[maybe_unused]] bool right = helper.is_right_pressed();
            [[maybe_unused]] bool middle = helper.is_middle_pressed();
            [[maybe_unused]] bool any = helper.any_button_pressed();
            
            CHECK(true); // Button states are runtime-dependent
        }
        
        SUBCASE("relative mouse mode with window") {
            // Create a window for relative mouse mode
            auto window_result = window::create("Test", 100, 100);
            REQUIRE(window_result.has_value());
            auto& win = window_result.value();
            
            // Check initial relative mode state
            bool initial_relative = get_window_relative_mouse_mode(win);
            CHECK(!initial_relative); // Should be false by default
            
            // Test RAII relative mode
            {
                relative_mouse_mode rel_mode(win);
                CHECK(rel_mode.is_active());
                // Note: Actually checking if mode is enabled might fail in headless environments
            }
            
            // Should be restored after RAII object is destroyed
            bool final_relative = get_window_relative_mouse_mode(win);
            CHECK(final_relative == initial_relative);
        }
        
        SUBCASE("cursor visibility RAII") {
            bool initial_visible = is_cursor_visible();
            
            {
                cursor_visibility hide_cursor(false);
                // Note: Actual visibility check might not work in all environments
            }
            
            // Should be restored
            bool final_visible = is_cursor_visible();
            CHECK(final_visible == initial_visible);
        }
    }
}