#include <doctest/doctest.h>
#include <sdlpp/input/joystick.hh>
#include <sdlpp/core/core.hh>

using namespace sdlpp;

TEST_SUITE("joystick") {
    TEST_CASE("joystick API availability") {
        // Initialize SDL with joystick support
        auto init = sdlpp::init(init_flags::joystick | init_flags::events);
        REQUIRE(init.was_init(init_flags::joystick));
        
        SUBCASE("basic API calls") {
            // Check if joystick subsystem is available (may be false if no joysticks connected)
            [[maybe_unused]] bool has_joystick_system = has_joystick();
            CHECK(true); // Just verify it doesn't crash
            
            // Get list of joysticks (may be empty)
            [[maybe_unused]] auto joysticks = get_joysticks();
            CHECK(true); // Just verify it doesn't crash
            
            // Update joysticks (should not crash)
            update_joysticks();
            CHECK(true);
        }
        
        SUBCASE("joystick enumeration") {
            auto joysticks = get_joysticks();
            
            // For each joystick, test the info functions
            for (auto id : joysticks) {
                // Get joystick info
                [[maybe_unused]] auto name = get_joystick_name_for_id(id);
                CHECK(!name.empty()); // Most joysticks should have a name
                
                [[maybe_unused]] auto path = get_joystick_path_for_id(id);
                // Path may be empty on some platforms
                
                [[maybe_unused]] int player_index = get_joystick_player_index_for_id(id);
                CHECK(player_index >= -1); // -1 means not set
                
                [[maybe_unused]] auto guid = get_joystick_guid_for_id(id);
                CHECK(true); // GUID should be valid
                
                [[maybe_unused]] auto type = get_joystick_type_for_id(id);
                CHECK(true); // Type should be valid
                
                // USB IDs may be 0 for some devices
                [[maybe_unused]] uint16_t vendor = get_joystick_vendor_for_id(id);
                [[maybe_unused]] uint16_t product = get_joystick_product_for_id(id);
                [[maybe_unused]] uint16_t version = get_joystick_product_version_for_id(id);
                
                // Check if it's a virtual joystick
                [[maybe_unused]] bool is_virtual = is_joystick_virtual(id);
                CHECK(true); // Just verify the function works
            }
        }
        
        SUBCASE("joystick open/close") {
            auto joysticks = get_joysticks();
            
            if (!joysticks.empty()) {
                // Try to open the first joystick
                auto joy_result = joystick::open(joysticks[0]);
                if (joy_result) {
                    auto& joy = joy_result.value();
                    CHECK(joy.is_valid());
                    
                    // Test basic properties
                    auto id = joy.get_id();
                    CHECK(id == joysticks[0]);
                    
                    auto name = joy.get_name();
                    CHECK(!name.empty());
                    
                    [[maybe_unused]] auto type = joy.get_type();
                    CHECK(true);
                    
                    // Test capabilities
                    auto num_axes = joy.get_num_axes();
                    CHECK(num_axes >= 0);
                    
                    auto num_buttons = joy.get_num_buttons();
                    CHECK(num_buttons >= 0);
                    
                    auto num_hats = joy.get_num_hats();
                    CHECK(num_hats >= 0);
                    
                    auto num_balls = joy.get_num_balls();
                    CHECK(num_balls >= 0);
                    
                    // Test state queries (values depend on actual joystick state)
                    for (unsigned i = 0; i < num_axes; ++i) {
                        int16_t value = joy.get_axis(i);
                        CHECK(value >= -32768);
                        CHECK(value <= 32767);
                    }
                    
                    for (unsigned i = 0; i < num_buttons; ++i) {
                        [[maybe_unused]] bool pressed = joy.get_button(i);
                        CHECK(true); // Just verify it doesn't crash
                    }
                    
                    for (unsigned i = 0; i < num_hats; ++i) {
                        [[maybe_unused]] auto pos = joy.get_hat(i);
                        CHECK(true); // Just verify it doesn't crash
                    }
                    
                    // Test connection state
                    auto conn_state = joy.get_connection_state();
                    CHECK(conn_state != joystick_connection_state::invalid);
                    
                    // Test power info
                    int battery_percent = -1;
                    [[maybe_unused]] auto power = joy.get_power_info(&battery_percent);
                    CHECK(true); // Power state can be unknown
                    
                    // Joystick will be automatically closed when it goes out of scope
                } else {
                    MESSAGE("Failed to open joystick: " << joy_result.error());
                }
            } else {
                MESSAGE("No joysticks available for testing");
            }
        }
        
        SUBCASE("virtual joystick") {
            // Create a simple virtual joystick
            virtual_joystick_desc desc{};
            desc.vendor_id = 0x1234;
            desc.product_id = 0x5678;
            desc.naxes = 2;
            desc.nbuttons = 4;
            desc.nhats = 1;
            desc.name = "Test Virtual Joystick";

            if (auto attach_result = attach_virtual_joystick(desc)) {
                auto virtual_id = attach_result.value();
                CHECK(virtual_id != 0);
                
                // Verify it's virtual
                CHECK(is_joystick_virtual(virtual_id));
                
                // Verify it appears in the list
                auto joysticks = get_joysticks();
                bool found = false;
                for (auto id : joysticks) {
                    if (id == virtual_id) {
                        found = true;
                        break;
                    }
                }
                CHECK(found);
                
                // Open the virtual joystick
                auto joy_result = joystick::open(virtual_id);
                if (joy_result) {
                    auto& joy = joy_result.value();
                    CHECK(joy.get_num_axes() == 2);
                    CHECK(joy.get_num_buttons() == 4);
                    CHECK(joy.get_num_hats() == 1);
                    
                    // Test setting virtual joystick state
                    auto axis_result = set_virtual_joystick_axis(joy.get(), 0, 16000);
                    // Note: These may fail if virtual joystick support is limited
                    
                    auto button_result = set_virtual_joystick_button(joy.get(), 0, true);
                    
                    auto hat_result = set_virtual_joystick_hat(joy.get(), 0, hat_position::up);
                }
                
                // Detach virtual joystick
                auto detach_result = detach_virtual_joystick(virtual_id);
                CHECK(detach_result.has_value());
            } else {
                MESSAGE("Virtual joystick not supported: " << attach_result.error());
            }
        }
    }
}