//
// Tests for the typed coordinate spaces (coordinate_spaces.hh): the algebra, what does not
// compile, int and float, rounding, rectangles and layout.
//

#include <doctest/doctest.h>

#include <sdlpp/utility/coordinate_spaces.hh>

#include <concepts>
#include <type_traits>

using namespace sdlpp;

namespace {
    // What an expression would do, asked without compiling it.
    template<typename A, typename B>
    concept can_add = requires(A a, B b) { a + b; };

    template<typename A, typename B>
    concept can_subtract = requires(A a, B b) { a - b; };

    template<typename A, typename S>
    concept can_scale = requires(A a, S s) { a * s; };

    template<typename A, typename B>
    concept can_add_assign = requires(A a, B b) { a += b; };

    template<typename A, typename B>
    concept can_compare = requires(A a, B b) { a == b; };

    struct game_space {};                        // a space a library adds
    using game_pos = pos <game_space>;
    using game_delta = delta <game_space>;
}

// ---- what compiles and what does not ---------------------------------------------------------

// Only the operations that mean something.
static_assert(can_subtract <render_pos, render_pos>);
static_assert(std::same_as <decltype(render_pos{} - render_pos{}), render_delta>);
static_assert(std::same_as <decltype(render_pos{} + render_delta{}), render_pos>);
static_assert(std::same_as <decltype(render_delta{} + render_pos{}), render_pos>);
static_assert(std::same_as <decltype(render_pos{} - render_delta{}), render_pos>);
static_assert(std::same_as <decltype(render_delta{} * 2.0f), render_delta>);
static_assert(std::same_as <decltype(2.0f * render_delta{}), render_delta>);
static_assert(std::same_as <decltype(render_extent{} * 2.0f), render_extent>);
static_assert(std::same_as <decltype(render_rect{} + render_delta{}), render_rect>);

static_assert(!can_add <render_pos, render_pos>);          // two places do not add
static_assert(!can_scale <render_pos, float>);             // a place does not scale
static_assert(!can_add <render_extent, render_pos>);
static_assert(!can_add <render_rect, render_pos>);

// Spaces never mix.
static_assert(!can_subtract <render_pos, window_pos>);
static_assert(!can_add <render_pos, window_delta>);
static_assert(!can_add <window_delta, render_delta>);
static_assert(!can_compare <render_pos, window_pos>);
static_assert(!std::convertible_to <window_pos, render_pos>);
static_assert(!std::convertible_to <render_pos, window_pos>);
static_assert(!std::convertible_to <output_pos, render_pos>);
static_assert(!std::convertible_to <texel_rect, area <render_space, int>>);
static_assert(!can_subtract <game_pos, render_pos>);       // nor with a library's own

// Kinds never convert into each other.
static_assert(!std::convertible_to <render_pos, render_delta>);
static_assert(!std::convertible_to <render_delta, render_pos>);
static_assert(!std::convertible_to <render_extent, render_delta>);

// Int widens to float in the same space, implicitly; float never narrows.
static_assert(std::convertible_to <pos <render_space, int>, render_pos>);
static_assert(std::convertible_to <delta <render_space, int>, render_delta>);
static_assert(std::convertible_to <extent <render_space, int>, render_extent>);
static_assert(std::convertible_to <area <render_space, int>, render_rect>);
static_assert(!std::convertible_to <render_pos, pos <render_space, int>>);
static_assert(!std::convertible_to <render_rect, area <render_space, int>>);
static_assert(std::same_as <decltype(pos <render_space, int>{} - render_pos{}), render_delta>);
static_assert(std::same_as <decltype(texel_pos{} - texel_pos{}), texel_delta>);
static_assert(std::same_as <decltype(texel_delta{} * 2), texel_delta>);
static_assert(std::same_as <decltype(texel_delta{} * 0.5f), delta <texel_space, float>>);

// An int value does not take a float one in place.
static_assert(can_add_assign <render_pos, delta <render_space, int>>);
static_assert(!can_add_assign <pos <render_space, int>, render_delta>);
static_assert(!can_add_assign <delta <render_space, int>, render_delta>);

// The names in use have their space's value type.
static_assert(std::same_as <display_pos::value_type, int>);
static_assert(std::same_as <output_extent::value_type, int>);
static_assert(std::same_as <texel_rect::value_type, int>);
static_assert(std::same_as <window_pos::value_type, float>);
static_assert(std::same_as <render_rect::value_type, float>);
static_assert(std::same_as <normalized_pos::value_type, float>);
static_assert(std::same_as <render_pos::space, render_space>);

