//
// Test window geometry concept updates
//

#include <doctest/doctest.h>
#include <sdlpp/video/window.hh>

// Test with built-in geometry types
TEST_CASE("window accepts built-in geometry types") {
    // Test create with built-in size
    {
        auto result = sdlpp::window::create("Test", sdlpp::size_i{800, 600});
        CHECK(result.has_value());
    }
    
    // Test window position/size getters return proper types
    {
        auto window_result = sdlpp::window::create("Test", 800, 600);
        if (window_result) {
            auto& window = *window_result;
            
            // Get position with default type
            auto pos = window.get_position();
            CHECK(pos.has_value());
            
            // Get size with default type  
            auto size = window.get_size();
            CHECK(size.has_value());
        }
    }
}

// Test with custom geometry types
TEST_CASE("window accepts custom geometry types") {
    struct CustomPoint {
        using value_type = int;
        int x, y;
    };
    
    struct CustomSize {
        using value_type = int;
        int width, height;
    };
    
    struct CustomRect {
        using value_type = int;
        int x, y, w, h;
    };
    
    // Verify concepts
    static_assert(sdlpp::point_like<CustomPoint>);
    static_assert(sdlpp::size_like<CustomSize>);
    static_assert(sdlpp::rect_like<CustomRect>);
    
    // Test create with custom size
    {
        auto result = sdlpp::window::create("Test", CustomSize{800, 600});
        CHECK(result.has_value());
    }
    
    // Test position/size with custom types
    {
        auto window_result = sdlpp::window::create("Test", 800, 600);
        if (window_result) {
            auto& window = *window_result;
            
            // Set position with custom point
            auto set_result = window.set_position(CustomPoint{100, 100});
            CHECK(set_result.has_value());
            
            // Get position as custom point
            auto pos = window.get_position<CustomPoint>();
            CHECK(pos.has_value());
            if (pos) {
                CHECK(pos->x == 100);
                CHECK(pos->y == 100);
            }
            
            // Get size as custom size
            auto size = window.get_size<CustomSize>();
            CHECK(size.has_value());
            if (size) {
                CHECK(size->width == 800);
                CHECK(size->height == 600);
            }
            
            // Set min/max size with custom types
            CHECK(window.set_minimum_size(CustomSize{400, 300}).has_value());
            CHECK(window.set_maximum_size(CustomSize{1920, 1080}).has_value());
            
            // Get min/max with custom types
            auto min_size = window.get_minimum_size<CustomSize>();
            CHECK(min_size.has_value());
            
            auto max_size = window.get_maximum_size<CustomSize>();
            CHECK(max_size.has_value());
        }
    }
    
    // Test update_surface_rects with custom rect container
    {
        auto window_result = sdlpp::window::create("Test", 800, 600);
        if (window_result) {
            auto& window = *window_result;
            
            std::vector<CustomRect> rects = {
                {0, 0, 100, 100},
                {100, 100, 200, 200}
            };
            
            // Get the window surface first
            auto surface_result = window.get_surface();
            if (!surface_result) {
                MESSAGE("Cannot get window surface: " << surface_result.error());
                // Skip this test if we can't get a surface
                return;
            }
            
            // This should compile and work
            auto update_result = window.update_surface_rects(rects);
            if (!update_result) {
                MESSAGE("update_surface_rects failed: " << update_result.error());
            }
            
            // Also test with SDL_Rect for backwards compatibility
            std::vector<SDL_Rect> sdl_rects = {
                {0, 0, 100, 100},
                {100, 100, 200, 200}
            };
            auto update_result2 = window.update_surface_rects(std::span<const SDL_Rect>(sdl_rects));
            if (!update_result2) {
                MESSAGE("update_surface_rects with SDL_Rect failed: " << update_result2.error());
            }
        }
    }
}

// Test create_at with custom types
TEST_CASE("window create_at accepts custom geometry types") {
    struct CustomPoint {
        using value_type = int;
        int x, y;
    };
    
    struct CustomSize {
        using value_type = int;
        int width, height;
    };
    
    auto window = sdlpp::window::create_at("Test",
        CustomPoint{100, 100},
        CustomSize{800, 600});
    
    CHECK(window.has_value());
    
    if (window) {
        auto pos = window->get_position<CustomPoint>();
        CHECK(pos.has_value());
        if (pos) {
            CHECK(pos->x == 100);
            CHECK(pos->y == 100);
        }
    }
}

// Test that it still works without specifying types (uses defaults)
TEST_CASE("window geometry methods work with default types") {
    auto window_result = sdlpp::window::create("Test", 800, 600);
    REQUIRE(window_result.has_value());
    
    auto& window = *window_result;
    
    // These should use the default built-in types
    auto pos = window.get_position();
    CHECK(pos.has_value());
    
    auto size = window.get_size();
    CHECK(size.has_value());
    
    auto min_size = window.get_minimum_size();
    CHECK(min_size.has_value());
    
    auto max_size = window.get_maximum_size();
    CHECK(max_size.has_value());
}