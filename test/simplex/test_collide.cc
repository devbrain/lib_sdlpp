//
// Unit tests for simplex::collide — pure world-space collision math.
//
// Covers the full public surface of <simplex/collide/shapes.hh>: containment,
// static overlap tests, the slab AABB/segment sweep, closest-point / squared-
// distance queries, circle/segment and segment/segment intersection, and the
// three continuous-collision-detection (CCD) overloads. Also pins the edge
// cases surfaced in review: no NaN for zero-duration sweeps that start
// overlapped, point-on-segment for degenerate segments, numerically stable
// point-to-segment distance, scale-consistent degeneracy thresholds, tangent
// circle hits, deterministic collinear normals, and convex exit selection.
//

#include <doctest/doctest.h>
#include <cmath>

#include <simplex/collide/shapes.hh>
#include <simplex/collide/bridge.hh>

using namespace simplex::collide;

namespace {
    bool vapprox(const vec& a, const vec& b, float eps = 1e-4f) {
        return std::fabs(a.x() - b.x()) <= eps && std::fabs(a.y() - b.y()) <= eps;
    }

    bool is_unit(const vec& v, float eps = 1e-4f) {
        return std::fabs(std::sqrt(v.x() * v.x() + v.y() * v.y()) - 1.0f) <= eps;
    }

    bool finite(float f) { return std::isfinite(f); }

    bool finite(const entry_exit_times& e) {
        return finite(e.entry) && finite(e.exit) &&
               finite(e.entry_normal.x()) && finite(e.entry_normal.y()) &&
               finite(e.exit_normal.x()) && finite(e.exit_normal.y());
    }
}