TEST_SUITE("sdlpp::coordinate_spaces") {
    TEST_CASE("the algebra: places, displacements and their arithmetic") {
        const render_pos a{10.0f, 20.0f};
        const render_pos b{4.0f, 5.0f};
        const render_delta d = a - b;
        CHECK(d == render_delta{6.0f, 15.0f});
        CHECK(b + d == a);
        CHECK(d + b == a);
        CHECK(a - d == b);
        CHECK(-d == render_delta{-6.0f, -15.0f});
        CHECK(d + d == render_delta{12.0f, 30.0f});
        CHECK(d - d == render_delta{});
        CHECK(d * 0.5f == render_delta{3.0f, 7.5f});
        CHECK(2.0f * d == render_delta{12.0f, 30.0f});

        render_pos p = b;
        p += d;
        CHECK(p == a);
        p -= d;
        CHECK(p == b);
        render_delta e = d;
        e += d;
        e -= render_delta{2.0f, 0.0f};
        e *= 0.5f;
        CHECK(e == render_delta{5.0f, 15.0f});
    }

    TEST_CASE("a delta's length and direction") {
        const window_delta d{3.0f, 4.0f};
        CHECK(d.length() == doctest::Approx(5.0f));
        CHECK(d.length_sq() == doctest::Approx(25.0f));
        const window_delta n = d.normalized();
        CHECK(n.x == doctest::Approx(0.6f));
        CHECK(n.y == doctest::Approx(0.8f));
        CHECK(window_delta{}.normalized() == window_delta{});    // no NaN
        CHECK(texel_delta{3, 4}.length_sq() == 25);
    }

    TEST_CASE("int and float of one space mix, and give float") {
        const pos <render_space, int> grid{3, 4};
        const render_pos widened = grid;                          // implicit
        CHECK(widened == render_pos{3.0f, 4.0f});
        CHECK(grid == render_pos{3.0f, 4.0f});
        CHECK(render_pos{3.5f, 4.0f} - grid == render_delta{0.5f, 0.0f});
        CHECK(grid + render_delta{0.25f, 0.5f} == render_pos{3.25f, 4.5f});
        CHECK(texel_delta{2, 3} * 2 == texel_delta{4, 6});
        CHECK(texel_delta{2, 3} * 0.5f == delta <texel_space, float>{1.0f, 1.5f});

        const area <render_space, int> cell{1, 2, 3, 4};
        const render_rect r = cell;
        CHECK(r == render_rect{1.0f, 2.0f, 3.0f, 4.0f});
    }

    TEST_CASE("float to int names its rounding") {
        const render_pos p{1.5f, -1.5f};
        CHECK(round_to_int(p) == pos <render_space, int>{2, -2});
        CHECK(floor_to_int(p) == pos <render_space, int>{1, -2});
        CHECK(ceil_to_int(p) == pos <render_space, int>{2, -1});
        CHECK(round_to_int(render_delta{0.4f, 0.6f}) == delta <render_space, int>{0, 1});
        CHECK(floor_to_int(render_extent{2.9f, 3.1f}) == extent <render_space, int>{2, 3});
        CHECK(ceil_to_int(render_extent{2.1f, 3.0f}) == extent <render_space, int>{3, 3});
    }

    TEST_CASE("a rounded rectangle rounds its corners, so neighbours still meet") {
        const render_rect left{0.0f, 0.0f, 10.4f, 5.0f};
        const render_rect right{10.4f, 0.0f, 10.4f, 5.0f};       // shares left's right edge
        const auto l = round_to_int(left);
        const auto r = round_to_int(right);
        CHECK(l.x + l.w == r.x);                                  // no gap, no overlap
        CHECK(r == area <render_space, int>{10, 0, 11, 5});       // 20.8 rounds to 21
    }

    TEST_CASE("rectangles: corners, size, containment and overlap") {
        const texel_rect r{texel_pos{2, 3}, texel_extent{4, 5}};
        CHECK(r == texel_rect{2, 3, 4, 5});
        CHECK(r.origin() == texel_pos{2, 3});
        CHECK(r.size() == texel_extent{4, 5});
        CHECK_FALSE(r.empty());
        CHECK(texel_rect{0, 0, 0, 5}.empty());
        CHECK(texel_extent{0, 1}.empty());

        CHECK(r.contains(texel_pos{2, 3}));                        // left and top edges: in
        CHECK(r.contains(texel_pos{5, 7}));
        CHECK_FALSE(r.contains(texel_pos{6, 3}));                  // right edge: out
        CHECK_FALSE(r.contains(texel_pos{2, 8}));                  // bottom edge: out
        CHECK(render_rect{0, 0, 1, 1}.contains(render_pos{0.999f, 0.0f}));
        CHECK_FALSE(render_rect{0, 0, 1, 1}.contains(render_pos{1.0f, 0.0f}));

        CHECK(r.intersects(texel_rect{5, 7, 3, 3}));
        CHECK_FALSE(r.intersects(texel_rect{6, 3, 3, 3}));         // only touching
        CHECK_FALSE(r.intersects(texel_rect{3, 4, 0, 1}));         // empty
        CHECK(r + texel_delta{1, -1} == texel_rect{3, 2, 4, 5});
        CHECK(r - texel_delta{1, -1} == texel_rect{1, 4, 4, 5});
    }

    TEST_CASE("a library's own space has the same algebra") {
        const game_pos a{1.0f, 2.0f};
        const game_delta d{0.5f, 0.5f};
        CHECK(a + d == game_pos{1.5f, 2.5f});
        CHECK((a + d) - a == d);
    }

    TEST_CASE("the float types are SDL's, field for field") {
        const render_rect r{1.0f, 2.0f, 3.0f, 4.0f};
        const auto* sdl = reinterpret_cast <const SDL_FRect*>(&r);
        CHECK(sdl->x == 1.0f);
        CHECK(sdl->y == 2.0f);
        CHECK(sdl->w == 3.0f);
        CHECK(sdl->h == 4.0f);
        const texel_pos p{5, 6};
        const auto* ip = reinterpret_cast <const SDL_Point*>(&p);
        CHECK(ip->x == 5);
        CHECK(ip->y == 6);
    }
}
