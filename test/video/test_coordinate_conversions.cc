//
// Tests for the typed coordinate conversions on renderer and window (coordinate_spaces.hh):
// window <-> render, output <-> render, normalized and display -> window.
//

#include <doctest/doctest.h>

#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/video/window.hh>

using namespace sdlpp;

namespace {
    // A 640 x 400 window drawn at a 320 x 100 logical size, letterboxed: scale 2, with 100
    // window units of bars above and below the 200-unit-high picture.
    constexpr int WINDOW_W = 640;
    constexpr int WINDOW_H = 400;
    constexpr int LOGICAL_W = 320;
    constexpr int LOGICAL_H = 100;
}

TEST_SUITE("sdlpp::coordinate conversions") {
    TEST_CASE("window <-> render positions follow logical presentation") {
        auto w = window::create("coordinates", WINDOW_W, WINDOW_H);
        REQUIRE(w.has_value());
        auto r = renderer::create(*w);
        REQUIRE(r.has_value());

        const auto plain = r->to_render(window_pos{10.0f, 20.0f});  // no presentation: identity
        REQUIRE(plain.has_value());
        CHECK(*plain == render_pos{10.0f, 20.0f});

        REQUIRE(r->set_logical_presentation(LOGICAL_W, LOGICAL_H, logical_presentation::letterbox));
        const auto top_left = r->to_render(window_pos{0.0f, 100.0f}); // the picture's corner
        REQUIRE(top_left.has_value());
        CHECK(top_left->x == doctest::Approx(0.0f));
        CHECK(top_left->y == doctest::Approx(0.0f));
        const auto centre = r->to_render(window_pos{320.0f, 200.0f});
        REQUIRE(centre.has_value());
        CHECK(centre->x == doctest::Approx(160.0f));
        CHECK(centre->y == doctest::Approx(50.0f));

        const auto back = r->to_window(*centre);
        REQUIRE(back.has_value());
        CHECK(back->x == doctest::Approx(320.0f));
        CHECK(back->y == doctest::Approx(200.0f));
    }

    TEST_CASE("a window delta converts with the scale only, without the letterbox offset") {
        auto w = window::create("coordinates delta", WINDOW_W, WINDOW_H);
        REQUIRE(w.has_value());
        auto r = renderer::create(*w);
        REQUIRE(r.has_value());
        REQUIRE(r->set_logical_presentation(LOGICAL_W, LOGICAL_H, logical_presentation::letterbox));

        const auto d = r->to_render(window_delta{10.0f, 10.0f});
        REQUIRE(d.has_value());
        CHECK(d->x == doctest::Approx(5.0f));                    // halved, not offset by 100
        CHECK(d->y == doctest::Approx(5.0f));

        const auto as_point = r->to_render(window_pos{10.0f, 10.0f});
        REQUIRE(as_point.has_value());
        CHECK(as_point->y == doctest::Approx(-45.0f));          // what the bug would give

        const auto back = r->to_window(*d);
        REQUIRE(back.has_value());
        CHECK(back->x == doctest::Approx(10.0f));
        CHECK(back->y == doctest::Approx(10.0f));
    }

    TEST_CASE("output pixels <-> render positions, and the presentation rectangle") {
        auto w = window::create("coordinates output", WINDOW_W, WINDOW_H);
        REQUIRE(w.has_value());
        auto r = renderer::create(*w);
        REQUIRE(r.has_value());
        REQUIRE(r->set_logical_presentation(LOGICAL_W, LOGICAL_H, logical_presentation::letterbox));

        const auto rect = r->get_logical_presentation_rect();
        REQUIRE(rect.has_value());
        const auto picture = r->to_render(rect->origin());      // where the picture starts
        REQUIRE(picture.has_value());
        CHECK(picture->x == doctest::Approx(0.0f));
        CHECK(picture->y == doctest::Approx(0.0f));

        const auto from_int = r->to_render(output_pos{0, 0});    // int output pixels widen
        REQUIRE(from_int.has_value());
        const auto out = r->to_output(render_pos{160.0f, 50.0f});
        REQUIRE(out.has_value());
        const auto round_trip = r->to_render(*out);
        REQUIRE(round_trip.has_value());
        CHECK(round_trip->x == doctest::Approx(160.0f));
        CHECK(round_trip->y == doctest::Approx(50.0f));
    }

    TEST_CASE("output conversions need a window") {
        auto s = surface::create_rgb(16, 16, pixel_format_enum::RGBA8888);
        REQUIRE(s.has_value());
        auto r = renderer::create_software(s->get());
        REQUIRE(r.has_value());
        CHECK_FALSE(r->to_render(output_pos{1, 1}).has_value());
        CHECK_FALSE(r->to_output(render_pos{1.0f, 1.0f}).has_value());
    }

    TEST_CASE("an empty renderer or window converts nothing") {
        const renderer r;
        CHECK_FALSE(r.to_render(window_pos{}).has_value());
        CHECK_FALSE(r.to_render(window_delta{}).has_value());
        CHECK_FALSE(r.to_window(render_pos{}).has_value());
        CHECK_FALSE(r.to_window(render_delta{}).has_value());
        CHECK_FALSE(r.get_logical_presentation_rect().has_value());
        const window w;
        CHECK_FALSE(w.to_window(normalized_pos{}).has_value());
        CHECK_FALSE(w.to_window(display_pos{}).has_value());
        CHECK_FALSE(w.to_display(window_pos{}).has_value());
    }

    TEST_CASE("normalized touch positions scale by the window's size") {
        auto w = window::create("coordinates touch", WINDOW_W, WINDOW_H);
        REQUIRE(w.has_value());
        const auto p = w->to_window(normalized_pos{0.5f, 0.25f});
        REQUIRE(p.has_value());
        CHECK(*p == window_pos{320.0f, 100.0f});
        const auto d = w->to_window(normalized_delta{0.1f, 0.1f});
        REQUIRE(d.has_value());
        CHECK(d->x == doctest::Approx(64.0f));
        CHECK(d->y == doctest::Approx(40.0f));
    }

    TEST_CASE("desktop positions are relative to the window's corner") {
        auto w = window::create("coordinates display", WINDOW_W, WINDOW_H);
        REQUIRE(w.has_value());
        const auto corner = w->to_display(window_pos{});
        REQUIRE(corner.has_value());
        const auto p = w->to_window(display_pos{round_to_int(*corner).x + 7, round_to_int(*corner).y + 9});
        REQUIRE(p.has_value());
        CHECK(p->x == doctest::Approx(7.0f));
        CHECK(p->y == doctest::Approx(9.0f));
        const auto back = w->to_display(*p);
        REQUIRE(back.has_value());
        CHECK(*back - *corner == delta <display_space, float>{7.0f, 9.0f});
    }
}