TEST_SUITE("simplex::collide") {
    // ------------------------------------------------------------------
    // Basic shape helpers
    // ------------------------------------------------------------------
    TEST_CASE("aabb size and center") {
        aabb b{{1.0f, 2.0f}, {5.0f, 8.0f}};
        CHECK(vapprox(b.size(), vec{4.0f, 6.0f}));
        CHECK(vapprox(b.center(), vec{3.0f, 5.0f}));
    }

    TEST_CASE("segment point_in_time") {
        segment s{{0.0f, 0.0f}, {10.0f, 4.0f}};
        CHECK(vapprox(s.point_in_time(0.0f), vec{0.0f, 0.0f}));
        CHECK(vapprox(s.point_in_time(1.0f), vec{10.0f, 4.0f}));
        CHECK(vapprox(s.point_in_time(0.5f), vec{5.0f, 2.0f}));
        // Extrapolation beyond [0,1] is allowed.
        CHECK(vapprox(s.point_in_time(2.0f), vec{20.0f, 8.0f}));
    }

    // ------------------------------------------------------------------
    // Containment
    // ------------------------------------------------------------------
    TEST_CASE("contains(aabb, point)") {
        aabb b{{0.0f, 0.0f}, {4.0f, 4.0f}};
        CHECK(contains(b, vec{2.0f, 2.0f}));   // interior
        CHECK(contains(b, vec{0.0f, 0.0f}));   // corner (boundary)
        CHECK(contains(b, vec{4.0f, 2.0f}));   // edge (boundary)
        CHECK_FALSE(contains(b, vec{4.001f, 2.0f}));
        CHECK_FALSE(contains(b, vec{-0.001f, 2.0f}));
    }

    TEST_CASE("contains(circle, point)") {
        circle c{{0.0f, 0.0f}, 2.0f};
        CHECK(contains(c, vec{0.0f, 0.0f}));   // center
        CHECK(contains(c, vec{2.0f, 0.0f}));   // on boundary
        CHECK(contains(c, vec{1.0f, 1.0f}));   // interior
        CHECK_FALSE(contains(c, vec{2.0f, 2.0f}));
    }

    // ------------------------------------------------------------------
    // Static overlap tests
    // ------------------------------------------------------------------
    TEST_CASE("intersects(aabb, aabb)") {
        aabb a{{0.0f, 0.0f}, {2.0f, 2.0f}};
        CHECK(intersects(a, aabb{{1.0f, 1.0f}, {3.0f, 3.0f}}));   // overlap
        CHECK(intersects(a, aabb{{2.0f, 0.0f}, {4.0f, 2.0f}}));   // edge touch
        CHECK_FALSE(intersects(a, aabb{{2.001f, 0.0f}, {4.0f, 2.0f}}));
        CHECK_FALSE(intersects(a, aabb{{5.0f, 5.0f}, {6.0f, 6.0f}}));
    }

    TEST_CASE("intersects(circle, circle)") {
        circle a{{0.0f, 0.0f}, 1.0f};
        CHECK(intersects(a, circle{{1.0f, 0.0f}, 1.0f}));   // overlap
        CHECK(intersects(a, circle{{2.0f, 0.0f}, 1.0f}));   // exact touch (r1+r2 == d)
        CHECK_FALSE(intersects(a, circle{{2.001f, 0.0f}, 1.0f}));
    }

    TEST_CASE("intersects(circle, aabb) static") {
        circle c{{0.0f, 0.0f}, 1.0f};
        CHECK(intersects(c, aabb{{0.5f, 0.0f}, {3.0f, 3.0f}}));    // center near box
        CHECK(intersects(c, aabb{{1.0f, -1.0f}, {3.0f, 1.0f}}));   // touch left face
        CHECK_FALSE(intersects(c, aabb{{2.0f, 2.0f}, {3.0f, 3.0f}}));   // far corner
    }

    // ------------------------------------------------------------------
    // line_intersects / segment_intersects predicates
    // ------------------------------------------------------------------
    TEST_CASE("line / segment interval predicates") {
        CHECK(line_intersects(entry_exit_times{0.2f, 0.8f, {}, {}}));
        CHECK_FALSE(line_intersects(entry_exit_times{0.8f, 0.2f, {}, {}}));   // entry > exit

        CHECK(segment_intersects(entry_exit_times{0.2f, 0.8f, {}, {}}));
        CHECK(segment_intersects(entry_exit_times{-0.3f, 0.5f, {}, {}}));     // starts inside
        CHECK_FALSE(segment_intersects(entry_exit_times{1.2f, 1.5f, {}, {}})); // entry past end
        CHECK_FALSE(segment_intersects(entry_exit_times{-0.9f, -0.2f, {}, {}}));// exit before start
    }

    // ------------------------------------------------------------------
    // Slab method: intersect_times(aabb, segment)
    // ------------------------------------------------------------------
    TEST_CASE("intersect_times(aabb, segment)") {
        aabb box{{0.0f, 0.0f}, {2.0f, 2.0f}};

        SUBCASE("horizontal crossing through the middle") {
            segment s{{-1.0f, 1.0f}, {3.0f, 1.0f}};   // dx = 4
            auto e = intersect_times(box, s);
            CHECK(e.entry == doctest::Approx(0.25f));
            CHECK(e.exit == doctest::Approx(0.75f));
            CHECK(vapprox(e.entry_normal, vec{-1.0f, 0.0f}));
            CHECK(vapprox(e.exit_normal, vec{1.0f, 0.0f}));
            CHECK(segment_intersects(e));
        }

        SUBCASE("vertical segment inside the x-slab (dx == 0)") {
            segment s{{1.0f, -1.0f}, {1.0f, 3.0f}};   // dx = 0, x in [0,2]
            auto e = intersect_times(box, s);
            CHECK(e.entry == doctest::Approx(0.25f));
            CHECK(e.exit == doctest::Approx(0.75f));
            CHECK(segment_intersects(e));
        }

        SUBCASE("vertical segment outside the x-slab misses") {
            segment s{{5.0f, -1.0f}, {5.0f, 3.0f}};   // dx = 0, x outside [0,2]
            auto e = intersect_times(box, s);
            CHECK_FALSE(line_intersects(e));
        }

        SUBCASE("segment starting inside the box yields entry < 0") {
            segment s{{1.0f, 1.0f}, {5.0f, 1.0f}};
            auto e = intersect_times(box, s);
            CHECK(e.entry < 0.0f);
            CHECK(e.exit > 0.0f);
            CHECK(segment_intersects(e));
        }

        SUBCASE("parallel miss above the box") {
            segment s{{-1.0f, 5.0f}, {3.0f, 5.0f}};   // dy = 0, y outside [0,2]
            auto e = intersect_times(box, s);
            CHECK_FALSE(line_intersects(e));
        }
    }

    // ------------------------------------------------------------------
    // Closest point / parameter
    // ------------------------------------------------------------------
    TEST_CASE("closest_parameter / closest_point(segment)") {
        segment s{{0.0f, 0.0f}, {10.0f, 0.0f}};
        CHECK(closest_parameter(vec{5.0f, 5.0f}, s) == doctest::Approx(0.5f));
        CHECK(closest_parameter(vec{-5.0f, 0.0f}, s) == doctest::Approx(0.0f));   // clamp low
        CHECK(closest_parameter(vec{15.0f, 0.0f}, s) == doctest::Approx(1.0f));   // clamp high
        CHECK(vapprox(closest_point(vec{5.0f, 5.0f}, s), vec{5.0f, 0.0f}));
        CHECK(vapprox(closest_point(vec{-5.0f, 9.0f}, s), vec{0.0f, 0.0f}));
    }

    TEST_CASE("closest_parameter on a short (non-degenerate) segment") {
        // 0.01 units long — well above the 1e-6 degeneracy threshold; must NOT
        // collapse to parameter 0.
        segment s{{0.0f, 0.0f}, {0.01f, 0.0f}};
        CHECK(closest_parameter(vec{0.005f, 0.0f}, s) == doctest::Approx(0.5f));
    }

    TEST_CASE("closest_parameter on a truly degenerate segment") {
        segment s{{3.0f, 3.0f}, {3.0f, 3.0f}};
        CHECK(closest_parameter(vec{9.0f, 9.0f}, s) == doctest::Approx(0.0f));
    }

    TEST_CASE("closest_point(aabb)") {
        aabb b{{0.0f, 0.0f}, {4.0f, 4.0f}};
        CHECK(vapprox(closest_point(vec{2.0f, 2.0f}, b), vec{2.0f, 2.0f}));   // inside
        CHECK(vapprox(closest_point(vec{-1.0f, 2.0f}, b), vec{0.0f, 2.0f}));  // left
        CHECK(vapprox(closest_point(vec{6.0f, 6.0f}, b), vec{4.0f, 4.0f}));   // corner
    }

    TEST_CASE("closest_point(circle)") {
        circle c{{0.0f, 0.0f}, 2.0f};
        CHECK(vapprox(closest_point(vec{1.0f, 0.0f}, c), vec{1.0f, 0.0f}));   // inside -> itself
        CHECK(vapprox(closest_point(vec{4.0f, 0.0f}, c), vec{2.0f, 0.0f}));   // project to rim
        CHECK(vapprox(closest_point(vec{0.0f, -5.0f}, c), vec{0.0f, -2.0f}));
    }

    // ------------------------------------------------------------------
    // Squared distance
    // ------------------------------------------------------------------
    TEST_CASE("squared_distance(point, segment)") {
        segment s{{0.0f, 0.0f}, {10.0f, 0.0f}};
        CHECK(squared_distance(vec{5.0f, 3.0f}, s) == doctest::Approx(9.0f));    // perpendicular
        CHECK(squared_distance(vec{-5.0f, 0.0f}, s) == doctest::Approx(25.0f));  // before start
        CHECK(squared_distance(vec{15.0f, 0.0f}, s) == doctest::Approx(25.0f));  // past end
        CHECK(squared_distance(vec{5.0f, 0.0f}, s) == doctest::Approx(0.0f));    // on segment
    }

    TEST_CASE("squared_distance(point, segment) is stable for large coordinates") {
        // |ac|^2 - e^2/f loses all precision here; the cross-product form must not.
        segment s{{0.0f, 0.0f}, {100000000.0f, 1.0f}};
        CHECK(squared_distance(vec{50000000.0f, 10.0f}, s) == doctest::Approx(90.25f).epsilon(0.01));
    }

    TEST_CASE("squared_distance(point, aabb)") {
        aabb b{{0.0f, 0.0f}, {4.0f, 4.0f}};
        CHECK(squared_distance(vec{2.0f, 2.0f}, b) == doctest::Approx(0.0f));   // inside
        CHECK(squared_distance(vec{6.0f, 2.0f}, b) == doctest::Approx(4.0f));   // right of edge
        CHECK(squared_distance(vec{6.0f, 6.0f}, b) == doctest::Approx(8.0f));   // off a corner
        CHECK(squared_distance(vec{-3.0f, 2.0f}, b) == doctest::Approx(9.0f));  // left of edge
    }

    TEST_CASE("squared_distance(point, circle)") {
        circle c{{0.0f, 0.0f}, 2.0f};
        CHECK(squared_distance(vec{1.0f, 0.0f}, c) == doctest::Approx(0.0f));   // inside
        CHECK(squared_distance(vec{2.0f, 0.0f}, c) == doctest::Approx(0.0f));   // on rim
        CHECK(squared_distance(vec{5.0f, 0.0f}, c) == doctest::Approx(9.0f));   // (5-2)^2
    }

    // ------------------------------------------------------------------
    // intersect_times(circle, segment)
    // ------------------------------------------------------------------
    TEST_CASE("intersect_times(circle, segment)") {
        circle c{{0.0f, 0.0f}, 1.0f};

        SUBCASE("secant line through the circle") {
            segment s{{-2.0f, 0.0f}, {2.0f, 0.0f}};
            auto r = intersect_times(c, s);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.25f));
            CHECK(r->exit == doctest::Approx(0.75f));
            CHECK(vapprox(r->entry_normal, vec{-1.0f, 0.0f}));
            CHECK(vapprox(r->exit_normal, vec{1.0f, 0.0f}));
            CHECK(segment_intersects(*r));
        }

        SUBCASE("line misses the circle entirely") {
            segment s{{-2.0f, 5.0f}, {2.0f, 5.0f}};
            CHECK_FALSE(intersect_times(c, s).has_value());
        }

        SUBCASE("tangent line yields a single grazing contact") {
            segment s{{-2.0f, 1.0f}, {2.0f, 1.0f}};   // touches top of circle at (0,1)
            auto r = intersect_times(c, s);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(0.5f));
            CHECK(vapprox(r->entry_normal, vec{0.0f, 1.0f}));
        }

        SUBCASE("degenerate (point) segment inside the circle") {
            segment s{{0.0f, 0.0f}, {0.0f, 0.0f}};
            auto r = intersect_times(c, s);
            REQUIRE(r.has_value());
            CHECK(r->entry == constants::NEG_INF);
            CHECK(r->exit == constants::INF);
        }

        SUBCASE("degenerate (point) segment outside the circle") {
            segment s{{5.0f, 5.0f}, {5.0f, 5.0f}};
            CHECK_FALSE(intersect_times(c, s).has_value());
        }
    }

    // ------------------------------------------------------------------
    // Segment / segment
    // ------------------------------------------------------------------
    TEST_CASE("intersect_times(segment, segment)") {
        SUBCASE("crossing diagonals") {
            segment a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            segment b{{0.0f, 2.0f}, {2.0f, 0.0f}};
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(0.5f));
            CHECK(is_unit(r->entry_normal));
            CHECK(intersects(a, b));
        }

        SUBCASE("T-intersection") {
            segment a{{0.0f, 0.0f}, {4.0f, 0.0f}};
            segment b{{2.0f, -1.0f}, {2.0f, 1.0f}};
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(intersects(a, b));
        }

        SUBCASE("parallel, non-collinear miss") {
            segment a{{0.0f, 0.0f}, {1.0f, 0.0f}};
            segment b{{2.0f, 1.0f}, {3.0f, 1.0f}};
            CHECK_FALSE(intersect_times(a, b).has_value());
            CHECK_FALSE(intersects(a, b));
        }

        SUBCASE("disjoint, non-parallel miss") {
            segment a{{0.0f, 0.0f}, {1.0f, 0.0f}};
            segment b{{5.0f, -1.0f}, {5.0f, 1.0f}};   // would cross at x=5, off segment a
            CHECK_FALSE(intersect_times(a, b).has_value());
            CHECK_FALSE(intersects(a, b));
        }

        SUBCASE("collinear overlap has a deterministic unit normal") {
            segment a{{0.0f, 0.0f}, {4.0f, 0.0f}};
            segment b{{2.0f, 0.0f}, {6.0f, 0.0f}};   // overlap [2,4]
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(1.0f));
            CHECK(is_unit(r->entry_normal));
            CHECK(vapprox(r->entry_normal, r->exit_normal));
            CHECK(intersects(a, b));
        }

        SUBCASE("degenerate segment a is a point lying mid-segment of b") {
            segment a{{0.0f, 0.0f}, {0.0f, 0.0f}};
            segment b{{-1.0f, 0.0f}, {1.0f, 0.0f}};
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.0f));
            CHECK(intersects(a, b));
        }

        SUBCASE("degenerate segment a is a point off segment b") {
            segment a{{0.0f, 5.0f}, {0.0f, 5.0f}};
            segment b{{-1.0f, 0.0f}, {1.0f, 0.0f}};
            CHECK_FALSE(intersect_times(a, b).has_value());
            CHECK_FALSE(intersects(a, b));
        }

        SUBCASE("point b on segment a reports its parameter along a") {
            segment a{{0.0f, 0.0f}, {4.0f, 0.0f}};
            segment b{{2.0f, 0.0f}, {2.0f, 0.0f}};   // point at the midpoint of a
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(0.5f));
        }

        SUBCASE("intersects matches intersect_times.has_value()") {
            segment a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            segment b{{0.0f, 2.0f}, {2.0f, 0.0f}};
            CHECK(intersects(a, b) == intersect_times(a, b).has_value());
            segment c{{10.0f, 10.0f}, {11.0f, 11.0f}};
            CHECK(intersects(a, c) == intersect_times(a, c).has_value());
        }
    }

    // ------------------------------------------------------------------
    // CCD: moving AABB vs moving AABB
    // ------------------------------------------------------------------
    TEST_CASE("intersects(aabb, vel, aabb, vel, time)") {
        SUBCASE("head-on approach collides partway through") {
            aabb a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            aabb b{{5.0f, 0.0f}, {7.0f, 2.0f}};
            auto r = intersects(a, vec{10.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.3f));
            CHECK(r->exit == doctest::Approx(0.7f));
            CHECK(r->entry >= 0.0f);
            CHECK(r->exit <= 1.0f);
        }

        SUBCASE("moving apart never collides") {
            aabb a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            aabb b{{5.0f, 0.0f}, {7.0f, 2.0f}};
            CHECK_FALSE(intersects(a, vec{-10.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f).has_value());
        }

        SUBCASE("zero-duration sweep of overlapping boxes does not produce NaN") {
            aabb a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            aabb b{{1.0f, 1.0f}, {3.0f, 3.0f}};
            auto r = intersects(a, vec{0.0f, 0.0f}, b, vec{0.0f, 0.0f}, 0.0f);
            REQUIRE(r.has_value());
            CHECK(finite(*r));
            CHECK(r->entry == doctest::Approx(0.0f));
            CHECK(r->exit == doctest::Approx(0.0f));
        }

        SUBCASE("already-overlapping boxes with zero relative velocity") {
            aabb a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            aabb b{{1.0f, 1.0f}, {3.0f, 3.0f}};
            auto r = intersects(a, vec{0.0f, 0.0f}, b, vec{0.0f, 0.0f}, 0.5f);
            REQUIRE(r.has_value());
            CHECK(finite(*r));
            CHECK(r->entry == doctest::Approx(0.0f));
            CHECK(r->exit <= 0.5f);
        }
    }

    // ------------------------------------------------------------------
    // CCD: moving circle vs moving circle
    // ------------------------------------------------------------------
    TEST_CASE("intersects(circle, vel, circle, vel, time)") {
        SUBCASE("head-on approach collides partway through") {
            circle a{{0.0f, 0.0f}, 1.0f};
            circle b{{5.0f, 0.0f}, 1.0f};
            auto r = intersects(a, vec{10.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.3f));
            CHECK(r->exit == doctest::Approx(0.7f));
        }

        SUBCASE("passing wide misses") {
            circle a{{0.0f, 0.0f}, 1.0f};
            circle b{{5.0f, 10.0f}, 1.0f};
            CHECK_FALSE(intersects(a, vec{10.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f).has_value());
        }

        SUBCASE("resting overlap with zero relative velocity does not produce NaN") {
            circle a{{0.0f, 0.0f}, 1.0f};
            circle b{{0.5f, 0.0f}, 1.0f};   // already overlapping
            auto r = intersects(a, vec{0.0f, 0.0f}, b, vec{0.0f, 0.0f}, 0.016f);
            REQUIRE(r.has_value());
            CHECK(finite(*r));
            CHECK(r->entry == doctest::Approx(0.0f));
            CHECK(r->exit <= 0.016f);
        }

        SUBCASE("zero-duration overlap does not produce NaN") {
            circle a{{0.0f, 0.0f}, 1.0f};
            circle b{{0.5f, 0.0f}, 1.0f};
            auto r = intersects(a, vec{0.0f, 0.0f}, b, vec{0.0f, 0.0f}, 0.0f);
            REQUIRE(r.has_value());
            CHECK(finite(*r));
            CHECK(r->entry == doctest::Approx(0.0f));
            CHECK(r->exit == doctest::Approx(0.0f));
        }
    }

    // ------------------------------------------------------------------
    // CCD: moving circle vs moving AABB
    // ------------------------------------------------------------------
    TEST_CASE("intersects(circle, vel, aabb, vel, time)") {
        SUBCASE("axis-aligned approach hits the left face") {
            circle c{{0.0f, 0.0f}, 1.0f};
            aabb b{{5.0f, -1.0f}, {7.0f, 1.0f}};
            auto r = intersects(c, vec{10.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.4f));   // center reaches x = 4 (min.x - R)
            CHECK(vapprox(r->entry_normal, vec{-1.0f, 0.0f}));
            CHECK(r->entry >= 0.0f);
            CHECK(r->exit <= 1.0f);
            CHECK(r->exit >= r->entry);
            CHECK(finite(*r));
        }

        SUBCASE("moving away misses") {
            circle c{{0.0f, 0.0f}, 1.0f};
            aabb b{{5.0f, -1.0f}, {7.0f, 1.0f}};
            CHECK_FALSE(intersects(c, vec{-10.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f).has_value());
        }

        SUBCASE("diagonal approach toward a corner collides") {
            circle c{{0.0f, 0.0f}, 1.0f};
            aabb b{{5.0f, 5.0f}, {7.0f, 7.0f}};
            auto r = intersects(c, vec{10.0f, 10.0f}, b, vec{0.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(finite(*r));
            CHECK(r->entry >= 0.0f);
            CHECK(r->exit <= 1.0f);
            CHECK(r->exit >= r->entry);
            CHECK(is_unit(r->entry_normal));   // corner-circle contact -> radial unit normal
        }

        SUBCASE("zero-duration overlap does not produce NaN") {
            circle c{{0.0f, 0.0f}, 1.0f};
            aabb b{{-0.5f, -0.5f}, {0.5f, 0.5f}};   // box overlaps the circle center
            auto r = intersects(c, vec{0.0f, 0.0f}, b, vec{0.0f, 0.0f}, 0.0f);
            REQUIRE(r.has_value());
            CHECK(finite(*r));
            CHECK(r->entry == doctest::Approx(0.0f));
            CHECK(r->exit == doctest::Approx(0.0f));
        }
    }

    // ------------------------------------------------------------------
    // Slab method: axis-parallel boundary cases (the original slab bug)
    // ------------------------------------------------------------------
    TEST_CASE("intersect_times(aabb, segment) boundary-parallel cases") {
        aabb box{{0.0f, 0.0f}, {2.0f, 2.0f}};

        SUBCASE("vertical segment exactly on x == min grazes the edge") {
            auto e = intersect_times(box, segment{{0.0f, -1.0f}, {0.0f, 3.0f}});
            CHECK(segment_intersects(e));
            CHECK(e.entry == doctest::Approx(0.25f));
            CHECK(e.exit == doctest::Approx(0.75f));
        }

        SUBCASE("vertical segment exactly on x == max grazes the edge") {
            auto e = intersect_times(box, segment{{2.0f, -1.0f}, {2.0f, 3.0f}});
            CHECK(segment_intersects(e));
            CHECK(e.entry == doctest::Approx(0.25f));
            CHECK(e.exit == doctest::Approx(0.75f));
        }

        SUBCASE("horizontal segment exactly on y == min grazes the edge") {
            auto e = intersect_times(box, segment{{-1.0f, 0.0f}, {3.0f, 0.0f}});
            CHECK(segment_intersects(e));
            CHECK(e.entry == doctest::Approx(0.25f));
            CHECK(e.exit == doctest::Approx(0.75f));
        }

        SUBCASE("horizontal segment exactly on y == max grazes the edge") {
            auto e = intersect_times(box, segment{{-1.0f, 2.0f}, {3.0f, 2.0f}});
            CHECK(segment_intersects(e));
            CHECK(e.entry == doctest::Approx(0.25f));
            CHECK(e.exit == doctest::Approx(0.75f));
        }

        SUBCASE("degenerate point segment inside the box") {
            auto e = intersect_times(box, segment{{1.0f, 1.0f}, {1.0f, 1.0f}});
            CHECK(segment_intersects(e));
            CHECK(e.entry == constants::NEG_INF);
            CHECK(e.exit == constants::INF);
        }

        SUBCASE("degenerate point segment on the boundary") {
            auto e = intersect_times(box, segment{{0.0f, 1.0f}, {0.0f, 1.0f}});
            CHECK(segment_intersects(e));
        }

        SUBCASE("degenerate point segment outside the box misses") {
            auto e = intersect_times(box, segment{{5.0f, 5.0f}, {5.0f, 5.0f}});
            CHECK_FALSE(line_intersects(e));
        }
    }

    // ------------------------------------------------------------------
    // intersect_times(circle, segment): infinite line hits, finite segment does not
    // ------------------------------------------------------------------
    TEST_CASE("intersect_times(circle, segment) finite-segment miss with a line hit") {
        circle c{{0.0f, 0.0f}, 1.0f};
        // The x-axis crosses the circle at x = +/-1, but this finite segment lives
        // entirely at x in [-10, -5]: the line hits, the segment does not.
        auto r = intersect_times(c, segment{{-10.0f, 0.0f}, {-5.0f, 0.0f}});
        REQUIRE(r.has_value());          // infinite line intersects
        CHECK(r->entry > 1.0f);          // both crossings lie beyond the segment
        CHECK(r->exit > 1.0f);
        CHECK_FALSE(segment_intersects(*r));   // finite segment does not reach the circle
    }

    // ------------------------------------------------------------------
    // Zero-radius circle (a point): a common degenerate that must not produce NaN
    // ------------------------------------------------------------------
    TEST_CASE("zero-radius circle") {
        circle z{{0.0f, 0.0f}, 0.0f};

        SUBCASE("contains only the exact center") {
            CHECK(contains(z, vec{0.0f, 0.0f}));
            CHECK_FALSE(contains(z, vec{0.001f, 0.0f}));
        }

        SUBCASE("closest_point collapses to the center") {
            CHECK(vapprox(closest_point(vec{3.0f, 4.0f}, z), vec{0.0f, 0.0f}));
            CHECK(vapprox(closest_point(vec{0.0f, 0.0f}, z), vec{0.0f, 0.0f}));
        }

        SUBCASE("squared_distance is the distance to the point") {
            CHECK(squared_distance(vec{3.0f, 4.0f}, z) == doctest::Approx(25.0f));
            CHECK(squared_distance(vec{0.0f, 0.0f}, z) == doctest::Approx(0.0f));
        }

        SUBCASE("segment through the point is a single finite contact, no NaN") {
            auto r = intersect_times(z, segment{{-2.0f, 0.0f}, {2.0f, 0.0f}});
            REQUIRE(r.has_value());
            CHECK(finite(*r));
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(0.5f));
            CHECK(segment_intersects(*r));
        }

        SUBCASE("segment missing the point returns nullopt") {
            CHECK_FALSE(intersect_times(z, segment{{-2.0f, 1.0f}, {2.0f, 1.0f}}).has_value());
        }
    }

    // ------------------------------------------------------------------
    // Segment / segment: the full collinear / degenerate matrix
    // ------------------------------------------------------------------
    TEST_CASE("intersect_times(segment, segment) collinear and degenerate matrix") {
        segment a{{0.0f, 0.0f}, {4.0f, 0.0f}};

        SUBCASE("collinear but disjoint") {
            CHECK_FALSE(intersect_times(a, segment{{6.0f, 0.0f}, {8.0f, 0.0f}}).has_value());
            CHECK_FALSE(intersects(a, segment{{6.0f, 0.0f}, {8.0f, 0.0f}}));
        }

        SUBCASE("collinear endpoint-only touch") {
            auto r = intersect_times(a, segment{{4.0f, 0.0f}, {8.0f, 0.0f}});
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(1.0f));
            CHECK(r->exit == doctest::Approx(1.0f));
        }

        SUBCASE("identical segments fully overlap") {
            auto r = intersect_times(a, segment{{0.0f, 0.0f}, {4.0f, 0.0f}});
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.0f));
            CHECK(r->exit == doctest::Approx(1.0f));
        }

        SUBCASE("reversed segment overlapping in the middle") {
            // b runs from x=6 back to x=2, overlapping a on [2,4] => params [0.5,1.0] of a.
            auto r = intersect_times(a, segment{{6.0f, 0.0f}, {2.0f, 0.0f}});
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(1.0f));
        }

        SUBCASE("both degenerate, coincident") {
            auto r = intersect_times(segment{{1.0f, 1.0f}, {1.0f, 1.0f}},
                                     segment{{1.0f, 1.0f}, {1.0f, 1.0f}});
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.0f));
            CHECK(r->exit == doctest::Approx(0.0f));
        }

        SUBCASE("both degenerate, distinct") {
            CHECK_FALSE(intersect_times(segment{{1.0f, 1.0f}, {1.0f, 1.0f}},
                                        segment{{2.0f, 2.0f}, {2.0f, 2.0f}}).has_value());
        }
    }

    // ------------------------------------------------------------------
    // CCD boundary timing
    // ------------------------------------------------------------------
    TEST_CASE("CCD boundary timing (aabb)") {
        aabb a{{0.0f, 0.0f}, {2.0f, 2.0f}};
        aabb b{{5.0f, 0.0f}, {7.0f, 2.0f}};   // 3-unit gap to a's right edge

        SUBCASE("collision exactly at time == time is reported") {
            // relative displacement v*T = 3 just reaches contact at the final instant.
            auto r = intersects(a, vec{3.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(1.0f));   // entry == time
        }

        SUBCASE("contact just after the window is a miss") {
            CHECK_FALSE(intersects(a, vec{3.0f, 0.0f}, b, vec{0.0f, 0.0f}, 0.9f).has_value());
        }

        SUBCASE("zero-duration query on non-overlapping boxes returns nullopt") {
            CHECK_FALSE(intersects(a, vec{3.0f, 0.0f}, b, vec{0.0f, 0.0f}, 0.0f).has_value());
        }
    }

    // ------------------------------------------------------------------
    // CCD depends only on relative velocity (both objects moving)
    // ------------------------------------------------------------------
    TEST_CASE("CCD with both objects moving") {
        SUBCASE("aabb: only relative velocity matters") {
            aabb a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            aabb b{{5.0f, 0.0f}, {7.0f, 2.0f}};
            // a moves +5, b moves -5 => relative speed 10, same as the head-on case.
            auto r = intersects(a, vec{5.0f, 0.0f}, b, vec{-5.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.3f));
            CHECK(r->exit == doctest::Approx(0.7f));
        }

        SUBCASE("circle: only relative velocity matters") {
            circle a{{0.0f, 0.0f}, 1.0f};
            circle b{{5.0f, 0.0f}, 1.0f};
            auto r = intersects(a, vec{5.0f, 0.0f}, b, vec{-5.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.3f));
            CHECK(r->exit == doctest::Approx(0.7f));
        }
    }

    // ------------------------------------------------------------------
    // CCD circle/AABB: convex exit selection (numeric exit + exit normal)
    // ------------------------------------------------------------------
    TEST_CASE("CCD circle/aabb pass-through pins exit time and normal") {
        circle c{{0.0f, 0.0f}, 1.0f};
        aabb b{{5.0f, -1.0f}, {7.0f, 1.0f}};
        // Center sweeps along y = 0 from x=0 to x=20. The rounded rectangle spans
        // x in [4, 8] on the center line, so entry/exit are exact and the exit comes
        // from the latest-exiting sub-shape (the horizontal expanded box).
        auto r = intersects(c, vec{20.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f);
        REQUIRE(r.has_value());
        CHECK(r->entry == doctest::Approx(0.2f));          // x = 4 at t = 0.2
        CHECK(r->exit == doctest::Approx(0.4f));           // x = 8 at t = 0.4
        CHECK(vapprox(r->entry_normal, vec{-1.0f, 0.0f}));
        CHECK(vapprox(r->exit_normal, vec{1.0f, 0.0f}));
    }

    // ------------------------------------------------------------------
    // Static circle/AABB: exact corner tangent (3-4-5)
    // ------------------------------------------------------------------
    TEST_CASE("intersects(circle, aabb) exact corner tangent") {
        // Circle at origin radius 5; AABB nearest corner at (3,4) is exactly 5 away.
        CHECK(intersects(circle{{0.0f, 0.0f}, 5.0f}, aabb{{3.0f, 4.0f}, {10.0f, 10.0f}}));
        // Nudge the corner out to (4,4): distance sqrt(32) > 5 => no touch.
        CHECK_FALSE(intersects(circle{{0.0f, 0.0f}, 5.0f}, aabb{{4.0f, 4.0f}, {10.0f, 10.0f}}));
    }

    // ------------------------------------------------------------------
    // Additional Extended Tests
    // ------------------------------------------------------------------
    TEST_CASE("intersect_times(segment, segment) diagonal collinear and parallel extensions") {
        SUBCASE("diagonal collinear overlap") {
            segment a{{0.0f, 0.0f}, {4.0f, 4.0f}};
            segment b{{2.0f, 2.0f}, {6.0f, 6.0f}};
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(1.0f));
            CHECK(is_unit(r->entry_normal));
            CHECK(vapprox(r->entry_normal, r->exit_normal));
            CHECK(intersects(a, b));
        }

        SUBCASE("diagonal collinear overlap reversed") {
            segment a{{0.0f, 0.0f}, {4.0f, 4.0f}};
            segment b{{6.0f, 6.0f}, {2.0f, 2.0f}};
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(1.0f));
            CHECK(is_unit(r->entry_normal));
            CHECK(intersects(a, b));
        }

        SUBCASE("diagonal collinear fully inside") {
            segment a{{0.0f, 0.0f}, {4.0f, 4.0f}};
            segment b{{1.0f, 1.0f}, {3.0f, 3.0f}};
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.25f));
            CHECK(r->exit == doctest::Approx(0.75f));
            CHECK(is_unit(r->entry_normal));
            CHECK(intersects(a, b));
        }

        SUBCASE("diagonal collinear endpoint touch") {
            segment a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            segment b{{2.0f, 2.0f}, {4.0f, 4.0f}};
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(1.0f));
            CHECK(r->exit == doctest::Approx(1.0f));
            CHECK(intersects(a, b));
        }

        SUBCASE("diagonal collinear disjoint") {
            segment a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            segment b{{3.0f, 3.0f}, {5.0f, 5.0f}};
            CHECK_FALSE(intersect_times(a, b).has_value());
            CHECK_FALSE(intersects(a, b));
        }

        SUBCASE("diagonal parallel non-collinear miss") {
            segment a{{0.0f, 0.0f}, {2.0f, 2.0f}};
            segment b{{1.0f, 0.0f}, {3.0f, 2.0f}}; // parallel but shifted by (1, 0)
            CHECK_FALSE(intersect_times(a, b).has_value());
            CHECK_FALSE(intersects(a, b));
        }

        SUBCASE("point b on endpoint of segment a") {
            segment a{{0.0f, 0.0f}, {4.0f, 4.0f}};
            segment b{{4.0f, 4.0f}, {4.0f, 4.0f}};
            auto r = intersect_times(a, b);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(1.0f));
            CHECK(r->exit == doctest::Approx(1.0f));
            CHECK(intersects(a, b));
        }
    }

    TEST_CASE("zero-radius circle normals and CCD") {
        circle z{{0.0f, 0.0f}, 0.0f};

        SUBCASE("normals are exactly zero on collision") {
            auto r = intersect_times(z, segment{{-2.0f, 0.0f}, {2.0f, 0.0f}});
            REQUIRE(r.has_value());
            CHECK(vapprox(r->entry_normal, vec{0.0f, 0.0f}));
            CHECK(vapprox(r->exit_normal, vec{0.0f, 0.0f}));
        }

        SUBCASE("CCD of two moving points (zero-radius circles) that cross") {
            circle a{{0.0f, 0.0f}, 0.0f};
            circle b{{5.0f, 0.0f}, 0.0f};
            auto r = intersects(a, vec{10.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.5f));
            CHECK(r->exit == doctest::Approx(0.5f));
            CHECK(vapprox(r->entry_normal, vec{0.0f, 0.0f}));
            CHECK(vapprox(r->exit_normal, vec{0.0f, 0.0f}));
        }

        SUBCASE("CCD of point (zero-radius circle) vs AABB") {
            circle c{{0.0f, 0.0f}, 0.0f};
            aabb b{{4.0f, -1.0f}, {6.0f, 1.0f}};
            auto r = intersects(c, vec{10.0f, 0.0f}, b, vec{0.0f, 0.0f}, 1.0f);
            REQUIRE(r.has_value());
            CHECK(r->entry == doctest::Approx(0.4f)); // reaches x = 4 at t = 0.4
            CHECK(r->exit == doctest::Approx(0.6f));  // leaves x = 6 at t = 0.6
            CHECK(vapprox(r->entry_normal, vec{-1.0f, 0.0f}));
            CHECK(vapprox(r->exit_normal, vec{1.0f, 0.0f}));
        }
    }

    TEST_CASE("intersect_times(aabb, segment) additional boundary and interior cases") {
        aabb box{{0.0f, 0.0f}, {2.0f, 2.0f}};

        SUBCASE("vertical segment on x == min checks normals") {
            auto e = intersect_times(box, segment{{0.0f, -1.0f}, {0.0f, 3.0f}});
            REQUIRE(segment_intersects(e));
            CHECK(vapprox(e.entry_normal, vec{0.0f, -1.0f}));
            CHECK(vapprox(e.exit_normal, vec{0.0f, 1.0f}));
        }

        SUBCASE("vertical segment on x == max checks normals") {
            auto e = intersect_times(box, segment{{2.0f, -1.0f}, {2.0f, 3.0f}});
            REQUIRE(segment_intersects(e));
            CHECK(vapprox(e.entry_normal, vec{0.0f, -1.0f}));
            CHECK(vapprox(e.exit_normal, vec{0.0f, 1.0f}));
        }

        SUBCASE("horizontal segment on y == min checks normals") {
            auto e = intersect_times(box, segment{{-1.0f, 0.0f}, {3.0f, 0.0f}});
            REQUIRE(segment_intersects(e));
            CHECK(vapprox(e.entry_normal, vec{-1.0f, 0.0f}));
            CHECK(vapprox(e.exit_normal, vec{1.0f, 0.0f}));
        }

        SUBCASE("degenerate point segment inside the box checks normals") {
            auto e = intersect_times(box, segment{{1.0f, 1.0f}, {1.0f, 1.0f}});
            REQUIRE(segment_intersects(e));
            CHECK(vapprox(e.entry_normal, vec{-1.0f, 0.0f}));
            CHECK(vapprox(e.exit_normal, vec{1.0f, 0.0f}));
        }

        SUBCASE("segment completely inside the box yields entry < 0 and exit > 1") {
            segment s{{0.5f, 0.5f}, {1.5f, 1.5f}};
            auto e = intersect_times(box, s);
            CHECK(e.entry == doctest::Approx(-0.5f));
            CHECK(e.exit == doctest::Approx(1.5f));
            CHECK(segment_intersects(e));
            CHECK(vapprox(e.entry_normal, vec{-1.0f, 0.0f}));
            CHECK(vapprox(e.exit_normal, vec{1.0f, 0.0f}));
        }
    }

    TEST_CASE("coordinate bridge conversion tests") {
        // Test with view v having origin at (10, 20), pixels_per_unit = 2.0f, y_up = true
        simplex::view v{{10.0f, 20.0f}, 2.0f, true};

        SUBCASE("point / vec conversions") {
            simplex::point dp_pt = simplex::to_design(simplex::collide::vec{15.0f, 25.0f}, v);
            CHECK(dp_pt.x.design() == doctest::Approx(10.0f));
            CHECK(dp_pt.y.design() == doctest::Approx(-10.0f));

            simplex::collide::vec w_pt = simplex::to_world(simplex::point{simplex::dp{10.0f}, simplex::dp{-10.0f}}, v);
            CHECK(w_pt.x() == doctest::Approx(15.0f));
            CHECK(w_pt.y() == doctest::Approx(25.0f));
        }

        SUBCASE("vector (translation-invariant) conversions") {
            simplex::point dp_vec = simplex::to_design_vector(simplex::collide::vec{5.0f, 5.0f}, v);
            CHECK(dp_vec.x.design() == doctest::Approx(10.0f));
            CHECK(dp_vec.y.design() == doctest::Approx(-10.0f));

            simplex::collide::vec w_vec = simplex::to_world_vector(simplex::point{simplex::dp{10.0f}, simplex::dp{-10.0f}}, v);
            CHECK(w_vec.x() == doctest::Approx(5.0f));
            CHECK(w_vec.y() == doctest::Approx(5.0f));
        }

        SUBCASE("rect / aabb conversions") {
            simplex::rect r{simplex::dp{0.0f}, simplex::dp{0.0f}, simplex::dp{10.0f}, simplex::dp{20.0f}};
            simplex::collide::aabb box = simplex::to_world(r, v);
            CHECK(box.min.x() == doctest::Approx(10.0f));
            CHECK(box.min.y() == doctest::Approx(10.0f));
            CHECK(box.max.x() == doctest::Approx(15.0f));
            CHECK(box.max.y() == doctest::Approx(20.0f));

            simplex::rect r2 = simplex::to_design(box, v);
            CHECK(r2.x.design() == doctest::Approx(0.0f));
            CHECK(r2.y.design() == doctest::Approx(0.0f));
            CHECK(r2.w.design() == doctest::Approx(10.0f));
            CHECK(r2.h.design() == doctest::Approx(20.0f));
        }

        SUBCASE("circle conversions") {
            simplex::circle c{simplex::point{simplex::dp{10.0f}, simplex::dp{-10.0f}}, simplex::dp{4.0f}};
            simplex::collide::circle wc = simplex::to_world(c, v);
            CHECK(wc.center.x() == doctest::Approx(15.0f));
            CHECK(wc.center.y() == doctest::Approx(25.0f));
            CHECK(wc.radius == doctest::Approx(2.0f));

            simplex::circle c2 = simplex::to_design(wc, v);
            CHECK(c2.center().x.design() == doctest::Approx(10.0f));
            CHECK(c2.center().y.design() == doctest::Approx(-10.0f));
            CHECK(c2.radius.design() == doctest::Approx(4.0f));
        }

        SUBCASE("line / segment conversions") {
            simplex::line l{simplex::point{simplex::dp{0.0f}, simplex::dp{0.0f}}, simplex::point{simplex::dp{10.0f}, simplex::dp{-10.0f}}};
            simplex::collide::segment ws_seg = simplex::to_world(l, v);
            CHECK(ws_seg.from.x() == doctest::Approx(10.0f));
            CHECK(ws_seg.from.y() == doctest::Approx(20.0f));
            CHECK(ws_seg.to.x() == doctest::Approx(15.0f));
            CHECK(ws_seg.to.y() == doctest::Approx(25.0f));

            simplex::line l2 = simplex::to_design(ws_seg, v);
            CHECK(l2.start().x.design() == doctest::Approx(0.0f));
            CHECK(l2.start().y.design() == doctest::Approx(0.0f));
            CHECK(l2.end().x.design() == doctest::Approx(10.0f));
            CHECK(l2.end().y.design() == doctest::Approx(-10.0f));
        }
    }
}
