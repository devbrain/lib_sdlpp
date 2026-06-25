//
// Created by igor on 21/06/2026.
//
// Tests for the collision world (Layer 3) skeleton in
// <simplex/collide/dynamic/world.hh>: collider lifecycle (add/remove), the
// generation-tagged handle (collider_id) and its validity/recycling semantics, and
// the mutator precondition guards (set_shape / set_velocity / remove).
//
// Scope note: the world does not yet expose any read-back of the broadphase proxy or
// the stored body, so the "every resident has a fat box that bounds its shape"
// invariant cannot be observed here -- it belongs to the Phase 2 query API. These
// tests cover exactly what the public surface allows today, with ENFORCE-guard checks
// via CHECK_THROWS (ENFORCE throws).
//
#include <doctest/doctest.h>

#include <set>
#include <unordered_map>
#include <vector>

#include <simplex/collide/dynamic/world.hh>

using namespace simplex::collide;

namespace {
    aabb box_at(float x, float y, float w = 1.0f, float h = 1.0f) {
        return aabb{{x, y}, {x + w, y + h}};
    }

    static_body mk_static(const shape_t& s) {
        static_body b;
        b.shape = s;
        return b;
    }

    kinematic_body mk_kine(const moving_shape_t& s, vec v) {
        kinematic_body b;
        b.shape = s;
        b.velocity = v;
        return b;
    }

    bullet mk_bullet(const moving_shape_t& s, vec v) {
        bullet b;
        b.shape = s;
        b.velocity = v;
        return b;
    }
} // namespace

TEST_SUITE("world: handle lifecycle") {
    TEST_CASE("add returns a valid handle") {
        world w;
        const collider_id a = w.add(1, mk_static(box_at(0, 0)));
        const collider_id b = w.add(2, mk_kine(box_at(5, 5), vec{1, 0}));
        CHECK(w.is_valid(a));
        CHECK(w.is_valid(b));
    }

    TEST_CASE("distinct live bodies get distinct slots") {
        world w;
        const collider_id a = w.add(1, mk_static(box_at(0, 0)));
        const collider_id b = w.add(2, mk_static(box_at(1, 1)));
        const collider_id c = w.add(3, mk_static(box_at(2, 2)));
        CHECK(a.value != b.value);
        CHECK(b.value != c.value);
        CHECK(a.value != c.value);
        CHECK(w.is_valid(a));
        CHECK(w.is_valid(b));
        CHECK(w.is_valid(c));
    }

    TEST_CASE("remove invalidates the handle") {
        world w;
        const collider_id a = w.add(1, mk_static(box_at(0, 0)));
        REQUIRE(w.is_valid(a));
        w.remove(a);
        CHECK_FALSE(w.is_valid(a));
    }

    TEST_CASE("remove of a stale handle is an idempotent no-op (no throw)") {
        world w;
        const collider_id a = w.add(1, mk_static(box_at(0, 0)));
        w.remove(a);
        CHECK_NOTHROW(w.remove(a)); // double remove: gated by is_valid, never hits the double-free ENFORCE
        CHECK_NOTHROW(w.remove(a)); // and again
        CHECK_FALSE(w.is_valid(a));
    }

    TEST_CASE("is_valid rejects out-of-range and mismatched-generation handles") {
        world w;
        const collider_id a = w.add(1, mk_static(box_at(0, 0)));

        const collider_id oob{9999u, 0u};                       // value past the pool
        CHECK_FALSE(w.is_valid(oob));

        const collider_id wrong_gen{a.value, a.generation + 7}; // right slot, wrong generation
        CHECK_FALSE(w.is_valid(wrong_gen));

        CHECK(w.is_valid(a)); // the genuine handle still validates
    }

    TEST_CASE("handles into an empty world are invalid (no UB)") {
        const world w;
        CHECK_FALSE(w.is_valid(collider_id{0u, 0u}));
        CHECK_FALSE(w.is_valid(collider_id{123u, 0u}));
    }

    TEST_CASE("default-constructed collider_id is the null sentinel") {
        const collider_id null{};
        CHECK_FALSE(null.valid());                 // the cheap self-check
        CHECK(null.value == collider_id::INVALID);

        world w;
        CHECK_FALSE(w.is_valid(null));             // and the storage check rejects it too

        const collider_id a = w.add(1, mk_static(box_at(0, 0)));
        CHECK(a.valid());                          // a real handle is not null
        CHECK(w.is_valid(a));
    }
}

