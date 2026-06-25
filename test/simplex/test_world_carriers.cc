//
// Created by igor on 25/06/2026.
//
// Moving platforms / carriers (§19 #1, MP1 — carrying). A carrier_body is a kinematic "solid"
// that moves rigidly on a scripted path and transports the actors riding it. Each frame it
// translates by velocity*dt; a rider on top inherits (velocity + surface_velocity)*dt:
//   moving platform -> surface_velocity {0,0}; conveyor -> velocity {0,0}, surface = belt speed.
// MP1 covers carrying only; pushing/crushing are follow-ups.
//
#include <doctest/doctest.h>

#include <variant>

#include <simplex/collide/dynamic/world.hh>

using namespace simplex::collide;

namespace {
    world carrier_world() {
        world_config cfg;
        cfg.bounds = aabb{vec{0, 0}, vec{64, 64}};
        cfg.up = {0, 1};
        cfg.skin = 0.01f;
        return world(cfg);
    }

    aabb box_of(const world& w, collider_id id) {
        return std::visit([](const auto& s) { return enclose(s); }, w.get_shape(id));
    }

    // a 1x1 rider resting on top of a carrier whose top is at y (skin gap above it)
    collider_id add_rider(world& w, entity_id_t eid, float x, float top) {
        return w.add(eid, kinematic_body{
                         moving_shape_t{aabb{vec{x, top + 0.01f}, vec{x + 1.0f, top + 1.01f}}}, {}, {}, vec{0, 0}});
    }

    constexpr float DT = 1.0f / 60.0f;
}

