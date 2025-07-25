#include <doctest/doctest.h>
#include <sdlpp/input/touch.hh>
#include <sdlpp/core/core.hh>
#include <iostream>

TEST_SUITE("touch") {
    TEST_CASE("touch enumeration") {
        // Initialize SDL
        auto init = sdlpp::init(sdlpp::init_flags::video | sdlpp::init_flags::events);
        if (init) {
            SUBCASE("get touch devices") {
                auto devices = sdlpp::get_touch_devices();
                // May be empty if no touch devices
                CHECK(devices.size() >= 0);
                
                for (auto id : devices) {
                    std::cout << "Found touch device: " << id << "\n";
                    
                    // Test device name
                    auto name = sdlpp::get_touch_device_name(id);
                    CHECK(!name.empty());
                    std::cout << "  Name: " << name << "\n";
                    
                    // Test device type
                    auto type = sdlpp::get_touch_device_type(id);
                    CHECK(type != sdlpp::touch_device_type::invalid);
                    std::cout << "  Type: " << static_cast<int>(type) << "\n";
                    
                    // Test finger count
                    auto fingers = sdlpp::get_touch_fingers(id);
                    CHECK(fingers.size() >= 0);
                    std::cout << "  Active fingers: " << fingers.size() << "\n";
                }
            }
            
            SUBCASE("touch state helper") {
                auto devices = sdlpp::get_touch_devices();
                if (!devices.empty()) {
                    sdlpp::touch_state state(devices[0]);
                    
                    // Test basic properties
                    CHECK(state.get_device_id() == devices[0]);
                    
                    auto name = state.get_name();
                    CHECK(!name.empty());
                    
                    auto type = state.get_type();
                    CHECK(type != sdlpp::touch_device_type::invalid);
                    
                    // Check type helpers
                    bool is_direct = state.is_direct();
                    bool is_indirect = state.is_indirect();
                    CHECK((is_direct || is_indirect || type == sdlpp::touch_device_type::invalid));
                    
                    // Test finger access
                    auto num_fingers = state.get_num_fingers();
                    CHECK(num_fingers >= 0);
                    
                    auto fingers = state.get_fingers();
                    CHECK(fingers.size() == static_cast<size_t>(num_fingers));
                    
                    // Test primary finger
                    if (num_fingers > 0) {
                        auto primary = state.get_primary_finger();
                        CHECK(primary.has_value());
                        
                        // Test finger properties
                        CHECK(primary->x >= 0.0f);
                        CHECK(primary->x <= 1.0f);
                        CHECK(primary->y >= 0.0f);
                        CHECK(primary->y <= 1.0f);
                        CHECK(primary->pressure >= 0.0f);
                        CHECK(primary->pressure <= 1.0f);
                    }
                    
                    CHECK(state.has_touch() == (num_fingers > 0));
                }
            }
            
            SUBCASE("get all touch states") {
                auto states = sdlpp::get_all_touch_states();
                CHECK(states.size() == sdlpp::get_touch_devices().size());
                
                for (const auto& state : states) {
                    CHECK(state.get_device_id() != 0);
                }
            }
            
        }
    }
    
    TEST_CASE("finger operations") {
        auto init = sdlpp::init(sdlpp::init_flags::video | sdlpp::init_flags::events);
        if (init) {
            auto devices = sdlpp::get_touch_devices();
            if (!devices.empty()) {
                auto id = devices[0];
                sdlpp::touch_state state(id);
                
                SUBCASE("get finger by index") {
                    auto num_fingers = state.get_num_fingers();
                    
                    // Test valid indices
                    for (unsigned i = 0; i < num_fingers; ++i) {
                        auto finger = state.get_finger(i);
                        CHECK(finger.has_value());
                    }
                    
                    // Test invalid index
                    auto invalid = state.get_finger(num_fingers + 10);
                    CHECK(!invalid.has_value());
                }
                
                SUBCASE("find finger by ID") {
                    auto fingers = state.get_fingers();
                    
                    for (const auto& finger : fingers) {
                        auto found = state.find_finger(finger.id);
                        CHECK(found.has_value());
                        CHECK(found->id == finger.id);
                        CHECK(found->x == finger.x);
                        CHECK(found->y == finger.y);
                        CHECK(found->pressure == finger.pressure);
                    }
                    
                    // Test non-existent ID
                    auto not_found = state.find_finger(static_cast<sdlpp::finger_id>(-1));
                    CHECK(!not_found.has_value());
                }
            }
        }
    }
}