TEST_SUITE("world: bullet pool lifecycle") {
    TEST_CASE("add returns a BULLET-typed valid handle; remove invalidates") {
        world w;
        const collider_id b = w.add(7, mk_bullet(circle{{0, 0}, 0.5f}, vec{1, 0}));
        CHECK(b.type_id == collider_id::BULLET);
        CHECK(w.is_valid(b));
        CHECK(w.get_eid(b) == 7u);
        w.remove(b);
        CHECK_FALSE(w.is_valid(b));
        CHECK_NOTHROW(w.remove(b)); // idempotent on a stale handle
    }

    TEST_CASE("bullet slot recycles with a fresh generation; old handle goes stale") {
        world w;
        const collider_id a = w.add(1, mk_bullet(circle{{0, 0}, 0.5f}, vec{1, 0}));
        w.remove(a);
        const collider_id b = w.add(2, mk_bullet(circle{{9, 9}, 0.5f}, vec{0, 1}));
        CHECK(b.value == a.value);              // same slot
        CHECK(b.generation == a.generation + 1u);
        CHECK(w.is_valid(b));
        CHECK_FALSE(w.is_valid(a));             // stale handle to the recycled bullet slot
    }

    TEST_CASE("bullet and body pools are independent (same slot index, different type)") {
        world w;
        const collider_id body = w.add(1, mk_static(box_at(0, 0)));
        const collider_id bul = w.add(2, mk_bullet(circle{{0, 0}, 0.5f}, vec{1, 0}));
        CHECK(body.type_id == collider_id::BODY);
        CHECK(bul.type_id == collider_id::BULLET);
        // both are slot 0 of their own pool, but the type tag keeps them distinct + valid
        CHECK(body.value == bul.value);
        CHECK(w.is_valid(body));
        CHECK(w.is_valid(bul));
    }

    TEST_CASE("set_velocity / set_shape on a live bullet; set_shape(segment) throws") {
        world w;
        const collider_id b = w.add(1, mk_bullet(circle{{0, 0}, 0.5f}, vec{1, 0}));
        CHECK_NOTHROW(w.set_velocity(b, vec{5, -2}));     // bullets accept velocity (no kind guard)
        CHECK(w.get_velocity(b).x() == doctest::Approx(5.0f));
        CHECK_NOTHROW(w.set_shape(b, shape_t{circle{{1, 1}, 0.25f}}));
        CHECK_NOTHROW(w.set_shape(b, shape_t{box_at(2, 2)}));
        CHECK_THROWS(w.set_shape(b, shape_t{segment{{0, 0}, {1, 1}}})); // a bullet cannot be a segment
    }

    TEST_CASE("mutating a stale bullet handle throws") {
        world w;
        const collider_id b = w.add(1, mk_bullet(circle{{0, 0}, 0.5f}, vec{1, 0}));
        w.remove(b);
        CHECK_THROWS(w.set_velocity(b, vec{1, 1}));
        CHECK_THROWS(w.set_shape(b, shape_t{circle{{0, 0}, 0.5f}}));
    }
}

TEST_SUITE("world: generation and slot recycling") {
    TEST_CASE("a recycled slot bumps generation and invalidates the old handle") {
        world w;
        const collider_id a = w.add(1, mk_static(box_at(0, 0)));
        w.remove(a);

        // The free list is LIFO and only one slot was freed, so the next add reuses it.
        const collider_id b = w.add(2, mk_static(box_at(9, 9)));
        CHECK(b.value == a.value);            // same physical slot
        CHECK(b.generation != a.generation);  // but a fresh generation
        CHECK(b.generation == a.generation + 1u);

        CHECK(w.is_valid(b));        // new handle is live
        CHECK_FALSE(w.is_valid(a));  // old handle to the recycled slot is dead
    }

    TEST_CASE("repeated recycle of the same slot keeps advancing the generation") {
        world w;
        collider_id prev = w.add(0, mk_static(box_at(0, 0)));
        const uint32_t slot = prev.value;
        for (int i = 0; i < 16; ++i) {
            w.remove(prev);
            const collider_id next = w.add(i, mk_static(box_at(0, 0)));
            CHECK(next.value == slot);                      // same slot reused each round
            CHECK(next.generation == prev.generation + 1u); // strictly increasing
            CHECK_FALSE(w.is_valid(prev));
            CHECK(w.is_valid(next));
            prev = next;
        }
    }

    TEST_CASE("churn: add/remove many, validity stays consistent") {
        world w;
        std::vector<collider_id> ids;
        for (int i = 0; i < 100; ++i) {
            ids.push_back(w.add(static_cast<entity_id_t>(i),
                                mk_kine(box_at(static_cast<float>(i), 0.0f), vec{1, 0})));
        }
        for (const auto& id : ids) {
            CHECK(w.is_valid(id));
        }

        // Remove the even-indexed ones; odd ones must remain valid, evens become invalid.
        for (std::size_t i = 0; i < ids.size(); i += 2) {
            w.remove(ids[i]);
        }
        for (std::size_t i = 0; i < ids.size(); ++i) {
            CHECK(w.is_valid(ids[i]) == (i % 2 == 1));
        }

        // Re-add as many as we removed: kept handles still valid, new handles valid,
        // and no new handle collides (slot+generation) with a still-live one.
        std::set<std::pair<uint32_t, uint32_t>> live;
        for (std::size_t i = 1; i < ids.size(); i += 2) {
            live.emplace(ids[i].value, ids[i].generation);
        }
        for (int i = 0; i < 50; ++i) {
            const collider_id n = w.add(1000 + i, mk_static(box_at(0, 0)));
            CHECK(w.is_valid(n));
            CHECK(live.emplace(n.value, n.generation).second); // unique among the living
        }
        for (std::size_t i = 1; i < ids.size(); i += 2) {
            CHECK(w.is_valid(ids[i]));
        }
    }
}