TEST_SUITE("world: carriers (moving platforms / conveyors)") {
    TEST_CASE("a horizontal moving platform carries its rider") {
        world w = carrier_world();
        const collider_id plat = w.add(1, carrier_body{
                                           moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, {}, vec{60, 0}, vec{0, 0}});
        const collider_id rider = add_rider(w, 2, 12, 10); // on the platform top (y=10)
        const float rx0 = box_of(w, rider).min.x();

        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);   // dx = +1

        CHECK(box_of(w, rider).min.x() == doctest::Approx(rx0 + 1.0f).epsilon(0.05));
        CHECK(box_of(w, plat).min.x() == doctest::Approx(11.0f).epsilon(0.05)); // platform moved too
    }

    TEST_CASE("a vertical elevator carries its rider up") {
        world w = carrier_world();
        w.add(1, carrier_body{moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, {}, vec{0, 60}, vec{0, 0}});
        const collider_id rider = add_rider(w, 2, 12, 10);
        const float ry0 = box_of(w, rider).min.y();

        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);   // dy = +1

        CHECK(box_of(w, rider).min.y() == doctest::Approx(ry0 + 1.0f).epsilon(0.05));
    }

    TEST_CASE("a conveyor drags its rider but does not move itself") {
        world w = carrier_world();
        const collider_id belt = w.add(1, carrier_body{
                                           moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, {}, vec{0, 0}, vec{120, 0}});
        const collider_id rider = add_rider(w, 2, 12, 10);
        const float rx0 = box_of(w, rider).min.x();
        const float bx0 = box_of(w, belt).min.x();

        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);   // drag = 120*dt = +2

        CHECK(box_of(w, rider).min.x() == doctest::Approx(rx0 + 2.0f).epsilon(0.05));
        CHECK(box_of(w, belt).min.x() == doctest::Approx(bx0)); // belt itself stays put
    }

    TEST_CASE("a moving conveyor carries AND drags (the sum)") {
        world w = carrier_world();
        // platform moves +1/step, belt drags another +2/step -> rider moves +3
        w.add(1, carrier_body{moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, {}, vec{60, 0}, vec{120, 0}});
        const collider_id rider = add_rider(w, 2, 12, 10);
        const float rx0 = box_of(w, rider).min.x();

        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);

        CHECK(box_of(w, rider).min.x() == doctest::Approx(rx0 + 3.0f).epsilon(0.05));
    }

    TEST_CASE("a body that is not riding is not carried") {
        world w = carrier_world();
        w.add(1, carrier_body{moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, {}, vec{60, 0}, vec{0, 0}});
        const collider_id other = w.add(2, kinematic_body{
                                            moving_shape_t{aabb{vec{40, 40}, vec{41, 41}}}, {}, {}, vec{0, 0}});
        const float ox0 = box_of(w, other).min.x();

        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);

        CHECK(box_of(w, other).min.x() == doctest::Approx(ox0)); // untouched
    }

    TEST_CASE("a rider carried into a wall stops, doesn't tunnel (MP1: blocked, no crush yet)") {
        world w = carrier_world();
        w.add(1, carrier_body{moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, {}, vec{600, 0}, vec{0, 0}}); // dx=+10
        w.add(3, static_body{shape_t{aabb{vec{14, 10}, vec{15, 14}}}, {}, {}});   // wall above-right of the platform
        const collider_id rider = add_rider(w, 2, 12, 10);

        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);

        CHECK(box_of(w, rider).max.x() <= 14.0f + 0.05f); // stopped at the wall's left face, not through it
    }

    TEST_CASE("a body touching the carrier's SIDE is not carried (contact direction)") {
        world w = carrier_world();
        w.add(1, carrier_body{moving_shape_t{aabb{vec{10, 8}, vec{16, 12}}}, {}, {}, vec{60, 0}, vec{0, 0}});
        // body flush against the LEFT side of the carrier (x meets at 10), not on top
        const collider_id sider = w.add(2, kinematic_body{
                                            moving_shape_t{aabb{vec{9.0f, 9.0f}, vec{10.0f, 11.0f}}}, {}, {}, vec{0, 0}});
        const float sx0 = box_of(w, sider).min.x();
        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);
        CHECK(box_of(w, sider).min.x() == doctest::Approx(sx0)); // side contact is not "riding"
    }

    TEST_CASE("a rider whose filter rejects the carrier is not carried") {
        world w = carrier_world();
        filter_props cf; cf.category = 0x0002; cf.mask = 0xFFFF;     // carrier in category 2
        w.add(1, carrier_body{moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, cf, vec{60, 0}, vec{0, 0}});
        filter_props rf; rf.category = 0xFFFF; rf.mask = 0x0001;     // rider masks OUT category 2
        const collider_id rider = w.add(2, kinematic_body{
                                            moving_shape_t{aabb{vec{12, 10.01f}, vec{13, 11.01f}}}, {}, rf, vec{0, 0}});
        const float rx0 = box_of(w, rider).min.x();
        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);
        CHECK(box_of(w, rider).min.x() == doctest::Approx(rx0)); // filtered out -> not carried
    }

    TEST_CASE("a SENSOR carrier (non-solid) carries nothing") {
        world w = carrier_world();
        material_props sens; sens.response = response_mode::SENSOR;
        w.add(1, carrier_body{moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, sens, {}, vec{60, 0}, vec{0, 0}});
        const collider_id rider = add_rider(w, 2, 12, 10);
        const float rx0 = box_of(w, rider).min.x();
        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);
        CHECK(box_of(w, rider).min.x() == doctest::Approx(rx0)); // non-solid carrier -> no carry
    }

    TEST_CASE("set_shape on a carrier rejects a non-mover shape (segment / triangle)") {
        world w = carrier_world();
        const collider_id c = w.add(1, carrier_body{
                                        moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, {}, vec{0, 0}, vec{0, 0}});
        CHECK_THROWS(w.set_shape(c, shape_t{segment{vec{10, 8}, vec{16, 10}}}));
        CHECK_THROWS(w.set_shape(c, shape_t{triangle{vec{10, 8}, vec{16, 8}, vec{10, 10}}}));
        CHECK_NOTHROW(w.set_shape(c, shape_t{circle{vec{13, 9}, 1.0f}})); // a mover shape is fine
    }

    TEST_CASE("set_velocity / set_surface_velocity work on a carrier") {
        world w = carrier_world();
        const collider_id belt = w.add(1, carrier_body{
                                           moving_shape_t{aabb{vec{10, 8}, vec{16, 10}}}, {}, {}, vec{0, 0}, vec{120, 0}});
        const collider_id rider = add_rider(w, 2, 12, 10);

        w.set_surface_velocity(belt, vec{0, 0}); // switch the belt off
        const float rx0 = box_of(w, rider).min.x();
        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);
        CHECK(box_of(w, rider).min.x() == doctest::Approx(rx0)); // no drag while off

        w.set_velocity(belt, vec{60, 0}); // now make it a moving platform
        (void)w.run(aabb{vec{0, 0}, vec{64, 64}}, DT);
        CHECK(box_of(w, rider).min.x() == doctest::Approx(rx0 + 1.0f).epsilon(0.05));
    }
}
