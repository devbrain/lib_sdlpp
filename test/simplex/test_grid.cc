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

#include <algorithm>
#include <vector>

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

    TEST_CASE("a cell is empty until set; get returns nullptr") {
        detail::grid_storage<int> s(4, 3);
        CHECK(s.get(detail::grid_coord{0, 0}) == nullptr);   // nothing set yet
        s.set(detail::grid_coord{0, 0}, 11);
        const int* p = s.get(detail::grid_coord{0, 0});
        REQUIRE(p != nullptr);
        CHECK(*p == 11);
        CHECK(s.get(detail::grid_coord{1, 0}) == nullptr);   // a neighbour stays empty
    }

    TEST_CASE("set then get round-trips per cell; overwrite updates in place") {
        detail::grid_storage<int> s(4, 3);
        s.set(detail::grid_coord{0, 0}, 11);
        s.set(detail::grid_coord{3, 2}, 99);   // opposite corner
        s.set(detail::grid_coord{1, 2}, 42);
        CHECK(*s.get(detail::grid_coord{0, 0}) == 11);
        CHECK(*s.get(detail::grid_coord{3, 2}) == 99);
        CHECK(*s.get(detail::grid_coord{1, 2}) == 42);

        s.set(detail::grid_coord{0, 0}, 7);    // overwrite an occupied cell
        CHECK(*s.get(detail::grid_coord{0, 0}) == 7);
    }

    TEST_CASE("cells are independent; (x,y) and (y,x) are distinct cells (row-major)") {
        detail::grid_storage<int> s(3, 3);
        for (uint32_t y = 0; y < 3; ++y) {
            for (uint32_t x = 0; x < 3; ++x) {
                s.set(detail::grid_coord{x, y}, static_cast<int>(y * 3 + x)); // = the flat index
            }
        }
        // (1,0) and (0,1) must be different cells -> different stored values.
        CHECK(*s.get(detail::grid_coord{1, 0}) == 1);
        CHECK(*s.get(detail::grid_coord{0, 1}) == 3);
        CHECK(*s.get(detail::grid_coord{2, 2}) == 8);
    }

    TEST_CASE("clear empties a cell; double-clear and clear-empty are safe no-ops") {
        detail::grid_storage<int> s(4, 3);
        s.set(detail::grid_coord{2, 1}, 55);
        REQUIRE(s.get(detail::grid_coord{2, 1}) != nullptr);
        s.clear(detail::grid_coord{2, 1});
        CHECK(s.get(detail::grid_coord{2, 1}) == nullptr);   // now empty
        // these must not corrupt the free list (the clear-empty / double-clear guard)
        CHECK_NOTHROW(s.clear(detail::grid_coord{2, 1}));    // already empty
        CHECK_NOTHROW(s.clear(detail::grid_coord{0, 0}));    // never set
    }

    TEST_CASE("a cleared slot is reused by a later set, with the new value") {
        detail::grid_storage<int> s(4, 3);
        s.set(detail::grid_coord{0, 0}, 1);
        s.set(detail::grid_coord{1, 0}, 2);
        s.clear(detail::grid_coord{0, 0});                   // frees a slot
        s.set(detail::grid_coord{2, 0}, 3);                  // should reuse the freed slot
        // all live cells read back correctly (no aliasing from slot reuse)
        CHECK(s.get(detail::grid_coord{0, 0}) == nullptr);
        CHECK(*s.get(detail::grid_coord{1, 0}) == 2);
        CHECK(*s.get(detail::grid_coord{2, 0}) == 3);
    }

    TEST_CASE("get out of range returns nullptr (no OOB)") {
        const detail::grid_storage<int> s(4, 3);
        CHECK(s.get(detail::grid_coord{4, 0}) == nullptr);   // x == width
        CHECK(s.get(detail::grid_coord{0, 3}) == nullptr);   // y == height
        CHECK(s.get(detail::grid_coord{999, 999}) == nullptr);
        CHECK(s.get(detail::grid_coord{}) == nullptr);       // invalid sentinel
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

TEST_SUITE("grid: physical API (set/get/clear/reset)") {
    TEST_CASE("set then get by world position; empty / out-of-bounds read as nullptr") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8}); // cell 2x2
        g.set(vec{1, 1}, 11);                     // cell (0,0)
        g.set(vec{5, 3}, 42);                     // cell (2,1)
        REQUIRE(g.get(vec{1, 1}) != nullptr);
        CHECK(*g.get(vec{1, 1}) == 11);
        CHECK(*g.get(vec{5, 3}) == 42);
        CHECK(*g.get(vec{0.5f, 0.5f}) == 11);    // same cell (0,0) -> same value
        CHECK(g.get(vec{7, 7}) == nullptr);      // empty cell
        CHECK(g.get(vec{-1, 0}) == nullptr);     // out of bounds -> tolerant nullptr
        CHECK(g.get(vec{100, 100}) == nullptr);
    }

    TEST_CASE("set overwrites the cell that contains the position") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        g.set(vec{1, 1}, 1);
        g.set(vec{1.5f, 0.5f}, 9); // same cell (0,0)
        CHECK(*g.get(vec{0, 0}) == 9);
    }

    TEST_CASE("clear by position empties the cell; reset empties everything") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        g.set(vec{1, 1}, 1);
        g.set(vec{5, 5}, 2);
        g.clear(vec{1, 1});
        CHECK(g.get(vec{1, 1}) == nullptr);
        CHECK(*g.get(vec{5, 5}) == 2);           // unaffected
        CHECK_NOTHROW(g.clear(vec{1, 1}));       // clear-empty no-op
        CHECK_NOTHROW(g.clear(vec{200, 0}));     // clear out-of-bounds no-op
        g.reset();
        CHECK(g.get(vec{5, 5}) == nullptr);      // all gone
    }

    TEST_CASE("works with a non-default-constructible payload") {
        struct body { int v; explicit body(int x) : v(x) {} };
        grid<body> g(2, 2, vec{0, 0}, vec{4, 4});
        g.set(vec{1, 1}, 7);                      // emplace body(7)
        REQUIRE(g.get(vec{1, 1}) != nullptr);
        CHECK(g.get(vec{1, 1})->v == 7);
    }
}

