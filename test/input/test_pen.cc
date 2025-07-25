#include <doctest/doctest.h>
#include <sdlpp/input/pen.hh>
#include <sdlpp/core/core.hh>
#include <iostream>

TEST_SUITE("pen") {
    TEST_CASE("pen constants") {
        // Just test that our enums and constants compile correctly
        
        SUBCASE("pen input flags") {
            using namespace sdlpp;
            
            pen_input_flags flag1 = pen_input_flags::down;
            pen_input_flags flag2 = pen_input_flags::eraser_tip;
            pen_input_flags flag3 = pen_input_flags::button_1;
            
            // Test OR
            pen_input_flags combined = flag1 | flag2;
            CHECK(has_flag(combined, flag1));
            CHECK(has_flag(combined, flag2));
            CHECK(!has_flag(combined, flag3));
            
            // Test AND
            pen_input_flags intersect = combined & flag1;
            CHECK(has_flag(intersect, flag1));
            CHECK(!has_flag(intersect, flag2));
            
            // Test XOR
            pen_input_flags exclusive = flag1 ^ flag2;
            CHECK(has_flag(exclusive, flag1));
            CHECK(has_flag(exclusive, flag2));
            exclusive ^= flag1;
            CHECK(!has_flag(exclusive, flag1));
            CHECK(has_flag(exclusive, flag2));
            
            // Test compound assignments
            pen_input_flags test = pen_input_flags::none;
            test |= flag1;
            CHECK(has_flag(test, flag1));
            
            test |= flag2;
            CHECK(has_flag(test, flag1));
            CHECK(has_flag(test, flag2));
            
            test &= flag1;
            CHECK(has_flag(test, flag1));
            CHECK(!has_flag(test, flag2));
        }
        
        SUBCASE("pen axis enum") {
            // Just verify the enum values exist
            [[maybe_unused]] auto pressure = sdlpp::pen_axis::pressure;
            [[maybe_unused]] auto xtilt = sdlpp::pen_axis::xtilt;
            [[maybe_unused]] auto ytilt = sdlpp::pen_axis::ytilt;
            [[maybe_unused]] auto distance = sdlpp::pen_axis::distance;
            [[maybe_unused]] auto rotation = sdlpp::pen_axis::rotation;
            [[maybe_unused]] auto slider = sdlpp::pen_axis::slider;
            [[maybe_unused]] auto tangential = sdlpp::pen_axis::tangential_pressure;
            
            CHECK(static_cast<int>(sdlpp::pen_axis::count) > 0);
        }
        
        SUBCASE("special IDs") {
            CHECK(sdlpp::pen_mouse_id == sdlpp::input_constants::pen_as_mouse);
            CHECK(sdlpp::pen_touch_id == sdlpp::input_constants::pen_as_touch);
        }
    }
}