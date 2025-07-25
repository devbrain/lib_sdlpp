//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <vector>
#include <iostream>

#include "sdlpp/video/display.hh"
#include "sdlpp/core/error.hh"

using namespace sdlpp;

TEST_SUITE("display") {
    
    // Note: These tests require SDL video subsystem to be initialized
    // In a real application, you would call SDL_Init(SDL_INIT_VIDEO) first
    
    TEST_CASE("display_manager basics") {
        SUBCASE("get all displays") {
            auto result = display_manager::get_displays();
            CHECK(result.has_value());
            
            if (result) {
                const auto& displays = *result;
                // If SDL is initialized, we should have at least one display
                if (displays.size() > 0) {
                    // Verify all displays are valid
                    for (const auto& disp : displays) {
                        CHECK(disp.is_valid());
                        CHECK(disp.get_id() != 0);
                    }
                }
                MESSAGE("Found " << displays.size() << " displays");
            } else {
                MESSAGE("SDL video not initialized or no displays found: " << result.error());
            }
        }
        
        SUBCASE("get primary display") {
            auto result = display_manager::get_primary_display();
            
            if (result) {
                const auto& primary = *result;
                CHECK(primary.is_valid());
                CHECK(primary.get_id() != 0);
            } else {
                MESSAGE("Could not get primary display: " << result.error());
            }
        }
        
        SUBCASE("get display count") {
            auto count = display_manager::get_display_count();

            // Verify count matches get_displays
            if (auto displays_result = display_manager::get_displays()) {
                CHECK(count == displays_result->size());
            }
            MESSAGE("Display count: " << count);
        }
        
        SUBCASE("get system theme") {
            system_theme theme = display_manager::get_system_theme();
            // Theme can be unknown, light, or dark
            CHECK((theme == system_theme::unknown || 
                   theme == system_theme::light || 
                   theme == system_theme::dark));
        }
    }
    
    TEST_CASE("display properties") {
        auto primary_result = display_manager::get_primary_display();
        if (!primary_result) {
            // No display available, skip tests
            return;
        }
        
        const auto& display = *primary_result;
        
        SUBCASE("display name") {
            auto name_result = display.get_name();
            CHECK(name_result.has_value());
            if (name_result) {
                CHECK(!name_result->empty());
                // Print for debugging
                MESSAGE("Display name: " << *name_result);
            }
        }
        
        SUBCASE("display bounds") {
            auto bounds_result = display.get_bounds();
            CHECK(bounds_result.has_value());
            if (bounds_result) {
                const auto& bounds = *bounds_result;
                CHECK(bounds.w > 0);
                CHECK(bounds.h > 0);
                MESSAGE("Display bounds: " << bounds.x << "," << bounds.y 
                        << " " << bounds.w << "x" << bounds.h);
            }
        }
        
        SUBCASE("usable display bounds") {
            auto usable_result = display.get_usable_bounds();
            CHECK(usable_result.has_value());
            if (usable_result) {
                const auto& usable = *usable_result;
                CHECK(usable.w > 0);
                CHECK(usable.h > 0);
                
                // Usable bounds should be within display bounds
                auto bounds_result = display.get_bounds();
                if (bounds_result) {
                    CHECK(usable.w <= bounds_result->w);
                    CHECK(usable.h <= bounds_result->h);
                }
            }
        }
        
        SUBCASE("content scale") {
            auto scale_result = display.get_content_scale();
            CHECK(scale_result.has_value());
            if (scale_result) {
                CHECK(*scale_result > 0.0f);
                MESSAGE("Content scale: " << *scale_result);
            }
        }
        
        SUBCASE("display orientation") {
            auto current_result = display.get_current_orientation();
            CHECK(current_result.has_value());
            
            auto natural_result = display.get_natural_orientation();
            CHECK(natural_result.has_value());
            
            if (current_result) {
                MESSAGE("Current orientation: " << static_cast<int>(*current_result));
            }
            if (natural_result) {
                MESSAGE("Natural orientation: " << static_cast<int>(*natural_result));
            }
        }
        
        SUBCASE("display properties") {
            auto props_result = display.get_properties();
            CHECK(props_result.has_value());
            if (props_result) {
                CHECK(*props_result != 0);
            }
        }
    }
    
    TEST_CASE("display modes") {
        auto primary_result = display_manager::get_primary_display();
        if (!primary_result) {
            return;
        }
        
        const auto& display = *primary_result;
        
        SUBCASE("current display mode") {
            auto mode_result = display.get_current_mode();
            CHECK(mode_result.has_value());
            if (mode_result) {
                const auto& mode = *mode_result;
                CHECK(mode.width > 0);
                CHECK(mode.height > 0);
                CHECK(mode.pixel_density > 0.0f);
                CHECK(mode.format != pixel_format_enum::unknown);
                
                MESSAGE("Current mode: " << mode.width << "x" << mode.height 
                        << " @ " << mode.refresh_rate << "Hz");
            }
        }
        
        SUBCASE("desktop display mode") {
            auto mode_result = display.get_desktop_mode();
            CHECK(mode_result.has_value());
            if (mode_result) {
                const auto& mode = *mode_result;
                CHECK(mode.width > 0);
                CHECK(mode.height > 0);
                
                MESSAGE("Desktop mode: " << mode.width << "x" << mode.height 
                        << " @ " << mode.refresh_rate << "Hz");
            }
        }
        
        SUBCASE("fullscreen display modes") {
            auto modes_result = display.get_fullscreen_modes();
            CHECK(modes_result.has_value());
            if (modes_result) {
                const auto& modes = *modes_result;
                CHECK(modes.size() > 0);  // Should have at least one mode
                
                // Check all modes are valid
                for (const auto& mode : modes) {
                    CHECK(mode.width > 0);
                    CHECK(mode.height > 0);
                    CHECK(mode.pixel_density > 0.0f);
                }
                
                MESSAGE("Found " << modes.size() << " fullscreen modes");
            }
        }
        
        SUBCASE("closest fullscreen mode") {
            // Try to find a mode close to 1920x1080
            auto mode_result = display.get_closest_fullscreen_mode(1920, 1080);
            CHECK(mode_result.has_value());
            if (mode_result) {
                const auto& mode = *mode_result;
                CHECK(mode.width > 0);
                CHECK(mode.height > 0);
                
                MESSAGE("Closest mode to 1920x1080: " << mode.width << "x" << mode.height);
            }
            
            // Try with refresh rate preference
            auto mode_60hz = display.get_closest_fullscreen_mode(1920, 1080, 60.0f);
            CHECK(mode_60hz.has_value());
            
            // Try with high DPI modes
            auto mode_hidpi = display.get_closest_fullscreen_mode(1920, 1080, 0.0f, true);
            CHECK(mode_hidpi.has_value());
        }
    }
    
    TEST_CASE("display mode utilities") {
        display_mode mode;
        mode.width = 1920;
        mode.height = 1080;
        mode.pixel_density = 2.0f;
        mode.refresh_rate = 60.0f;
        mode.refresh_rate_numerator = 60000;
        mode.refresh_rate_denominator = 1001;
        
        SUBCASE("resolution") {
            auto res = mode.resolution();
            CHECK(res.width == 1920);
            CHECK(res.height == 1080);
        }
        
        SUBCASE("precise refresh rate") {
            float precise = mode.get_precise_refresh_rate();
            CHECK(precise == doctest::Approx(59.94f).epsilon(0.01f));
        }
        
        SUBCASE("high DPI check") {
            CHECK(mode.is_high_dpi() == true);
            
            mode.pixel_density = 1.0f;
            CHECK(mode.is_high_dpi() == false);
        }
        
        SUBCASE("SDL conversion") {
            SDL_DisplayMode sdl_mode = mode.to_sdl();
            CHECK(sdl_mode.w == mode.width);
            CHECK(sdl_mode.h == mode.height);
            CHECK(sdl_mode.pixel_density == mode.pixel_density);
            
            display_mode converted = display_mode::from_sdl(sdl_mode);
            CHECK(converted.width == mode.width);
            CHECK(converted.height == mode.height);
            CHECK(converted.pixel_density == mode.pixel_density);
        }
    }
    
    TEST_CASE("display for geometry") {
        SUBCASE("display for point") {
            // Get primary display bounds
            auto primary_result = display_manager::get_primary_display();
            if (!primary_result) return;
            
            auto bounds_result = primary_result->get_bounds();
            if (!bounds_result) return;
            
            // Point in the middle of primary display should find it
            point center{bounds_result->x + bounds_result->w / 2,
                        bounds_result->y + bounds_result->h / 2};
            
            auto display_result = display_manager::get_display_for_point(center);
            CHECK(display_result.has_value());
            if (display_result) {
                CHECK(display_result->get_id() == primary_result->get_id());
            }
        }
        
        SUBCASE("display for rect") {
            auto primary_result = display_manager::get_primary_display();
            if (!primary_result) return;
            
            auto bounds_result = primary_result->get_bounds();
            if (!bounds_result) return;
            
            // Small rect in primary display
            rect test_rect{bounds_result->x + 100, bounds_result->y + 100, 200, 200};
            
            auto display_result = display_manager::get_display_for_rect(test_rect);
            CHECK(display_result.has_value());
            if (display_result) {
                CHECK(display_result->get_id() == primary_result->get_id());
            }
        }
    }
    
    TEST_CASE("display comparison") {
        auto displays_result = display_manager::get_displays();
        if (!displays_result || displays_result->size() < 1) return;
        
        const auto& displays = *displays_result;
        const auto& first = displays[0];
        
        SUBCASE("equality") {
            display copy(first.get_id());
            CHECK(copy == first);
            CHECK(!(copy != first));
        }
        
        SUBCASE("inequality") {
            display invalid;
            CHECK(invalid != first);
            CHECK(!(invalid == first));
        }
    }
    
    TEST_CASE("error handling") {
        SUBCASE("invalid display") {
            display invalid;
            CHECK(!invalid.is_valid());
            CHECK(!invalid);
            
            auto name = invalid.get_name();
            CHECK(!name.has_value());
            CHECK(name.error() == "Invalid display");
            
            auto mode = invalid.get_current_mode();
            CHECK(!mode.has_value());
            
            auto bounds = invalid.get_bounds();
            CHECK(!bounds.has_value());
        }
        
        SUBCASE("invalid display ID") {
            display bad_id(SDL_DisplayID(999999));  // Unlikely to be valid
            
            auto name = bad_id.get_name();
            CHECK(!name.has_value());
            
            auto mode = bad_id.get_current_mode();
            CHECK(!mode.has_value());
        }
    }
    
    TEST_CASE("screen saver control") {
        // Note: Screen saver functions may require SDL video to be initialized
        // SDL disables the screen saver by default
        
        SUBCASE("basic enable/disable") {
            // Save original state
            bool original_state = screen_saver::is_enabled();
            MESSAGE("Original screen saver state: " << (original_state ? "enabled" : "disabled"));
            
            // Test disable
            bool disable_result = screen_saver::disable();
            if (disable_result) {
                CHECK(screen_saver::is_enabled() == false);
            } else {
                MESSAGE("Failed to disable screen saver: " << get_error());
            }
            
            // Test enable
            bool enable_result = screen_saver::enable();
            if (enable_result) {
                CHECK(screen_saver::is_enabled() == true);
            } else {
                MESSAGE("Failed to enable screen saver: " << get_error());
            }
            
            // Restore original state
            if (!original_state) {
                screen_saver::disable();
            }
        }
        
        SUBCASE("double disable/enable") {
            bool original_state = screen_saver::is_enabled();
            
            // Double disable should be fine
            if (screen_saver::disable()) {
                bool second_disable = screen_saver::disable();
                CHECK(second_disable == true);
                CHECK(screen_saver::is_enabled() == false);
            } else {
                MESSAGE("Cannot test double disable - initial disable failed");
            }
            
            // Double enable should be fine
            if (screen_saver::enable()) {
                bool second_enable = screen_saver::enable();
                CHECK(second_enable == true);
                CHECK(screen_saver::is_enabled() == true);
            } else {
                MESSAGE("Cannot test double enable - initial enable failed");
            }
            
            // Restore
            if (!original_state) {
                screen_saver::disable();
            }
        }
        
        SUBCASE("RAII guard") {
            bool original_state = screen_saver::is_enabled();
            
            // Test guard when screen saver is enabled
            if (screen_saver::enable()) {
                {
                    screen_saver::guard guard;
                    // Guard should disable if screen saver was enabled
                    if (guard.is_active()) {
                        CHECK(screen_saver::is_enabled() == false);
                    }
                }
                // Should be re-enabled after guard destruction if it was active
                CHECK(screen_saver::is_enabled() == true);
            } else {
                MESSAGE("Cannot test guard with enabled screen saver");
            }
            
            // Test guard when screen saver is already disabled
            if (screen_saver::disable()) {
                {
                    screen_saver::guard guard;
                    CHECK(guard.is_active() == false);  // Didn't need to disable
                    CHECK(screen_saver::is_enabled() == false);
                }
                // Should still be disabled
                CHECK(screen_saver::is_enabled() == false);
            } else {
                MESSAGE("Cannot test guard with disabled screen saver");
            }
            
            // Restore original state
            if (original_state) {
                screen_saver::enable();
            } else {
                screen_saver::disable();
            }
        }
        
        SUBCASE("nested guards") {
            bool original_state = screen_saver::is_enabled();
            
            if (screen_saver::enable()) {
                {
                    screen_saver::guard guard1;
                    if (guard1.is_active()) {
                        CHECK(screen_saver::is_enabled() == false);
                        
                        {
                            screen_saver::guard guard2;
                            // guard2 won't be active since screen saver already disabled
                            CHECK(guard2.is_active() == false);
                            CHECK(screen_saver::is_enabled() == false);
                        } // guard2 destroyed, but guard1 still active
                        
                        CHECK(screen_saver::is_enabled() == false);
                    }
                } // guard1 destroyed
                
                // Should be re-enabled if guard1 was active
                CHECK(screen_saver::is_enabled() == true);
            } else {
                MESSAGE("Cannot test nested guards - failed to enable screen saver");
            }
            
            // Restore
            if (!original_state) {
                screen_saver::disable();
            }
        }
    }
}