TEST_SUITE("world: mutator guards") {
    TEST_CASE("set_shape on a live body succeeds and keeps the handle valid") {
        world w;
        const collider_id a = w.add(1, mk_static(box_at(0, 0)));
        CHECK_NOTHROW(w.set_shape(a, box_at(10, 10, 2, 2)));
        CHECK_NOTHROW(w.set_shape(a, circle{{3, 3}, 1.5f})); // shape variant may change
        CHECK_NOTHROW(w.set_shape(a, segment{{0, 0}, {4, 1}}));
        CHECK(w.is_valid(a)); // mutation does not recycle the handle
    }

    TEST_CASE("set_velocity on a live kinematic body succeeds") {
        world w;
        const collider_id k = w.add(1, mk_kine(box_at(0, 0), vec{0, 0}));
        CHECK_NOTHROW(w.set_velocity(k, vec{3, -2}));
        CHECK(w.is_valid(k));
    }

    TEST_CASE("set_velocity on a static body throws (wrong kind)") {
        world w;
        const collider_id s = w.add(1, mk_static(box_at(0, 0)));
        CHECK_THROWS(w.set_velocity(s, vec{1, 0}));
    }

    TEST_CASE("mutating a stale handle throws") {
        world w;
        const collider_id k = w.add(1, mk_kine(box_at(0, 0), vec{1, 0}));
        w.remove(k);
        CHECK_THROWS(w.set_shape(k, box_at(0, 0)));
        CHECK_THROWS(w.set_velocity(k, vec{2, 2}));
    }
}

TEST_SUITE("world: construction") {
    TEST_CASE("custom config constructs and operates") {
        world_config cfg;
        cfg.fatten_margin = 0.5f;
        world w(cfg);
        const collider_id a = w.add(1, mk_kine(box_at(0, 0), vec{2, 0}));
        CHECK(w.is_valid(a));
        CHECK_NOTHROW(w.set_velocity(a, vec{0, -3}));
        w.remove(a);
        CHECK_FALSE(w.is_valid(a));
    }
}

TEST_SUITE("world: collider_id comparison & hashing") {
    TEST_CASE("aggregate init survives the defaulted comparisons") {
        const collider_id a{1, 0, collider_id::BODY}; // must still compile as an aggregate
        CHECK(a.value == 1u);
        CHECK(a.generation == 0u);
        CHECK(a.type_id == collider_id::BODY);
        CHECK(collider_id{}.valid() == false); // default = null sentinel
    }

    TEST_CASE("equality is full identity (value, generation, type)") {
        const collider_id a{1, 0, collider_id::BODY};
        CHECK(a == collider_id{1, 0, collider_id::BODY});
        CHECK(a != collider_id{2, 0, collider_id::BODY}); // value
        CHECK(a != collider_id{1, 1, collider_id::BODY}); // generation
        CHECK(a != collider_id{1, 0, collider_id::BULLET}); // type
    }

    TEST_CASE("ordering (<=>) is usable for std::set") {
        std::set<collider_id> s{
            {1, 0, collider_id::BODY},
            {1, 0, collider_id::BODY},   // duplicate -> deduped
            {1, 1, collider_id::BODY},
            {1, 0, collider_id::BULLET},
        };
        CHECK(s.size() == 3u);
        CHECK(s.count(collider_id{1, 0, collider_id::BODY}) == 1u);
        CHECK((collider_id{1, 0, collider_id::BODY} <=> collider_id{1, 1, collider_id::BODY})
              == std::strong_ordering::less);
    }

    TEST_CASE("std::hash keys unordered containers") {
        std::unordered_map<collider_id, int> m;
        m[collider_id{1, 0, collider_id::BODY}] = 7;
        m[collider_id{1, 1, collider_id::BODY}] = 9;   // distinct generation -> distinct key
        CHECK(m.size() == 2u);
        CHECK(m.at(collider_id{1, 0, collider_id::BODY}) == 7);
        CHECK(m.at(collider_id{1, 1, collider_id::BODY}) == 9);
    }
}
