//
// Created by igor on 7/13/25.
//

#include <doctest/doctest.h>
#include <vector>

#include "sdlpp/video/palette.hh"
#include "sdlpp/video/surface.hh"

using namespace sdlpp;

TEST_SUITE("palette") {
    
    TEST_CASE("palette creation and basic operations") {
        SUBCASE("create palette") {
            auto result = palette::create(256);
            REQUIRE(result.has_value());
            auto& pal = *result;
            
            CHECK(pal.is_valid());
            CHECK(pal.size() == 256);
        }
        
        SUBCASE("create grayscale palette") {
            auto result = palette::create_grayscale(8);
            REQUIRE(result.has_value());
            auto& pal = *result;
            
            CHECK(pal.size() == 256);
            
            // Check first and last colors
            auto first = pal.get_color(0);
            CHECK(first.r == 0);
            CHECK(first.g == 0);
            CHECK(first.b == 0);
            
            auto last = pal.get_color(255);
            CHECK(last.r == 255);
            CHECK(last.g == 255);
            CHECK(last.b == 255);
            
            // Check middle gray
            auto mid = pal.get_color(128);
            CHECK(mid.r == mid.g);
            CHECK(mid.g == mid.b);
        }
        
        SUBCASE("create color ramp") {
            color start{255, 0, 0};    // Red
            color end{0, 0, 255};      // Blue
            
            auto result = palette::create_ramp(start, end, 256);
            REQUIRE(result.has_value());
            auto& pal = *result;
            
            CHECK(pal.size() == 256);
            
            // Check endpoints
            auto first = pal.get_color(0);
            CHECK(first.r == 255);
            CHECK(first.g == 0);
            CHECK(first.b == 0);
            
            auto last = pal.get_color(255);
            CHECK(last.r == 0);
            CHECK(last.g == 0);
            CHECK(last.b == 255);
        }
    }
    
    TEST_CASE("palette color operations") {
        auto result = palette::create(256);
        REQUIRE(result.has_value());
        auto& pal = *result;
        
        SUBCASE("set and get single color") {
            color test_color{100, 150, 200};
            auto set_result = pal.set_color(10, test_color);
            CHECK(set_result.has_value());
            
            auto retrieved = pal.get_color(10);
            CHECK(retrieved.r == test_color.r);
            CHECK(retrieved.g == test_color.g);
            CHECK(retrieved.b == test_color.b);
        }
        
        SUBCASE("set multiple colors") {
            std::vector<color> colors = {
                {255, 0, 0},
                {0, 255, 0},
                {0, 0, 255},
                {255, 255, 0}
            };
            
            auto set_result = pal.set_colors(colors, 10);
            CHECK(set_result.has_value());
            
            // Verify colors were set
            CHECK(pal.get_color(10) == colors[0]);
            CHECK(pal.get_color(11) == colors[1]);
            CHECK(pal.get_color(12) == colors[2]);
            CHECK(pal.get_color(13) == colors[3]);
        }
        
        SUBCASE("out of bounds access") {
            auto invalid_color = pal.get_color(1000);
            CHECK(invalid_color == color{0, 0, 0, 255});  // Black
            
            invalid_color = pal.get_color(256);
            CHECK(invalid_color == color{0, 0, 0, 255});  // Black
            
            auto set_result = pal.set_color(256, colors::red);
            CHECK(!set_result.has_value());
            CHECK(set_result.error() == "Index out of bounds");
        }
    }
    
    TEST_CASE("palette reference semantics") {
        auto result = palette::create(16);
        REQUIRE(result.has_value());
        auto& pal = *result;
        
        SUBCASE("palette to palette_ref conversion") {
            palette_ref ref = pal;  // Implicit conversion
            CHECK(ref.is_valid());
            CHECK(ref.size() == pal.size());
            CHECK(ref.get() == pal.get());
            
            // Can modify through palette_ref
            auto set_result = ref.set_color(0, colors::red);
            CHECK(set_result.has_value());
            CHECK(pal.get_color(0) == colors::red);
        }
        
        SUBCASE("palette to const_palette_ref conversion") {
            const_palette_ref cref = pal.cref();  // Explicit conversion
            CHECK(cref.is_valid());
            CHECK(cref.size() == pal.size());
            
            // Can read but not modify
            CHECK(cref.get_color(0) == pal.get_color(0));
            // cref.set_color(0, colors::red);  // Won't compile
        }
        
        SUBCASE("const palette to const_palette_ref") {
            const auto& const_pal = pal;
            const_palette_ref cref = const_pal;  // Should work
            CHECK(cref.is_valid());
            
            // palette_ref ref = const_pal;  // Should not compile
        }
        
        SUBCASE("palette_ref is non-owning") {
            palette_ref ref;
            {
                auto temp_pal = palette::create(16);
                REQUIRE(temp_pal.has_value());
                ref = temp_pal->ref();
                CHECK(ref.is_valid());
            }
            // temp_pal is destroyed here, ref should now be invalid
            // Note: This is technically undefined behavior in real use,
            // but demonstrates the non-owning nature
        }
    }
    
    TEST_CASE("surface palette integration") {
        SUBCASE("indexed surface with palette") {
            // Create an 8-bit indexed surface
            auto surf_result = surface::create_rgb(100, 100, pixel_format_enum::INDEX8);
            REQUIRE(surf_result.has_value());
            auto& surf = *surf_result;
            
            // Create and set a palette
            auto pal_result = palette::create_grayscale(8);
            REQUIRE(pal_result.has_value());
            auto& pal = *pal_result;
            
            auto set_result = surf.set_palette(pal.cref());
            CHECK(set_result.has_value());
            
            // Get mutable palette back
            auto surf_pal = surf.get_palette();
            CHECK(surf_pal.is_valid());
            CHECK(surf.has_palette());
            
            // The palette should have the same colors
            CHECK(surf_pal.size() == pal.size());
            
            // Can modify palette through reference
            auto color_result = surf_pal.set_color(0, colors::red);
            CHECK(color_result.has_value());
            
            // Verify change is visible
            CHECK(surf.get_palette().get_color(0) == colors::red);
        }
        
        SUBCASE("const surface returns const palette reference") {
            auto surf_result = surface::create_rgb(100, 100, pixel_format_enum::INDEX8);
            REQUIRE(surf_result.has_value());
            const auto& const_surf = *surf_result;
            
            // Create and set a palette
            auto pal_result = palette::create_grayscale(8);
            REQUIRE(pal_result.has_value());
            auto set_res = surf_result->set_palette(pal_result->cref());
            CHECK(set_res.has_value());
            
            // Get const palette reference from const surface
            const_palette_ref const_pal = const_surf.get_palette();
            CHECK(const_pal.is_valid());
            
            // Can read colors
            CHECK(const_pal.get_color(0) == color{0, 0, 0});
            
            // Cannot modify through const reference
            // const_pal.set_color(0, colors::red);  // Won't compile
        }
        
        SUBCASE("RGB surface has no palette") {
            auto surf_result = surface::create_rgb(100, 100, pixel_format_enum::RGBA8888);
            REQUIRE(surf_result.has_value());
            auto& surf = *surf_result;
            
            CHECK(!surf.has_palette());
            auto pal_ref = surf.get_palette();
            CHECK(!pal_ref.is_valid());
        }
    }
    
    TEST_CASE("palette utility operations") {
        auto result = palette::create_grayscale(4);  // 16 colors
        REQUIRE(result.has_value());
        auto& pal = *result;
        
        SUBCASE("colors span access") {
            auto colors_span = pal.colors();
            CHECK(colors_span.size() == 16);
            
            // Check it's actually grayscale
            for (size_t i = 0; i < colors_span.size(); ++i) {
                const auto& c = colors_span[i];
                CHECK(c.r == c.g);
                CHECK(c.g == c.b);
            }
        }
        
        SUBCASE("to_vector conversion") {
            auto colors_vec = pal.to_vector();
            CHECK(colors_vec.size() == 16);
            
            // Verify conversion worked
            for (size_t i = 0; i < pal.size(); ++i) {
                CHECK(colors_vec[static_cast<size_t>(i)] == pal.get_color(i));
            }
        }
    }
    
    TEST_CASE("error handling") {
        SUBCASE("invalid palette operations") {
            palette invalid_pal;
            CHECK(!invalid_pal.is_valid());
            
            auto set_result = invalid_pal.set_color(0, colors::red);
            CHECK(!set_result.has_value());
            CHECK(set_result.error() == "Invalid palette");
            
            CHECK(invalid_pal.size() == 0);
            CHECK(invalid_pal.get_color(0) == color{0, 0, 0, 255});
        }
        
        SUBCASE("invalid parameters") {
            auto result = palette::create_grayscale(9);  // > 8 bits
            CHECK(!result.has_value());
            CHECK(result.error() == "Bits must be between 1 and 8");
            
            auto ramp_result = palette::create_ramp(colors::red, colors::blue, 1);
            CHECK(!ramp_result.has_value());
            CHECK(ramp_result.error() == "Steps must be at least 2");
        }
    }
}