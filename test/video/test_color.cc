//
// Created by igor on 7/13/25.
//

#include <doctest/doctest.h>
#include <type_traits>

#include "sdlpp/video/color.hh"

using namespace sdlpp;

TEST_SUITE("color") {
    
    TEST_CASE("basic_color construction") {
        SUBCASE("default construction") {
            color c;
            CHECK(c.r == 0);
            CHECK(c.g == 0);
            CHECK(c.b == 0);
            CHECK(c.a == 255);
            
            fcolor fc;
            CHECK(fc.r == 0.0f);
            CHECK(fc.g == 0.0f);
            CHECK(fc.b == 0.0f);
            CHECK(fc.a == 1.0f);
        }
        
        SUBCASE("value construction") {
            color c(100, 150, 200, 128);
            CHECK(c.r == 100);
            CHECK(c.g == 150);
            CHECK(c.b == 200);
            CHECK(c.a == 128);
            
            fcolor fc(0.4f, 0.6f, 0.8f, 0.5f);
            CHECK(fc.r == 0.4f);
            CHECK(fc.g == 0.6f);
            CHECK(fc.b == 0.8f);
            CHECK(fc.a == 0.5f);
        }
        
        SUBCASE("SDL conversion") {
            color c(10, 20, 30, 40);
            SDL_Color sdl_c = c.to_sdl();
            CHECK(sdl_c.r == 10);
            CHECK(sdl_c.g == 20);
            CHECK(sdl_c.b == 30);
            CHECK(sdl_c.a == 40);
            
            auto c2 = color::from_sdl(sdl_c);
            CHECK(c2 == c);
            
            fcolor fc(0.1f, 0.2f, 0.3f, 0.4f);
            SDL_FColor sdl_fc = fc.to_sdl();
            CHECK(sdl_fc.r == 0.1f);
            CHECK(sdl_fc.g == 0.2f);
            CHECK(sdl_fc.b == 0.3f);
            CHECK(sdl_fc.a == 0.4f);
            
            auto fc2 = fcolor::from_sdl(sdl_fc);
            CHECK(fc2 == fc);
        }
    }
    
    TEST_CASE("color type conversion") {
        SUBCASE("fcolor to color") {
            fcolor fc(0.5f, 0.75f, 1.0f, 0.25f);
            color c(fc);
            CHECK(c.r == 127);  // 0.5 * 255
            CHECK(c.g == 191);  // 0.75 * 255
            CHECK(c.b == 255);  // 1.0 * 255
            CHECK(c.a == 63);   // 0.25 * 255
        }
        
        SUBCASE("color to fcolor") {
            color c(127, 191, 255, 63);
            fcolor fc(c);
            CHECK(fc.r == doctest::Approx(127.0f / 255.0f));
            CHECK(fc.g == doctest::Approx(191.0f / 255.0f));
            CHECK(fc.b == 1.0f);
            CHECK(fc.a == doctest::Approx(63.0f / 255.0f));
        }
        
        SUBCASE("clamping during conversion") {
            // Values outside [0,1] should be clamped
            fcolor fc(-0.5f, 0.5f, 1.5f, 0.75f);
            color c(fc);
            CHECK(c.r == 0);    // Clamped from negative
            CHECK(c.g == 127);  
            CHECK(c.b == 255);  // Clamped from > 1.0
            CHECK(c.a == 191);
        }
    }
    
    TEST_CASE("color operations") {
        SUBCASE("luminance calculation") {
            color red(255, 0, 0);
            CHECK(red.luminance() == 54);  // 0.2126 * 255
            
            color green(0, 255, 0);
            CHECK(green.luminance() == 182); // 0.7152 * 255
            
            color blue(0, 0, 255);
            CHECK(blue.luminance() == 18);   // 0.0722 * 255
            
            fcolor white(1.0f, 1.0f, 1.0f);
            CHECK(white.luminance() == doctest::Approx(1.0f));
        }
        
        SUBCASE("to grayscale") {
            color c(100, 150, 200);
            auto gray = c.to_grayscale();
            auto lum = c.luminance();
            CHECK(gray.r == lum);
            CHECK(gray.g == lum);
            CHECK(gray.b == lum);
            CHECK(gray.a == c.a); // Alpha preserved
        }
        
        SUBCASE("adjust brightness") {
            color c(100, 100, 100);
            
            auto brighter = c.adjust_brightness(1.5f);
            CHECK(brighter.r == 150);
            CHECK(brighter.g == 150);
            CHECK(brighter.b == 150);
            CHECK(brighter.a == 255); // Alpha unchanged
            
            auto darker = c.adjust_brightness(0.5f);
            CHECK(darker.r == 50);
            CHECK(darker.g == 50);
            CHECK(darker.b == 50);
            
            // Test clamping
            auto too_bright = c.adjust_brightness(3.0f);
            CHECK(too_bright.r == 255);
            CHECK(too_bright.g == 255);
            CHECK(too_bright.b == 255);
        }
        
        SUBCASE("premultiply alpha") {
            color c(200, 100, 50, 128);
            auto premul = c.premultiply();
            CHECK(premul.r == 100);  // 200 * (128/255)
            CHECK(premul.g == 50);   // 100 * (128/255)
            CHECK(premul.b == 25);   // 50 * (128/255)
            CHECK(premul.a == 128);  // Alpha unchanged
            
            fcolor fc(0.8f, 0.6f, 0.4f, 0.5f);
            auto fpremul = fc.premultiply();
            CHECK(fpremul.r == 0.4f);
            CHECK(fpremul.g == 0.3f);
            CHECK(fpremul.b == 0.2f);
            CHECK(fpremul.a == 0.5f);
        }
        
        SUBCASE("mix/lerp colors") {
            color c1(0, 0, 0);
            color c2(255, 255, 255);
            
            auto mid = c1.mix(c2, 0.5f);
            CHECK(mid.r == 127);
            CHECK(mid.g == 127);
            CHECK(mid.b == 127);
            
            auto quarter = c1.mix(c2, 0.25f);
            CHECK(quarter.r == 63);
            CHECK(quarter.g == 63);
            CHECK(quarter.b == 63);
            
            // Also test the free function
            auto lerped = lerp(c1, c2, 0.75f);
            CHECK(lerped.r == 191);
            CHECK(lerped.g == 191);
            CHECK(lerped.b == 191);
        }
        
        SUBCASE("color addition") {
            color c1(100, 100, 100);
            color c2(100, 100, 100);
            
            auto sum = c1 + c2;
            CHECK(sum.r == 200);
            CHECK(sum.g == 200);
            CHECK(sum.b == 200);
            
            // Test clamping
            color c3(200, 200, 200);
            auto clamped = c3 + c1;
            CHECK(clamped.r == 255);
            CHECK(clamped.g == 255);
            CHECK(clamped.b == 255);
        }
        
        SUBCASE("scalar multiplication") {
            color c(100, 50, 25);
            auto doubled = c * 2.0f;
            CHECK(doubled.r == 200);
            CHECK(doubled.g == 100);
            CHECK(doubled.b == 50);
        }
    }
    
    TEST_CASE("alpha blending") {
        SUBCASE("basic alpha blend") {
            color src(255, 0, 0, 128);    // Half-transparent red
            color dst(0, 0, 255, 255);    // Opaque blue
            
            auto blended = alpha_blend(src, dst);
            CHECK(blended.r == 128);  // Half red
            CHECK(blended.g == 0);
            CHECK((blended.b >= 126 && blended.b <= 127));  // Half blue (rounding)
            CHECK(blended.a == 255);  // Result is opaque
        }
        
        SUBCASE("float alpha blend") {
            fcolor src(1.0f, 0.0f, 0.0f, 0.5f);
            fcolor dst(0.0f, 0.0f, 1.0f, 1.0f);
            
            auto blended = alpha_blend(src, dst);
            CHECK(blended.r == 0.5f);
            CHECK(blended.g == 0.0f);
            CHECK(blended.b == 0.5f);
            CHECK(blended.a == 1.0f);
        }
    }
    
    TEST_CASE("32-bit color conversion") {
        SUBCASE("to_rgba32") {
            color c(0xAB, 0xCD, 0xEF, 0x12);
            uint32_t rgba = to_rgba32(c);
            CHECK(rgba == 0xABCDEF12);
        }
        
        SUBCASE("from_rgba32") {
            uint32_t rgba = 0xDEADBEEF;
            color c = from_rgba32(rgba);
            CHECK(c.r == 0xDE);
            CHECK(c.g == 0xAD);
            CHECK(c.b == 0xBE);
            CHECK(c.a == 0xEF);
        }
    }
    
    TEST_CASE("predefined colors") {
        SUBCASE("basic colors") {
            CHECK(colors::black == color(0, 0, 0));
            CHECK(colors::white == color(255, 255, 255));
            CHECK(colors::red == color(255, 0, 0));
            CHECK(colors::green == color(0, 255, 0));
            CHECK(colors::blue == color(0, 0, 255));
            CHECK(colors::transparent == color(0, 0, 0, 0));
        }
        
        SUBCASE("float colors") {
            CHECK(colors::f::black == fcolor(0.0f, 0.0f, 0.0f));
            CHECK(colors::f::white == fcolor(1.0f, 1.0f, 1.0f));
            CHECK(colors::f::red == fcolor(1.0f, 0.0f, 0.0f));
            CHECK(colors::f::transparent == fcolor(0.0f, 0.0f, 0.0f, 0.0f));
        }
    }
    
    TEST_CASE("color_like concept") {
        SUBCASE("concept verification") {
            CHECK(color_like<color>);
            CHECK(color_like<fcolor>);
            CHECK(!color_like<int>);
            CHECK(!color_like<SDL_Color>);  // SDL types don't have value_type
        }
        
        SUBCASE("value_type verification") {
            CHECK(std::is_same_v<color::value_type, uint8_t>);
            CHECK(std::is_same_v<fcolor::value_type, float>);
        }
    }
}