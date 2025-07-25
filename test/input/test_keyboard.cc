#include <doctest/doctest.h>
#include <../include/sdlpp/input/keyboard.hh>
#include <sdlpp/core/core.hh>

using namespace sdlpp;

TEST_SUITE("keyboard") {
    TEST_CASE("keyboard API availability") {
        // We can only test that the API exists and doesn't crash
        // Actual keyboard input requires human interaction
        
        auto init = sdlpp::init(init_flags::video | init_flags::events);
        REQUIRE(init.was_init(init_flags::video));
        
        // Test that basic queries don't crash
        SUBCASE("basic API calls") {
            // These should not crash even without keyboard input
            auto state = get_keyboard_state();
            CHECK(!state.empty());
            
            [[maybe_unused]] auto mods = get_mod_state();
            CHECK(true); // Just verify it doesn't crash
            
            [[maybe_unused]] bool has_kb = has_keyboard();
            CHECK(true); // Platform dependent
            
            auto keyboards = get_keyboards();
            CHECK(true); // May be empty on some platforms
        }
        
        SUBCASE("key name conversions") {
            // These conversions should work without actual key presses
            auto name = get_key_name(keycodes::space);
            CHECK(name == "Space");
            
            auto key = get_key_from_name("Escape");
            CHECK(key == keycodes::escape);
            
            auto scan = get_scancode_from_name("Tab");
            CHECK(scan == scancode::tab);
        }
    }
}