TEST_SUITE("grid: query (region enumeration)") {
    // helper: collect (value, cell_box) pairs a query yields
    struct hit { int v; aabb box; };

    TEST_CASE("enumerates occupied cells overlapping the region, with their cell_box") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8}); // cell 2x2
        g.set(vec{1, 1}, 1);   // cell (0,0) box [0,0]..[2,2]
        g.set(vec{3, 1}, 2);   // cell (1,0) box [2,0]..[4,2]
        g.set(vec{5, 5}, 3);   // cell (2,2) box [4,4]..[6,6]  (outside the query box below)

        std::vector<hit> hits;
        g.query(aabb{{0, 0}, {3, 3}}, [&](const int& v, const aabb& b) { hits.push_back({v, b}); });

        // the region [0,3]^2 covers cells (0,0) and (1,0) (and (0,1)/(1,1) which are empty)
        REQUIRE(hits.size() == 2);
        std::sort(hits.begin(), hits.end(), [](const hit& a, const hit& b) { return a.v < b.v; });
        CHECK(hits[0].v == 1);
        CHECK(hits[0].box.min.x() == 0.0f); CHECK(hits[0].box.max.x() == 2.0f); // cell (0,0) box
        CHECK(hits[1].v == 2);
        CHECK(hits[1].box.min.x() == 2.0f); CHECK(hits[1].box.max.x() == 4.0f); // cell (1,0) box
    }

    TEST_CASE("skips empty cells") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        g.set(vec{1, 1}, 5); // only one occupied cell
        int count = 0;
        g.query(aabb{{0, 0}, {8, 8}}, [&](const int&, const aabb&) { ++count; });
        CHECK(count == 1); // whole grid queried, but only the one occupied cell reported
    }

    TEST_CASE("a region overlapping the grid edge is clipped (no OOB, no throw)") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        g.set(vec{7, 7}, 9);                      // last cell (3,3)
        int count = 0;
        // region pokes past the max corner -> clipped to the grid
        CHECK_NOTHROW(g.query(aabb{{6, 6}, {100, 100}}, [&](const int&, const aabb&) { ++count; }));
        CHECK(count == 1);
    }

    TEST_CASE("a region entirely outside the grid is a tolerant no-op") {
        grid<int> g(4, 4, vec{0, 0}, vec{8, 8});
        g.set(vec{1, 1}, 1);
        int count = 0;
        CHECK_NOTHROW(g.query(aabb{{100, 100}, {110, 110}}, [&](const int&, const aabb&) { ++count; }));
        CHECK(count == 0);
    }
}
