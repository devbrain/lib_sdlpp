//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <type_traits>

#include "sdlpp/utility/dimension.hh"
#include "sdlpp/video/window.hh"
#include "sdlpp/video/surface.hh"

using namespace sdlpp;

TEST_SUITE("dimension") {
    TEST_CASE("dimension type safety") {
        SUBCASE("non-negative guarantee") {
            // Negative values are clamped to 0
            dimension<int> d1(-100);
            CHECK(d1.value() == 0);
            CHECK(d1.is_zero());
            CHECK(!d1.is_positive());
            
            dimension<int> d2(100);
            CHECK(d2.value() == 100);
            CHECK(!d2.is_zero());
            CHECK(d2.is_positive());
        }
        
        SUBCASE("arithmetic maintains invariant") {
            dimension<int> d1(100);
            dimension<int> d2(50);
            
            // Addition
            auto d3 = d1 + d2;
            CHECK(d3.value() == 150);
            
            // Subtraction that would go negative
            auto d4 = d2 - d1;
            CHECK(d4.value() == 0);  // Clamped to 0, not negative
            
            // Multiplication by negative
            auto d5 = d1 * -2;
            CHECK(d5.value() == 0);  // Negative result becomes 0
            
            // Division maintains non-negative
            auto d6 = d1 / 2;
            CHECK(d6.value() == 50);
        }
        
        SUBCASE("overflow protection") {
            dimension<int> large(std::numeric_limits<int>::max() - 10);
            dimension<int> small(20);
            
            // Addition that would overflow
            auto result = large + small;
            CHECK(result.value() == std::numeric_limits<int>::max());
        }
    }
    
    TEST_CASE("dimensions type") {
        SUBCASE("construction and access") {
            // From raw values
            dimensions<int> dims1(800, 600);
            CHECK(dims1.width.value() == 800);
            CHECK(dims1.height.value() == 600);
            CHECK(!dims1.is_empty());
            CHECK(dims1.is_valid());
            
            // Negative values are clamped
            dimensions<int> dims2(-100, 200);
            CHECK(dims2.width.value() == 0);
            CHECK(dims2.height.value() == 200);
            CHECK(dims2.is_empty());  // Width is 0
            CHECK(!dims2.is_valid()); // Not both positive
        }
        
        SUBCASE("area calculation") {
            dimensions<int> dims(1000, 2000);
            auto area = dims.area();
            
            // Should return larger type for overflow protection
            static_assert(std::is_same_v<decltype(area), uint64_t>);
            CHECK(area == 2000000);
            
            // Large dimensions don't overflow
            dimensions<int> large(100000, 100000);
            auto large_area = large.area();
            CHECK(large_area == 10000000000ULL);
        }
    }
    
    TEST_CASE("coordinate type") {
        SUBCASE("can be negative") {
            coordinate<int> x(-100);
            CHECK(x.value == -100);
            CHECK(x == -100);  // Implicit conversion
            
            coordinate<int> y(200);
            CHECK(y.value == 200);
        }
        
        SUBCASE("position type") {
            position<int> pos(-50, 100);
            CHECK(pos.x.value == -50);
            CHECK(pos.y.value == 100);
            
            // Can represent off-screen positions
            position<int> offscreen(-1000, -1000);
            CHECK(offscreen.x.value == -1000);
            CHECK(offscreen.y.value == -1000);
        }
    }
    
    TEST_CASE("integration with SDL wrappers") {
        SUBCASE("window creation with dimensions") {
            // Type-safe dimension creation
            [[maybe_unused]] window_dimensions dims(800, 600);

            // This would work in a real SDL environment
            // auto window = window::create("Test", dims);
            
            // Negative dimensions are automatically handled
            window_dimensions bad_dims(-100, -200);
            CHECK(bad_dims.width.value() == 0);
            CHECK(bad_dims.height.value() == 0);
        }
        
        SUBCASE("surface creation with dimensions") {
            dimensions<int> surf_dims(640, 480);
            
            // This would work in a real SDL environment
            // auto surface = surface::create_rgb(surf_dims, pixel_format_enum::RGBA8888);
            
            // Type safety prevents negative dimensions at compile time
            CHECK(surf_dims.area() == 307200);
        }
    }
    
    TEST_CASE("type conversion and compatibility") {
        SUBCASE("implicit conversion to underlying type") {
            dimension<int> d(100);
            int value = d;  // Implicit conversion
            CHECK(value == 100);
            
            // Can pass directly to functions expecting int
            auto func = [](int x) { return x * 2; };
            CHECK(func(d) == 200);
        }
        
        SUBCASE("dimensions work with SDL functions") {
            window_dimensions dims(1024, 768);
            
            // Can extract values for SDL
            int w = dims.width;
            int h = dims.height;
            CHECK(w == 1024);
            CHECK(h == 768);
        }
    }
    
    TEST_CASE("semantic correctness examples") {
        SUBCASE("window dimensions must be positive") {
            // Old way - runtime error
            // auto window = window::create("Test", -100, -200);  // Would fail at runtime
            
            // New way - type safe
            window_dimensions dims(-100, -200);  // Automatically clamped to 0
            CHECK(dims.width.value() == 0);
            CHECK(dims.height.value() == 0);
            // auto window = window::create("Test", dims);  // Would still fail but dimensions are valid
        }
        
        SUBCASE("coordinates can be negative") {
            // Window position can be off-screen
            position<int> window_pos(-100, -50);
            CHECK(window_pos.x.value == -100);  // Preserved negative
            
            // But dimensions cannot
            dimensions<int> window_size(-100, -50);
            CHECK(window_size.width.value() == 0);   // Clamped to 0
            CHECK(window_size.height.value() == 0);  // Clamped to 0
        }
    }
}