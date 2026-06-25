//
// Created by igor on 25/06/2026.
//
// Integration tests for the world <-> static grid fan-out (G4): tiles added via the
// unified add(tile_body) participate in the world's queries (raycast / cast /
// overlap-triggers) and in move_and_slide, merged with the BVH residents (nearest
// for closest-hit queries, union for overlap). Tiles are bucketed into the single
// cell containing their centre, so every tile here is cell-sized.
//
#include <doctest/doctest.h>

#include <cmath>
#include <variant>

#include <simplex/collide/dynamic/world.hh>

using namespace simplex::collide;

namespace {
    // 8x8 cells of 2x2 over [0,16]^2.
    world grid_world() {
        world_config cfg;
        cfg.bounds = aabb{vec{0, 0}, vec{16, 16}};
        cfg.grid = world_config::grid_config{vec{2, 2}};
        return world(cfg);
    }

    aabb tight_of(const world& w, collider_id id) {
        return std::visit([](const auto& s) { return enclose(s); }, w.get_shape(id));
    }
}

TEST_SUITE("world+grid: raycast") {
    TEST_CASE("a ray hits a tile and reports its TILE handle + eid + toi") {
        world w = grid_world();
        w.add(1, tile_body{aabb{vec{6, 0}, vec{8, 2}}, {}, {}}); // cell (3,0)
        const auto hit = w.raycast(segment{vec{0, 1}, vec{16, 1}});
        REQUIRE(hit.has_value());
        CHECK(hit->who.type_id == collider_id::TILE);
        CHECK(w.get_eid(hit->who) == 1u);
        CHECK(hit->toi == doctest::Approx(0.375f)); // left face x=6 over a length-16 ray
    }

    TEST_CASE("the nearer of a tile and a body wins the merge") {
        SUBCASE("body is nearer") {
            world w = grid_world();
            w.add(10, tile_body{aabb{vec{10, 0}, vec{12, 2}}, {}, {}});          // far tile
            w.add(20, static_body{shape_t{aabb{vec{4, 0}, vec{5, 2}}}, {}, {}}); // near body
            const auto hit = w.raycast(segment{vec{0, 1}, vec{16, 1}});
            REQUIRE(hit.has_value());
            CHECK(hit->who.type_id == collider_id::BODY);
            CHECK(w.get_eid(hit->who) == 20u);
        }
        SUBCASE("tile is nearer") {
            world w = grid_world();
            w.add(10, tile_body{aabb{vec{4, 0}, vec{6, 2}}, {}, {}});             // near tile
            w.add(20, static_body{shape_t{aabb{vec{10, 0}, vec{11, 2}}}, {}, {}});// far body
            const auto hit = w.raycast(segment{vec{0, 1}, vec{16, 1}});
            REQUIRE(hit.has_value());
            CHECK(hit->who.type_id == collider_id::TILE);
            CHECK(w.get_eid(hit->who) == 10u);
        }
    }

    TEST_CASE("a slope (segment) tile is crossed like a wall") {
        world w = grid_world();
        w.add(3, tile_body{segment{vec{4, 4}, vec{6, 6}}, {}, {}}); // diagonal in cell (2,2)
        // a ray crossing that diagonal
        const auto hit = w.raycast(segment{vec{4, 6}, vec{6, 4}});
        REQUIRE(hit.has_value());
        CHECK(hit->who.type_id == collider_id::TILE);
        CHECK(w.get_eid(hit->who) == 3u);
    }

    TEST_CASE("filter excludes a tile") {
        world w = grid_world();
        filter_props tf; tf.category = 0x0002; tf.mask = 0xFFFF;
        w.add(1, tile_body{aabb{vec{6, 0}, vec{8, 2}}, {}, tf});
        filter_props qf; qf.category = 0xFFFF; qf.mask = 0x0001; // does not include the tile's category
        CHECK_FALSE(w.raycast(segment{vec{0, 1}, vec{16, 1}}, qf).has_value());
    }
}

