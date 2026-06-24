//
// Created by igor on 24/06/2026.
//
// Tests for the static collision grid scaffolding in <simplex/collide/dynamic/grid.hh>
// (Phase G0): the grid_coord sentinel/validity, the row-major grid_storage<T>, the
// physical->grid cell mapping and its inverse cell_box (resolution w*h over a physical
// extent [min,max], so the cell size is derived). The mappings are private, so they are
// reached through the grid_test_access friend tap -- the same pattern as the world tests.
//
// Not yet built: the cell->shape materialization and the region/raycast/cast queries.
//
#include <doctest/doctest.h>

#include <simplex/collide/dynamic/grid.hh>

namespace simplex::collide {
    // Friend tap (declared in grid.hh): reach the private mappings.
    struct grid_test_access {
        template <class T>
        static detail::grid_coord physical_to_grid(const grid<T>& g, const vec& v) {
            return g.physical_to_grid(v);
        }
        template <class T>
        static aabb cell_box(const grid<T>& g, const detail::grid_coord& c) {
            return g.cell_box(c);
        }
    };
}

using namespace simplex::collide;

TEST_SUITE("grid: grid_coord") {
    TEST_CASE("default-constructed is the invalid sentinel") {
        const detail::grid_coord c;
        CHECK_FALSE(static_cast<bool>(c));
        CHECK(c.x == detail::grid_coord::INVALID);
        CHECK(c.y == detail::grid_coord::INVALID);
    }

    TEST_CASE("a real coord is valid and carries its components") {
        const detail::grid_coord c{2, 5};
        CHECK(static_cast<bool>(c));
        CHECK(c.x == 2u);
        CHECK(c.y == 5u);
    }

    TEST_CASE("a coord with either component invalid is invalid") {
        CHECK_FALSE(static_cast<bool>(detail::grid_coord{detail::grid_coord::INVALID, 0}));
        CHECK_FALSE(static_cast<bool>(detail::grid_coord{0, detail::grid_coord::INVALID}));
    }
}

TEST_SUITE("grid: grid_storage") {
    TEST_CASE("reports its dimensions") {
        const detail::grid_storage<int> s(3, 5);
        CHECK(s.get_width() == 3u);
        CHECK(s.get_height() == 5u);
    }

    TEST_CASE("write then read round-trips per cell") {
        detail::grid_storage<int> s(4, 3);
        s[detail::grid_coord{0, 0}] = 11;
        s[detail::grid_coord{3, 2}] = 99;   // opposite corner
        s[detail::grid_coord{1, 2}] = 42;
        CHECK(s[detail::grid_coord{0, 0}] == 11);
        CHECK(s[detail::grid_coord{3, 2}] == 99);
        CHECK(s[detail::grid_coord{1, 2}] == 42);
    }

    TEST_CASE("cells are independent; (x,y) and (y,x) are distinct cells (row-major)") {
        detail::grid_storage<int> s(3, 3);
        for (uint32_t y = 0; y < 3; ++y) {
            for (uint32_t x = 0; x < 3; ++x) {
                s[detail::grid_coord{x, y}] = static_cast<int>(y * 3 + x); // = the flat index
            }
        }
        // (1,0) and (0,1) must be different cells -> different stored values.
        CHECK(s[detail::grid_coord{1, 0}] == 1);
        CHECK(s[detail::grid_coord{0, 1}] == 3);
        CHECK(s[detail::grid_coord{2, 2}] == 8);
    }
}

