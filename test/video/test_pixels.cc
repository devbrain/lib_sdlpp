//
// Created by igor on 7/13/25.
//
#include <doctest/doctest.h>
#include <vector>

#include "sdlpp/video/pixels.hh"

using namespace sdlpp;

TEST_SUITE("pixels") {

    TEST_CASE("color to_sdl and from_sdl roundtrip") {
        color c(10, 20, 30, 40);
        SDL_Color sdl_c = c.to_sdl();
        color c2 = color::from_sdl(sdl_c);

        CHECK_EQ(c.r, c2.r);
        CHECK_EQ(c.g, c2.g);
        CHECK_EQ(c.b, c2.b);
        CHECK_EQ(c.a, c2.a);
    }

    TEST_CASE("fcolor to_sdl and from_sdl roundtrip") {
        fcolor fc(0.1f, 0.2f, 0.3f, 0.4f);
        SDL_FColor sdl_fc = fc.to_sdl();
        fcolor fc2 = fcolor::from_sdl(sdl_fc);

        CHECK_EQ(fc.r, doctest::Approx(fc2.r));
        CHECK_EQ(fc.g, doctest::Approx(fc2.g));
        CHECK_EQ(fc.b, doctest::Approx(fc2.b));
        CHECK_EQ(fc.a, doctest::Approx(fc2.a));
    }

    TEST_CASE("color <-> fcolor conversions") {
        color c(255, 127, 0, 255);
        fcolor fc(c);  // Use conversion constructor
        color c2(fc);   // Use conversion constructor

        CHECK_EQ(c2.r, c.r);
        CHECK_EQ(c2.g, c.g);
        CHECK_EQ(c2.b, c.b);
        CHECK_EQ(c2.a, c.a);
    }

    TEST_CASE("pixel_format maps and unmaps correctly") {
        pixel_format fmt(pixel_format_enum::RGBA8888);

        color c(255, 128, 64, 255);
        Uint32 pixel = fmt.map_rgba(c);
        color c_back = fmt.get_rgba(pixel);

        CHECK((c_back.r >= 254 && c_back.r <= 255));
        CHECK((c_back.g >= 127 && c_back.g <= 128));
        CHECK((c_back.b >= 63 && c_back.b <= 64));
        CHECK(c_back.a == 255);
    }

    TEST_CASE("pixel_format RAII allocates and frees") {
        pixel_format fmt(pixel_format_enum::RGBA8888);
        CHECK(fmt.details != nullptr);
    }

    TEST_CASE("palette creation works") {
        auto pal_result = make_palette(4);
        REQUIRE(pal_result.has_value());

        auto& pal = pal_result.value();
        CHECK(pal != nullptr);
    }

    TEST_CASE("convert_pixels success returns expected") {
        // Test successful conversion
        int w = 10, h = 10;
        std::vector<char> src_buffer(static_cast<size_t>(w * h * 4), 0);
        std::vector<char> dst_buffer(static_cast<size_t>(w * h * 3));

        auto result = convert_pixels(w, h,
                                     SDL_PIXELFORMAT_RGBA8888, src_buffer.data(), w * 4,
                                     SDL_PIXELFORMAT_RGB24, dst_buffer.data(), w * 3);

        CHECK(result.has_value());
    }

}
