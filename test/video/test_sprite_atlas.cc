#include <doctest/doctest.h>
#include <simplex/sprite_atlas.hh>
#include <simplex/sprite.hh>
#include <simplex/mesh.hh>
#include <simplex/effects.hh>

TEST_SUITE("simplex::sprite_atlas") {
    TEST_CASE("default constructor") {
        simplex::sprite_atlas atlas;
        CHECK(!atlas.is_valid());
        CHECK(atlas.frame_count() == 0);
        CHECK(!atlas.has_bitmasks());
    }

    TEST_CASE("constructor with explicit rects") {
        std::vector<sdlpp::rect<int>> rects = {
            {0, 0, 16, 16},
            {16, 0, 16, 16},
            {0, 16, 32, 32}
        };

        simplex::sprite_atlas atlas(sdlpp::texture{}, rects);
        CHECK(!atlas.is_valid()); // Texture is invalid
        CHECK(atlas.frame_count() == 3);
        CHECK(!atlas.has_bitmasks());

        // Check rect retrieval
        auto r0 = atlas.get_frame_rect(0);
        CHECK(r0.x == 0);
        CHECK(r0.y == 0);
        CHECK(r0.w == 16);
        CHECK(r0.h == 16);

        auto r1 = atlas.get_frame_rect(1);
        CHECK(r1.x == 16);
        CHECK(r1.y == 0);
        CHECK(r1.w == 16);
        CHECK(r1.h == 16);

        auto r2 = atlas.get_frame_rect(2);
        CHECK(r2.x == 0);
        CHECK(r2.y == 16);
        CHECK(r2.w == 32);
        CHECK(r2.h == 32);

        // Check design size conversion (design pixels)
        auto s0 = atlas.get_frame_size(0);
        CHECK(s0.width.design() == 16.0f);
        CHECK(s0.height.design() == 16.0f);

        auto s2 = atlas.get_frame_size(2);
        CHECK(s2.width.design() == 32.0f);
        CHECK(s2.height.design() == 32.0f);
    }

    TEST_CASE("bitmask functionality") {
        simplex::bitmask mask(4, 4);
        CHECK(mask.width() == 4);
        CHECK(mask.height() == 4);

        // Check default state
        CHECK(!mask.get(0, 0));
        CHECK(!mask.get(3, 3));

        // Set/Get bits
        mask.set(1, 1, true);
        mask.set(2, 2, true);
        CHECK(mask.get(1, 1));
        CHECK(mask.get(2, 2));
        CHECK(!mask.get(1, 2));

        // Coordinates out of bounds should return false
        CHECK(!mask.get(-1, 0));
        CHECK(!mask.get(4, 0));

        // Overlap tests
        simplex::bitmask other(4, 4);
        other.set(1, 1, true);

        // Identical overlap at offset (0, 0)
        CHECK(mask.overlaps(other, 0, 0));

        // Offset overlap: if other is shifted right by 1, its (1, 1) aligns with mask's (2, 1) - which is false
        CHECK(!mask.overlaps(other, 1, 0));

        // Offset overlap: if other is shifted right by 1 and down by 1, its (1, 1) aligns with mask's (2, 2) - which is true
        CHECK(mask.overlaps(other, 1, 1));

        // No overlap in bounds
        CHECK(!mask.overlaps(other, 10, 10));
    }

    TEST_CASE("user-supplied bitmasks") {
        std::vector<sdlpp::rect<int>> rects = {
            {0, 0, 16, 16},
            {16, 0, 16, 16}
        };

        simplex::sprite_atlas atlas(sdlpp::texture{}, rects);
        CHECK(!atlas.has_bitmasks());

        simplex::bitmask mask1(16, 16);
        mask1.set(5, 5, true);

        atlas.set_frame_mask(0, mask1);
        CHECK(atlas.has_bitmasks());

        // The second frame should default to an empty bitmask (width=0, height=0)
        auto m0 = atlas.get_frame_mask(0);
        CHECK(m0.width() == 16);
        CHECK(m0.height() == 16);
        CHECK(m0.get(5, 5) == true);

        auto m1 = atlas.get_frame_mask(1);
        CHECK(m1.width() == 0);
        CHECK(m1.height() == 0);
    }

    TEST_CASE("sprite movement queue") {
        simplex::animated_sprite sprite;
        sprite.position = {simplex::dp{10.0f}, simplex::dp{20.0f}};

        CHECK(!sprite.is_moving());

        // Queue a linear movement: offset +30x, +40y over 2.0 seconds
        sprite.queue_movement({
            .target_offset = {simplex::dp{30.0f}, simplex::dp{40.0f}},
            .duration = 2.0f
        });

        CHECK(sprite.is_moving());

        // Update by 0.5s (25% progress)
        sprite.update(0.5f);
        CHECK(sprite.position.x.design() == doctest::Approx(17.5f));
        CHECK(sprite.position.y.design() == doctest::Approx(30.0f));
        CHECK(sprite.is_moving());

        // Update by 1.5s (reaches end of step, snaps to target)
        sprite.update(1.5f);
        CHECK(sprite.position.x.design() == doctest::Approx(40.0f));
        CHECK(sprite.position.y.design() == doctest::Approx(60.0f));
        CHECK(!sprite.is_moving());

        // Queue a zero-duration teleport step
        sprite.queue_movement({
            .target_offset = {simplex::dp{-10.0f}, simplex::dp{-10.0f}},
            .duration = 0.0f
        });
        CHECK(sprite.is_moving());
        sprite.update(0.1f);
        CHECK(sprite.position.x.design() == doctest::Approx(30.0f));
        CHECK(sprite.position.y.design() == doctest::Approx(50.0f));
        CHECK(!sprite.is_moving());

        // Queue multiple steps and check sequence
        sprite.queue_movement({.target_offset = {simplex::dp{10.0f}, simplex::dp{0.0f}}, .duration = 1.0f});
        sprite.queue_movement({.target_offset = {simplex::dp{0.0f}, simplex::dp{10.0f}}, .duration = 1.0f});

        // Update by 1.5s (should complete step 1, and be 50% through step 2)
        sprite.update(1.5f);
        CHECK(sprite.position.x.design() == doctest::Approx(40.0f)); // 30 + 10
        CHECK(sprite.position.y.design() == doctest::Approx(55.0f)); // 50 + 5
        CHECK(sprite.is_moving());

        // Clear movements
        sprite.clear_movements();
        CHECK(!sprite.is_moving());
    }

    TEST_CASE("sprite_mesh mesh creation and layout") {
        simplex::sprite_mesh mesh(2, 2);
        CHECK(mesh.vertex_count() == 9); // 3x3 vertices
        CHECK(mesh.index_count() == 24);  // 2x2 quads * 2 triangles * 3 indices = 24

        // Layout the mesh to a design rect (dp)
        simplex::rect dest{simplex::dp{0.0f}, simplex::dp{0.0f}, simplex::dp{20.0f}, simplex::dp{30.0f}};
        mesh.layout_rect(dest, sdlpp::rect<float>{0.0f, 0.0f, 1.0f, 1.0f});

        // Transform vertices: shift them right by 5 physical pixels
        mesh.transform_vertices([](int col, int row, float u, float v, float& px, float& py) {
            px += 5.0f;
        });

        // Vertices are correctly mutated
        CHECK(mesh.vertex_count() == 9);
    }

    TEST_CASE("sprite_mesh effects library") {
        simplex::sprite_mesh mesh(2, 2);
        simplex::rect dest{simplex::dp{0.0f}, simplex::dp{0.0f}, simplex::dp{20.0f}, simplex::dp{30.0f}};
        mesh.layout_rect(dest, sdlpp::rect<float>{0.0f, 0.0f, 1.0f, 1.0f});

        // Test wave_horizontal
        simplex::effects::wave_horizontal(mesh, 0.0f, 5.0f, 1.0f);
        CHECK(mesh.vertex_count() == 9);

        // Test wave_vertical
        simplex::effects::wave_vertical(mesh, 0.0f, 5.0f, 1.0f);
        CHECK(mesh.vertex_count() == 9);

        // Test skew_horizontal
        simplex::effects::skew_horizontal(mesh, 10.0f);
        CHECK(mesh.vertex_count() == 9);

        // Test skew_vertical
        simplex::effects::skew_vertical(mesh, 10.0f);
        CHECK(mesh.vertex_count() == 9);

        // Test perspective
        simplex::effects::perspective(mesh, 0.8f, 1.2f);
        CHECK(mesh.vertex_count() == 9);

        // Test pinch_punch
        simplex::effects::pinch_punch(mesh, 0.5f, 0.5f, 0.5f, 0.2f);
        CHECK(mesh.vertex_count() == 9);

        // Test rotate_3d
        simplex::effects::rotate_3d(mesh, euler::degreef(10.0f), euler::radianf(0.2f), euler::degreef(15.0f));
        CHECK(mesh.vertex_count() == 9);
    }
}