TEST_SUITE("grid: physical_to_grid mapping") {
    TEST_CASE("square cells: 4x4 over [0,8]^2 -> 2x2 cells") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        auto cell = [&](float x, float y) { return grid_test_access::physical_to_grid(g, vec{x, y}); };

        CHECK(cell(0.0f, 0.0f).x == 0u);   CHECK(cell(0.0f, 0.0f).y == 0u);   // min corner
        CHECK(cell(1.0f, 1.0f).x == 0u);   CHECK(cell(1.0f, 1.0f).y == 0u);   // inside cell 0
        CHECK(cell(2.0f, 2.0f).x == 1u);   CHECK(cell(2.0f, 2.0f).y == 1u);   // on a boundary -> next
        CHECK(cell(5.0f, 3.0f).x == 2u);   CHECK(cell(5.0f, 3.0f).y == 1u);
        CHECK(cell(7.9f, 7.9f).x == 3u);   CHECK(cell(7.9f, 7.9f).y == 3u);   // last cell
    }

    TEST_CASE("the exact max corner is clamped into the last cell, not out of range") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        const auto c = grid_test_access::physical_to_grid(g, vec{8.0f, 8.0f});
        REQUIRE(static_cast<bool>(c));
        CHECK(c.x == 3u);   // not 4 (which would be OOB)
        CHECK(c.y == 3u);
    }

    TEST_CASE("points outside the physical bounds map to the invalid sentinel") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        CHECK_FALSE(static_cast<bool>(grid_test_access::physical_to_grid(g, vec{-0.1f, 4.0f})));
        CHECK_FALSE(static_cast<bool>(grid_test_access::physical_to_grid(g, vec{4.0f, -0.1f})));
        CHECK_FALSE(static_cast<bool>(grid_test_access::physical_to_grid(g, vec{8.1f, 4.0f})));
        CHECK_FALSE(static_cast<bool>(grid_test_access::physical_to_grid(g, vec{4.0f, 8.1f})));
    }

    TEST_CASE("non-origin bounds: extent is measured from grid_min") {
        grid<int> g(2, 2, vec{-10, -10}, vec{-6, -6}); // 2x2 over a 4x4 region -> cell 2x2
        auto cell = [&](float x, float y) { return grid_test_access::physical_to_grid(g, vec{x, y}); };
        CHECK(cell(-10.0f, -10.0f).x == 0u); CHECK(cell(-10.0f, -10.0f).y == 0u);
        CHECK(cell(-7.0f, -9.0f).x == 1u);   CHECK(cell(-7.0f, -9.0f).y == 0u);
        CHECK(cell(-6.0f, -6.0f).x == 1u);   CHECK(cell(-6.0f, -6.0f).y == 1u); // max corner clamped
    }

    TEST_CASE("rectangular (non-square) cells: 2x4 over [0,8]^2 -> cells 4 wide, 2 tall") {
        grid<int> g(2, 4, vec{0, 0}, vec{8, 8}); // cell_dim = (8/2, 8/4) = (4, 2)
        auto cell = [&](float x, float y) { return grid_test_access::physical_to_grid(g, vec{x, y}); };
        CHECK(cell(1.0f, 1.0f).x == 0u);   CHECK(cell(1.0f, 1.0f).y == 0u);
        CHECK(cell(5.0f, 3.0f).x == 1u);   CHECK(cell(5.0f, 3.0f).y == 1u);   // x: 5/4=1, y: 3/2=1
        CHECK(cell(7.0f, 7.0f).x == 1u);   CHECK(cell(7.0f, 7.0f).y == 3u);   // x: 7/4=1, y: 7/2=3
    }
}

TEST_SUITE("grid: cell_box") {
    // exact-corner box equality (cell_box only adds/multiplies exact cell dims, so bit-exact
    // for representable inputs).
    auto box_eq = [](const aabb& b, float minx, float miny, float maxx, float maxy) {
        return b.min.x() == minx && b.min.y() == miny && b.max.x() == maxx && b.max.y() == maxy;
    };

    TEST_CASE("square cells: world AABB of a cell (4x4 over [0,8]^2 -> 2x2 cells)") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        CHECK(box_eq(grid_test_access::cell_box(g, {0, 0}), 0.0f, 0.0f, 2.0f, 2.0f)); // min corner cell
        CHECK(box_eq(grid_test_access::cell_box(g, {2, 1}), 4.0f, 2.0f, 6.0f, 4.0f));
        CHECK(box_eq(grid_test_access::cell_box(g, {3, 3}), 6.0f, 6.0f, 8.0f, 8.0f)); // last cell hits max
    }

    TEST_CASE("non-origin bounds: boxes are offset from grid_min") {
        grid<int> g(2, 2, vec{-10, -10}, vec{-6, -6}); // cell 2x2
        CHECK(box_eq(grid_test_access::cell_box(g, {0, 0}), -10.0f, -10.0f, -8.0f, -8.0f));
        CHECK(box_eq(grid_test_access::cell_box(g, {1, 1}), -8.0f, -8.0f, -6.0f, -6.0f));
    }

    TEST_CASE("rectangular cells: 2x4 over [0,8]^2 -> cells 4 wide, 2 tall") {
        grid<int> g(2, 4, vec{0, 0}, vec{8, 8}); // cell_dim = (4, 2)
        CHECK(box_eq(grid_test_access::cell_box(g, {0, 0}), 0.0f, 0.0f, 4.0f, 2.0f));
        CHECK(box_eq(grid_test_access::cell_box(g, {1, 3}), 4.0f, 6.0f, 8.0f, 8.0f)); // far cell hits max
    }

    TEST_CASE("adjacent cells tile contiguously (shared edge, no gap/overlap)") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        const aabb a = grid_test_access::cell_box(g, {0, 0});
        const aabb right = grid_test_access::cell_box(g, {1, 0});
        const aabb below = grid_test_access::cell_box(g, {0, 1});
        CHECK(a.max.x() == right.min.x()); // x-adjacent cells share the vertical edge
        CHECK(a.max.y() == below.min.y()); // y-adjacent cells share the horizontal edge
    }

    TEST_CASE("round-trip: a cell's corner (and center) map back to that cell") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        for (uint32_t y = 0; y < 4; ++y) {
            for (uint32_t x = 0; x < 4; ++x) {
                const detail::grid_coord c{x, y};
                const aabb b = grid_test_access::cell_box(g, c);
                const auto from_min = grid_test_access::physical_to_grid(g, b.min);
                CHECK(from_min.x == x);
                CHECK(from_min.y == y);
                const vec center{(b.min.x() + b.max.x()) * 0.5f, (b.min.y() + b.max.y()) * 0.5f};
                const auto from_center = grid_test_access::physical_to_grid(g, center);
                CHECK(from_center.x == x);
                CHECK(from_center.y == y);
            }
        }
    }
}
