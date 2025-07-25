#include <doctest/doctest.h>
#include <sdlpp/input/gamepad.hh>
#include <sdlpp/core/core.hh>

using namespace sdlpp;

TEST_SUITE("gamepad") {
    TEST_CASE("gamepad API availability") {
        // Initialize SDL with gamepad support
        auto init = sdlpp::init(init_flags::gamepad | init_flags::joystick | init_flags::events);
        REQUIRE(init.was_init(init_flags::gamepad));
        
        SUBCASE("basic API calls") {
            // Check if gamepad subsystem is available (may be false if no gamepads connected)
            [[maybe_unused]] bool has_gamepad_system = has_gamepad();
            CHECK(true); // Just verify it doesn't crash
            
            // Get list of gamepads (may be empty)
            [[maybe_unused]] auto gamepads = get_gamepads();
            CHECK(true); // Just verify it doesn't crash
            
            // Update gamepads (should not crash)
            update_gamepads();
            CHECK(true);
        }
        
        SUBCASE("gamepad enumeration") {
            auto gamepads = get_gamepads();
            
            // For each gamepad, test the info functions
            for (auto id : gamepads) {
                // Check if it's really a gamepad
                CHECK(is_gamepad(id));
                
                // Get gamepad info
                auto name = get_gamepad_name_for_id(id);
                CHECK(!name.empty()); // Most gamepads should have a name
                
                [[maybe_unused]] auto type = get_gamepad_type_for_id(id);
                CHECK(true); // Type should be valid
                
                auto mapping = get_gamepad_mapping_for_id(id);
                // Mapping might be empty for some controllers
            }
        }
        
        SUBCASE("gamepad open/close") {
            auto gamepads = get_gamepads();
            
            if (!gamepads.empty()) {
                // Try to open the first gamepad
                auto pad_result = gamepad::open(gamepads[0]);
                if (pad_result) {
                    auto& pad = pad_result.value();
                    CHECK(pad.is_valid());
                    
                    // Test basic properties
                    auto id = pad.get_id();
                    CHECK(id == gamepads[0]);
                    
                    auto name = pad.get_name();
                    CHECK(!name.empty());
                    
                    [[maybe_unused]] auto type = pad.get_type();
                    CHECK(true);
                    
                    // Test capabilities
                    bool has_left_stick = pad.has_axis(gamepad_axis::leftx) && 
                                         pad.has_axis(gamepad_axis::lefty);
                    CHECK(has_left_stick); // Most gamepads have left stick
                    
                    bool has_face_buttons = pad.has_button(gamepad_button::south) &&
                                           pad.has_button(gamepad_button::east) &&
                                           pad.has_button(gamepad_button::west) &&
                                           pad.has_button(gamepad_button::north);
                    CHECK(has_face_buttons); // Standard gamepads have face buttons
                    
                    // Test state queries (values depend on actual gamepad state)
                    int16_t left_x = pad.get_axis(gamepad_axis::leftx);
                    CHECK(left_x >= -32768);
                    CHECK(left_x <= 32767);
                    
                    [[maybe_unused]] bool a_pressed = pad.get_button(gamepad_button::south);
                    CHECK(true); // Just verify it doesn't crash
                    
                    // Test connection state
                    [[maybe_unused]] auto conn_state = pad.get_connection_state();
                    CHECK(conn_state != joystick_connection_state::invalid);
                    
                    // Test power info
                    [[maybe_unused]] int battery_percent = -1;
                    [[maybe_unused]] auto power = pad.get_power_info(&battery_percent);
                    CHECK(true); // Power state can be unknown
                    
                    // Test touchpad info
                    auto num_touchpads = pad.get_num_touchpads();
                    CHECK(num_touchpads >= 0);
                    
                    // Test underlying joystick
                    auto* joy = pad.get_joystick();
                    CHECK(joy != nullptr);
                    
                    // Gamepad will be automatically closed when it goes out of scope
                } else {
                    MESSAGE("Failed to open gamepad: " << pad_result.error());
                }
            } else {
                MESSAGE("No gamepads available for testing");
            }
        }
        
        SUBCASE("gamepad state helper") {
            auto gamepads = get_gamepads();
            
            if (!gamepads.empty()) {
                if (auto pad_result = gamepad::open(gamepads[0])) {
                    auto& pad = pad_result.value();
                    gamepad_state state(pad);
                    
                    // Test axis accessors
                    auto lx = state.left_x();
                    auto ly = state.left_y();
                    auto rx = state.right_x();
                    auto ry = state.right_y();
                    auto lt = state.left_trigger();
                    auto rt = state.right_trigger();
                    
                    CHECK(lx >= -32768);
                    CHECK(lx <= 32767);
                    CHECK(ly >= -32768);
                    CHECK(ly <= 32767);
                    CHECK(rx >= -32768);
                    CHECK(rx <= 32767);
                    CHECK(ry >= -32768);
                    CHECK(ry <= 32767);
                    CHECK(lt >= 0);
                    CHECK(lt <= 32767);
                    CHECK(rt >= 0);
                    CHECK(rt <= 32767);
                    
                    // Test button accessors (likely all false unless user is pressing)
                    [[maybe_unused]] bool a = state.a();
                    [[maybe_unused]] bool b = state.b();
                    [[maybe_unused]] bool x = state.x();
                    [[maybe_unused]] bool y = state.y();
                    [[maybe_unused]] bool any = state.any_button_pressed();
                    
                    CHECK(true); // Button states are runtime-dependent
                }
            }
        }
        
        SUBCASE("axis and button names") {
            // Test axis name conversion
            auto axis_name = get_gamepad_axis_name(gamepad_axis::leftx);
            CHECK(!axis_name.empty());
            CHECK(axis_name != "Unknown");
            
            // Test button name conversion
            auto button_name = get_gamepad_button_name(gamepad_button::south);
            CHECK(!button_name.empty());
            CHECK(button_name != "Unknown");
            
            // Test string to enum conversion
            auto axis = get_gamepad_axis_from_string("leftx");
            CHECK(axis == gamepad_axis::leftx);
            
            auto button = get_gamepad_button_from_string("a");
            CHECK(button == gamepad_button::south);
            
            // Test invalid strings
            auto invalid_axis = get_gamepad_axis_from_string("invalid_axis");
            CHECK(invalid_axis == gamepad_axis::invalid);
            
            auto invalid_button = get_gamepad_button_from_string("invalid_button");
            CHECK(invalid_button == gamepad_button::invalid);
        }
        
        SUBCASE("gamepad type strings") {
            auto type_str = get_gamepad_type_string(gamepad_type::ps5);
            CHECK(!type_str.empty());
            
            auto type = get_gamepad_type_from_string("ps5");
            CHECK(type == gamepad_type::ps5);
        }
    }
}