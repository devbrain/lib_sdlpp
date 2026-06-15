#include <doctest/doctest.h>
#include <sdlpp/system/random.hh>
#include <array>
#include <cstdint>
#include <limits>
#include <random>

TEST_SUITE("random") {
    TEST_CASE("engine satisfies uniform_random_bit_generator") {
        static_assert(std::uniform_random_bit_generator<sdlpp::random::engine>);

        CHECK(sdlpp::random::engine::min() == std::numeric_limits<std::uint32_t>::min());
        CHECK(sdlpp::random::engine::max() == std::numeric_limits<std::uint32_t>::max());
    }

    TEST_CASE("global generator can be seeded reproducibly") {
        sdlpp::random::seed(1234);
        const std::array first{
            sdlpp::random::bits(),
            sdlpp::random::bits(),
            sdlpp::random::bits()
        };

        sdlpp::random::seed(1234);
        CHECK(sdlpp::random::bits() == first[0]);
        CHECK(sdlpp::random::bits() == first[1]);
        CHECK(sdlpp::random::bits() == first[2]);
    }

    TEST_CASE("global helpers return values in requested ranges") {
        sdlpp::random::seed(5678);

        for (int i = 0; i < 100; ++i) {
            const auto die = sdlpp::random::uniform_int(6) + 1;
            CHECK(die >= 1);
            CHECK(die <= 6);

            const auto signed_value = sdlpp::random::uniform_int(-10, 10);
            CHECK(signed_value >= -10);
            CHECK(signed_value <= 10);

            const auto unsigned_value = sdlpp::random::uniform_int(10u, 20u);
            CHECK(unsigned_value >= 10u);
            CHECK(unsigned_value <= 20u);

            const auto real_value = sdlpp::random::uniform_real(-2.0, 4.0);
            CHECK(real_value >= -2.0);
            CHECK(real_value < 4.0);
        }
    }

    TEST_CASE("explicit engines are reproducible and independent") {
        sdlpp::random::engine a{42};
        sdlpp::random::engine b{42};

        for (int i = 0; i < 10; ++i) {
            CHECK(a() == b());
        }

        a.seed(42);
        b.seed(42);
        CHECK(a.uniform_int(100, 200) == b.uniform_int(100, 200));
    }

    TEST_CASE("explicit engine works with standard distributions") {
        sdlpp::random::engine engine{99};
        std::uniform_int_distribution<int> distribution{100, 200};

        for (int i = 0; i < 100; ++i) {
            const auto value = distribution(engine);
            CHECK(value >= 100);
            CHECK(value <= 200);
        }
    }

    TEST_CASE("explicit engine convenience helpers return requested ranges") {
        sdlpp::random::engine engine{2468};

        for (int i = 0; i < 100; ++i) {
            const auto index = engine.uniform_int(12);
            CHECK(index >= 0);
            CHECK(index < 12);

            const auto real_value = engine.uniform_real(10.0f, 11.0f);
            CHECK(real_value >= 10.0f);
            CHECK(real_value < 11.0f);
        }
    }

    TEST_CASE("geometry helpers create contained integer shapes") {
        sdlpp::random::engine engine{1357};
        const sdlpp::rect_i bounds{10, 20, 100, 80};

        for (int i = 0; i < 100; ++i) {
            const auto p = sdlpp::random::point_i(engine, bounds);
            CHECK(bounds.contains(p));

            const auto s = sdlpp::random::size_i(engine, sdlpp::size_i{4, 5}, sdlpp::size_i{20, 30});
            CHECK(s.width >= 4);
            CHECK(s.width <= 20);
            CHECK(s.height >= 5);
            CHECK(s.height <= 30);

            const auto r = sdlpp::random::rect_i(engine, bounds, sdlpp::size_i{5, 6}, sdlpp::size_i{25, 30});
            CHECK(bounds.contains(r));
            CHECK(r.w >= 5);
            CHECK(r.w <= 25);
            CHECK(r.h >= 6);
            CHECK(r.h <= 30);

            const auto l = sdlpp::random::line_i(engine, bounds);
            CHECK(bounds.contains(l.start()));
            CHECK(bounds.contains(l.end()));

            const auto c = sdlpp::random::circle_i(engine, bounds, 3, 12);
            CHECK(c.radius >= 3);
            CHECK(c.radius <= 12);
            CHECK(bounds.contains(c.bounding_rect()));

            const auto t = sdlpp::random::triangle_i(engine, bounds);
            CHECK(bounds.contains(t.a));
            CHECK(bounds.contains(t.b));
            CHECK(bounds.contains(t.c));
        }
    }

    TEST_CASE("geometry helpers create contained floating shapes") {
        sdlpp::random::engine engine{9753};
        const sdlpp::rect_f bounds{1.5f, 2.5f, 40.0f, 30.0f};

        for (int i = 0; i < 100; ++i) {
            const auto p = sdlpp::random::point_f(engine, bounds);
            CHECK(bounds.contains(p));

            const auto r = sdlpp::random::rect_f(engine, bounds, sdlpp::size_f{2.0f, 3.0f}, sdlpp::size_f{8.0f, 9.0f});
            CHECK(bounds.contains(r));
            CHECK(r.w >= 2.0f);
            CHECK(r.w < 8.0f);
            CHECK(r.h >= 3.0f);
            CHECK(r.h < 9.0f);

            const auto c = sdlpp::random::circle_f(engine, bounds, 1.0f, 5.0f);
            CHECK(c.radius >= 1.0f);
            CHECK(c.radius < 5.0f);
            CHECK(bounds.contains(c.bounding_rect()));
        }
    }

    TEST_CASE("geometry helpers are reproducible with explicit engines") {
        sdlpp::random::engine a{1122};
        sdlpp::random::engine b{1122};
        const sdlpp::rect_i bounds{0, 0, 640, 480};

        CHECK(sdlpp::random::point_i(a, bounds) == sdlpp::random::point_i(b, bounds));
        CHECK(sdlpp::random::line_i(a, bounds) == sdlpp::random::line_i(b, bounds));
        CHECK(sdlpp::random::rect_i(a, bounds, sdlpp::size_i{10, 10}, sdlpp::size_i{100, 100}) ==
              sdlpp::random::rect_i(b, bounds, sdlpp::size_i{10, 10}, sdlpp::size_i{100, 100}));
        CHECK(sdlpp::random::circle_i(a, bounds, 4, 24) == sdlpp::random::circle_i(b, bounds, 4, 24));
        CHECK(sdlpp::random::triangle_i(a, bounds) == sdlpp::random::triangle_i(b, bounds));
    }

    TEST_CASE("invalid ranges are enforced") {
        sdlpp::random::engine engine{3344};

        CHECK_THROWS([] { static_cast<void>(sdlpp::random::uniform_int(0)); }());
        CHECK_THROWS([&] { static_cast<void>(engine.uniform_int(0)); }());
        CHECK_THROWS([] { static_cast<void>(sdlpp::random::uniform_int(10, 1)); }());
        CHECK_THROWS([] { static_cast<void>(sdlpp::random::uniform_real(4.0f, 2.0f)); }());

        CHECK_THROWS([&] {
            static_cast<void>(sdlpp::random::point_i(engine, sdlpp::rect_i{0, 0, 0, 10}));
        }());
        CHECK_THROWS([&] {
            static_cast<void>(sdlpp::random::rect_i(
                engine,
                sdlpp::rect_i{0, 0, 10, 10},
                sdlpp::size_i{4, 4},
                sdlpp::size_i{20, 8}));
        }());
        CHECK_THROWS([&] {
            static_cast<void>(sdlpp::random::circle_i(engine, sdlpp::rect_i{0, 0, 10, 10}, 2, 8));
        }());
    }
}
