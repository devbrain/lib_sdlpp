#include <doctest/doctest.h>
#include <simplex/sprite_atlas.hh>

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
}
