//
// Unit tests for simplex::collide::bridge — display-space (dp) <-> world-space
// coordinate conversions. The collision math itself is covered in test_collide.cc;
// this file exercises only the bridge that crosses the two coordinate systems.
//

#include <doctest/doctest.h>

#include <simplex/collide/bridge.hh>

TEST_SUITE("simplex::collide::bridge") {
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