TEST_SUITE("world+grid: triggers") {
    TEST_CASE("a sensor body senses an overlapping tile (TRIGGER_BEGIN, then END on exit)") {
        world w = grid_world();
        const collider_id tile = w.add(2, tile_body{aabb{vec{4, 4}, vec{6, 6}}, {}, {}}); // cell (2,2)
        material_props sens; sens.response = response_mode::SENSOR;
        const collider_id s = w.add(99, kinematic_body{
                                        moving_shape_t{aabb{vec{4.5f, 4.5f}, vec{5.5f, 5.5f}}}, sens, {}, vec{0, 0}});

        auto count_kind = [](const std::vector<world_event>& evs, event_kind k) {
            int n = 0;
            for (const auto& e : evs) {
                if (e.kind != k) continue;
                if (e.target.type_id == collider_id::TILE || e.mover.type_id == collider_id::TILE) ++n;
            }
            return n;
        };

        const auto& f1 = w.run(aabb{vec{0, 0}, vec{16, 16}}, 1.0f / 60.0f);
        CHECK(count_kind(f1, event_kind::TRIGGER_BEGIN) == 1);

        // still overlapping next frame -> no repeat begin
        const auto& f2 = w.run(aabb{vec{0, 0}, vec{16, 16}}, 1.0f / 60.0f);
        CHECK(count_kind(f2, event_kind::TRIGGER_BEGIN) == 0);

        // remove the tile -> the overlap ends
        w.remove(tile);
        const auto& f3 = w.run(aabb{vec{0, 0}, vec{16, 16}}, 1.0f / 60.0f);
        CHECK(count_kind(f3, event_kind::TRIGGER_END) == 1);
        (void)s;
    }
}

TEST_SUITE("world+grid: move_and_slide") {
    TEST_CASE("a falling body lands on a tile floor (does not tunnel through)") {
        world w = grid_world();
        material_props floor; floor.response = response_mode::BLOCK; floor.block_normal = vec{0, 1};
        w.add(5, tile_body{aabb{vec{2, 0}, vec{4, 2}}, floor, {}}); // floor tile, cell (1,0)
        const collider_id b = w.add(6, kinematic_body{
                                        moving_shape_t{aabb{vec{2.5f, 3.0f}, vec{3.5f, 4.0f}}}, {}, {}, vec{0, -120}});

        const float y0 = tight_of(w, b).min.y();
        const auto& evs = w.run(aabb{vec{0, 0}, vec{16, 16}}, 1.0f / 60.0f); // dy = -2 (into the floor)
        const float y1 = tight_of(w, b).min.y();

        CHECK(y1 < y0);              // it fell
        CHECK(y1 >= 2.0f - 0.05f);   // but stopped at/above the floor top y=2 (within skin)

        bool collided_with_tile = false;
        for (const auto& e : evs) {
            if (e.kind == event_kind::COLLISION && e.target.type_id == collider_id::TILE) {
                collided_with_tile = true;
                CHECK(w.get_eid(e.target) == 5u);
            }
        }
        CHECK(collided_with_tile);
    }

    TEST_CASE("a body slides along a tile wall instead of stopping dead") {
        world w = grid_world();
        material_props wall; wall.response = response_mode::BLOCK; wall.block_normal = vec{-1, 0};
        w.add(7, tile_body{aabb{vec{6, 0}, vec{8, 2}}, wall, {}}); // wall tile, cell (3,0)
        // move down-right into the wall's left face; should slide down along it
        const collider_id b = w.add(8, kinematic_body{
                                        moving_shape_t{aabb{vec{4.0f, 0.5f}, vec{5.0f, 1.5f}}}, {}, {}, vec{120, -60}});
        const float x0 = tight_of(w, b).min.x();
        (void)w.run(aabb{vec{0, 0}, vec{16, 16}}, 1.0f / 60.0f);
        const aabb after = tight_of(w, b);
        CHECK(after.min.x() > x0);          // advanced toward the wall
        CHECK(after.max.x() <= 6.0f + 0.05f); // but did not cross the wall's left face x=6
    }
}

TEST_SUITE("world+grid: cast (aiming probe)") {
    TEST_CASE("a swept cast reports the earliest tile contact") {
        world w = grid_world();
        w.add(1, tile_body{aabb{vec{6, 0}, vec{8, 2}}, {}, {}}); // cell (3,0)
        // sweep a small box rightward through the tile
        const auto hit = w.cast(moving_shape_t{aabb{vec{0, 0.4f}, vec{1, 1.4f}}}, vec{16, 0});
        REQUIRE(hit.has_value());
        CHECK(hit->who.type_id == collider_id::TILE);
        CHECK(w.get_eid(hit->who) == 1u);
    }
}
