#include <doctest/doctest.h>

#include <sdlpp/video/surface.hh>
#include <sdlpp/video/surface_renderer.hh>

using namespace sdlpp;

namespace {
    [[nodiscard]] bool is_locked(const surface& s) {
        return (s.get()->flags & SDL_SURFACE_LOCKED) != 0;
    }

    [[nodiscard]] color pixel(const surface& s, int x, int y) {
        const auto c = s.get_pixel(x, y);
        REQUIRE(c.has_value());
        return *c;
    }
}

TEST_SUITE("surface_renderer") {
    TEST_CASE("draws reach the surface and leave it unlocked") {
        auto created = surface::create_rgb(4, 4, pixel_format_enum::RGBA8888);
        REQUIRE(created.has_value());
        auto& surf = *created;
        surface_renderer painter(surf);

        REQUIRE(painter.set_draw_color(color{255, 0, 0, 255}).has_value());
        REQUIRE(painter.clear().has_value());
        CHECK(pixel(surf, 3, 3).r == 255);

        REQUIRE(painter.set_draw_color(color{0, 0, 255, 255}).has_value());
        REQUIRE(painter.draw_point(point_i{1, 1}).has_value());
        CHECK(pixel(surf, 1, 1).b == 255);
        CHECK_FALSE(is_locked(surf));

        REQUIRE(painter.set_draw_color(color{0, 255, 0, 255}).has_value());
        REQUIRE(painter.fill_rect(rect_i{2, 2, 2, 2}).has_value());
        CHECK(pixel(surf, 3, 3).g == 255);
        CHECK(pixel(surf, 1, 1).b == 255);
        CHECK_FALSE(is_locked(surf));
    }
}
