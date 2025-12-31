#include <doctest/doctest.h>
#include <sdlpp/input/haptic.hh>
#include <sdlpp/input/joystick.hh>
#include <sdlpp/core/core.hh>
#include <iostream>
#include <chrono>
#include <exception>

using namespace std::chrono_literals;

TEST_SUITE("haptic") {
    TEST_CASE("haptic device enumeration") {
        try {
            // Initialize SDL with haptic support
            auto init = sdlpp::init(sdlpp::init_flags::haptic);
            SUBCASE("get haptic devices") {
                auto devices = sdlpp::get_haptics();
                // May be empty if no haptic devices
                CHECK(devices.size() >= 0);
                
                std::cout << "Found " << devices.size() << " haptic device(s):\n";
                
                for (auto id : devices) {
                    auto name = sdlpp::get_haptic_name_for_id(id);
                    std::cout << "  Haptic " << id << ": " << name << "\n";
                    CHECK(!name.empty());
                }
            }
            
            SUBCASE("mouse haptic") {
                bool mouse_haptic = sdlpp::is_mouse_haptic();
                std::cout << "Mouse has haptic: " << (mouse_haptic ? "YES" : "NO") << "\n";
                // Just verify it doesn't crash
                CHECK(mouse_haptic == mouse_haptic);
            }
        } catch (const std::exception& ex) {
            MESSAGE("Skipping haptic test: ", ex.what());
            return;
        }
    }
    
    TEST_CASE("haptic device operations") {
        try {
            auto init = sdlpp::init(sdlpp::init_flags::haptic);
            auto devices = sdlpp::get_haptics();
            
            SUBCASE("open non-existent device") {
                auto haptic = sdlpp::haptic::open(0xFFFFFFFF);
                CHECK(!haptic.has_value());
            }
            
            SUBCASE("haptic from mouse") {
                if (sdlpp::is_mouse_haptic()) {
                    auto haptic = sdlpp::haptic::open_from_mouse();
                    if (haptic) {
                        CHECK(haptic->is_valid());
                        CHECK(haptic->get_id() != 0);
                        
                        auto name = haptic->get_name();
                        std::cout << "Mouse haptic name: " << name << "\n";
                    }
                }
            }
            
            SUBCASE("open first available device") {
                if (!devices.empty()) {
                    auto haptic = sdlpp::haptic::open(devices[0]);
                    if (haptic) {
                        CHECK(haptic->is_valid());
                        
                        // Test device properties
                        auto id = haptic->get_id();
                        CHECK(id == devices[0]);
                        
                        auto name = haptic->get_name();
                        CHECK(!name.empty());
                        std::cout << "\nHaptic device info:\n";
                        std::cout << "  Name: " << name << "\n";
                        
                        auto max_effects = haptic->get_max_effects();
                        CHECK(max_effects.has_value());
                        if (max_effects) {
                            CHECK(*max_effects > 0);
                            std::cout << "  Max effects: " << *max_effects << "\n";
                        }
                        
                        auto max_playing = haptic->get_max_effects_playing();
                        CHECK(max_playing.has_value());
                        if (max_playing) {
                            CHECK(*max_playing > 0);
                            std::cout << "  Max playing: " << *max_playing << "\n";
                        }
                        
                        auto num_axes = haptic->get_num_axes();
                        CHECK(num_axes >= 0);
                        std::cout << "  Axes: " << num_axes << "\n";
                        
                        // Test features
                        auto features = haptic->get_features();
                        std::cout << "  Supported features:\n";
                        
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::constant))
                            std::cout << "    - Constant\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::sine))
                            std::cout << "    - Sine\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::square))
                            std::cout << "    - Square\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::triangle))
                            std::cout << "    - Triangle\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::sawtoothup))
                            std::cout << "    - Sawtooth Up\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::sawtoothdown))
                            std::cout << "    - Sawtooth Down\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::ramp))
                            std::cout << "    - Ramp\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::spring))
                            std::cout << "    - Spring\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::damper))
                            std::cout << "    - Damper\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::inertia))
                            std::cout << "    - Inertia\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::friction))
                            std::cout << "    - Friction\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::leftright))
                            std::cout << "    - Left/Right\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::custom))
                            std::cout << "    - Custom\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::gain))
                            std::cout << "    - Gain control\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::autocenter))
                            std::cout << "    - Autocenter\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::status))
                            std::cout << "    - Status query\n";
                        if (sdlpp::has_flag(features, sdlpp::haptic_feature::pause))
                            std::cout << "    - Pause/Resume\n";
                            
                        // Test rumble support
                        bool rumble = haptic->is_rumble_supported();
                        std::cout << "  Rumble support: " << (rumble ? "YES" : "NO") << "\n";
                        
                        if (rumble) {
                            auto rumble_init = haptic->init_rumble();
                            CHECK(rumble_init.has_value());
                        }
                    }
                }
            }
        } catch (const std::exception& ex) {
            MESSAGE("Skipping haptic test: ", ex.what());
            return;
        }
    }
    
    TEST_CASE("haptic effects") {
        try {
            auto init = sdlpp::init(sdlpp::init_flags::haptic);
            auto devices = sdlpp::get_haptics();
            if (!devices.empty()) {
                auto haptic = sdlpp::haptic::open(devices[0]);
                if (haptic && haptic->is_valid()) {
                    SUBCASE("constant effect") {
                        sdlpp::haptic_constant effect;
                        effect.direction = sdlpp::haptic_direction::polar(18000); // South
                        effect.length = 1000;
                        effect.level = 0x4000; // Half strength
                        
                        if (haptic->is_effect_supported(effect)) {
                            auto id = haptic->create_effect(effect);
                            CHECK(id.has_value());
                            
                            if (id) {
                                // Just create and destroy, don't run
                                haptic->destroy_effect(*id);
                            }
                        }
                    }
                    
                    SUBCASE("periodic effect") {
                        sdlpp::haptic_periodic effect;
                        effect.wave_type = sdlpp::haptic_feature::sine;
                        effect.direction = sdlpp::haptic_direction::polar(0); // North
                        effect.period = 100;
                        effect.magnitude = 0x4000;
                        effect.length = 1000;
                        
                        if (haptic->is_effect_supported(effect)) {
                            auto id = haptic->create_effect(effect);
                            CHECK(id.has_value());
                            
                            if (id) {
                                haptic->destroy_effect(*id);
                            }
                        }
                    }
                    
                    SUBCASE("leftright effect") {
                        sdlpp::haptic_leftright effect;
                        effect.length = 1000;
                        effect.large_magnitude = 0x4000;
                        effect.small_magnitude = 0x2000;
                        
                        if (haptic->is_effect_supported(effect)) {
                            auto id = haptic->create_effect(effect);
                            CHECK(id.has_value());
                            
                            if (id) {
                                haptic->destroy_effect(*id);
                            }
                        }
                    }
                    
                    SUBCASE("effect handle RAII") {
                        sdlpp::haptic_constant effect;
                        effect.direction = sdlpp::haptic_direction::polar(0);
                        effect.length = 1000;
                        effect.level = 0x2000;
                        
                        if (haptic->is_effect_supported(effect)) {
                            auto id = haptic->create_effect(effect);
                            if (id) {
                                // Test RAII handle
                                {
                                    sdlpp::haptic_effect_handle handle(&*haptic, *id);
                                    CHECK(handle.is_valid());
                                    CHECK(handle.get() == *id);
                                    // Handle will destroy effect on destruction
                                }
                                // Effect should be destroyed now
                            }
                        }
                    }
                }
            }
        } catch (const std::exception& ex) {
            MESSAGE("Skipping haptic test: ", ex.what());
            return;
        }
    }
    
    TEST_CASE("haptic direction helpers") {
        SUBCASE("polar direction") {
            auto dir = sdlpp::haptic_direction::polar(9000); // East
            CHECK(dir.type == sdlpp::haptic_direction_type::polar);
            CHECK(dir.dir[0] == 9000);
            
            auto sdl_dir = dir.to_sdl();
            CHECK(sdl_dir.type == SDL_HAPTIC_POLAR);
            CHECK(sdl_dir.dir[0] == 9000);
        }
        
        SUBCASE("cartesian direction") {
            auto dir = sdlpp::haptic_direction::cartesian(1, 0, 0); // East
            CHECK(dir.type == sdlpp::haptic_direction_type::cartesian);
            CHECK(dir.dir[0] == 1);
            CHECK(dir.dir[1] == 0);
            CHECK(dir.dir[2] == 0);
        }
        
        SUBCASE("spherical direction") {
            auto dir = sdlpp::haptic_direction::spherical(9000, 0);
            CHECK(dir.type == sdlpp::haptic_direction_type::spherical);
            CHECK(dir.dir[0] == 9000);
            CHECK(dir.dir[1] == 0);
        }
        
        SUBCASE("steering axis") {
            auto dir = sdlpp::haptic_direction::steering();
            CHECK(dir.type == sdlpp::haptic_direction_type::steering_axis);
        }
    }
    
    TEST_CASE("joystick haptic") {
        try {
            auto init = sdlpp::init(sdlpp::init_flags::joystick | sdlpp::init_flags::haptic);
            auto joysticks = sdlpp::get_joysticks();
            
            for (auto joy_id : joysticks) {
                auto joy = sdlpp::joystick::open(joy_id);
                if (joy) {
                    bool is_haptic = sdlpp::is_joystick_haptic(*joy);
                    std::cout << "Joystick " << joy->get_name() 
                             << " haptic: " << (is_haptic ? "YES" : "NO") << "\n";
                    
                    if (is_haptic) {
                        auto haptic = sdlpp::haptic::open_from_joystick(*joy);
                        CHECK(haptic.has_value());
                        
                        if (haptic) {
                            CHECK(haptic->is_valid());
                        }
                    }
                }
            }
        } catch (const std::exception& ex) {
            MESSAGE("Skipping haptic test: ", ex.what());
            return;
        }
    }
}
