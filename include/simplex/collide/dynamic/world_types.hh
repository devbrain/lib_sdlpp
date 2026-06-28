//
// Value types, handles, and internal storage that the `world` class is built on.
// Split out of world.hh for navigability (see COLLISION_WORLD_IMPLEMENTATION_PLAN refactor).
// Public vocabulary: shapes, materials, filters, body DTOs, collider_id, contact, footing,
// world_event, world_config. Internal (detail): the grid tile payload, resident/nonresident
// body records, the slot-map internal_storage, and eval_velocity_response.
//

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <type_traits>
#include <variant>
#include <vector>
#include <optional>

#include <simplex/collide/shapes.hh>
#include <simplex/collide/units.hh>
#include <simplex/collide/dynamic/grid.hh>
#include <simplex/collide/dynamic/dynamic_aabb_tree.hh>

namespace simplex::collide {
    // Full shape set -- static bodies and broadphase targets may be any of these.
    using shape_t = std::variant <segment, aabb, circle, triangle>;
    // Movers only -- a kinematic body / bullet is swept each frame, and a segment cannot be a
    // swept mover (that is raycast's job). Restricting the type makes an invalid mover
    // unconstructable, and lets cast's swept dispatch cover every combination with no guard.
    using moving_shape_t = std::variant <aabb, circle>;

    enum class response_mode {
        BLOCK,
        ONE_WAY,
        SENSOR,
        IGNORE
    };

    // this governs response function
    struct material_props {
        // in [0, 1]. restitution > 1 injects energy
        //     relative velocity of separation after collision
        float restitution{0}; // e = --------------------------------------------------
        //     relative velocity of approach before collision
        //
        float friction{0}; // in [0, 1], friction > 1 reverses the tangential motion

        response_mode response{response_mode::BLOCK};
        vec block_normal{0, 1}; // you can not fall through the floor
    };

    struct filter_props {
        uint16_t category = 0xFFFF;
        uint16_t mask = 0xFFFF;
    };

    // Client DTOs (transient input to world::add). No inheritance: a kinematic/bullet shape is
    // a *narrower* type than a static one, so they are independent structs.
    struct static_body {
        shape_t shape;
        material_props material;
        filter_props filter;
    };

    struct kinematic_body {
        moving_shape_t shape;
        material_props material;
        filter_props filter;
        vec velocity;
    };

    // A carrier ("solid" in the actors-and-solids model): a kinematic body that moves RIGIDLY on a
    // scripted path and transports the actors riding it -- moving platforms, elevators, crushers,
    // conveyors. Distinct type from kinematic_body purely so world::add tags it CARRIER. Each frame
    // it translates by `velocity*dt`, and a rider on top inherits `(velocity + surface_velocity)*dt`:
    //   moving platform -> surface_velocity {0,0}; conveyor -> velocity {0,0}, surface_velocity = belt
    //   speed; a moving conveyor sets both (the rider gets the sum).
    struct carrier_body {
        moving_shape_t shape;
        material_props material;
        filter_props filter;
        vec velocity{};         // the carrier's own rigid motion
        vec surface_velocity{}; // tangential drag imparted to riders without moving the carrier
    };

    struct bullet {
        moving_shape_t shape;
        material_props material;
        filter_props filter;
        vec velocity;
    };

    // A static tile: identical fields to static_body, distinct type purely so world::add routes
    // it into the internal static grid instead of the BVH. The shape is bucketed into the single
    // grid cell containing its centre, so a tile must fit within one cell -- larger or free-form
    // statics belong in static_body (BVH). Slopes are just `segment` tiles, like resident walls.
    struct tile_body {
        shape_t shape;
        material_props material;
        filter_props filter;
        // Opt a plain static solid tile into boundary-compilation: on the first run() the world
        // merges adjacent mergeable cell-filling BLOCK aabb tiles (same material+filter) into bigger
        // AABB residents, removing the internal tile seams that snag fast movers. Merged tiles lose
        // their per-cell TILE handle (it goes invalid), so leave this false for tiles you address
        // individually (destructibles, sensors, slopes -- which also can't merge anyway).
        bool mergeable = false;
    };

    namespace detail {
        // The static grid's cell payload: the tile's geometry stored verbatim (so get_shape is a
        // straight read), plus its material/filter and identity eid (for events, like residents).
        struct tile {
            shape_t shape{aabb{}};
            material_props material;
            filter_props filter;
            entity_id_t eid{};
            bool mergeable{false}; // opted into boundary-compilation (§19 #4)
        };
    }

    namespace detail {
        enum class body_kind {
            STATIC,
            KINEMATIC,
            CARRIER // a kinematic "solid": moves rigidly on a path and carries/pushes riders
        };

        struct resident_body {
            shape_t shape{aabb{}};
            material_props material;
            filter_props filter;
            vec velocity{0, 0};
            vec surface_velocity{0, 0}; // CARRIER only: tangential drag imparted to riders (conveyor)
            entity_id_t eid{};
            body_kind kind{body_kind::STATIC};
            node_ptr proxy{}; // broadphase handle
            uint32_t generation{0};
            bool alive{true};
        };

        struct nonresident_body {
            moving_shape_t shape{aabb{}}; // bullets are always movers -> tight variant
            material_props material;
            filter_props filter;
            vec velocity{0, 0};
            entity_id_t eid{};
            uint32_t generation{0};
            bool alive{true};
        };

        template<typename T>
        concept has_generation = requires(T obj)
        {
            obj.generation;
        };

        template<typename Body>
        class internal_storage {
            public:
                internal_storage() = default;

                uint32_t allocate() {
                    if (!m_free.empty()) {
                        auto idx = m_free.back();
                        m_free.pop_back();
                        ENFORCE(idx < m_pool.size());
                        init(m_pool[idx]);
                        return idx;
                    }
                    m_pool.emplace_back();
                    init(m_pool.back());
                    return static_cast <uint32_t>(m_pool.size() - 1);
                }

                void deallocate(uint32_t idx) {
                    ENFORCE(idx < m_pool.size());
                    ENFORCE(m_pool[idx].alive);
                    m_pool[idx].alive = false;
                    if constexpr (has_generation <Body>) {
                        ++m_pool[idx].generation;
                    }
                    m_free.push_back(idx);
                }

                // Drop every element. Each live slot is deallocated (so its generation bumps and the
                // free list is rebuilt), leaving all generations MONOTONIC -- a handle held across a
                // clear never aliases a slot reused afterwards. Capacity is retained for reuse.
                void clear() {
                    for (uint32_t i = 0; i < m_pool.size(); ++i) {
                        if (m_pool[i].alive) {
                            deallocate(i);
                        }
                    }
                }

                Body& operator[](uint32_t idx) {
                    return m_pool[idx];
                }

                const Body& operator[](uint32_t idx) const {
                    return m_pool[idx];
                }

                [[nodiscard]] bool is_alive(uint32_t idx) const { return idx < m_pool.size() && m_pool[idx].alive; }

                // Generation of slot `idx`. For a Body without a generation field the
                // concept is false and there is nothing to recycle-detect, so 0 is returned
                // (validity then reduces to liveness). Keeps every .generation access guarded.
                [[nodiscard]] uint32_t generation([[maybe_unused]] uint32_t idx) const {
                    if constexpr (has_generation <Body>) {
                        return m_pool[idx].generation;
                    } else {
                        return 0;
                    }
                }

                // ---- live-only iteration ------------------------------------------------
                // Forward iterator over the *live* slots (dead slots from deallocate() are
                // skipped). Marking a slot dead via deallocate() does not move storage, so
                // an in-flight iterator stays valid -- you can deallocate(it.index()) the
                // current element mid-loop and continue. The iterator dereferences through
                // the vector each step, so it also survives a reallocation from allocate().
                template<bool Const>
                class iterator_t {
                    using pool_t = std::conditional_t <Const, const std::vector <Body>, std::vector <Body>>;
                    pool_t* m_pool = nullptr;
                    std::size_t m_idx = 0;

                    void skip_dead() {
                        while (m_idx < m_pool->size() && !(*m_pool)[m_idx].alive) {
                            ++m_idx;
                        }
                    }

                    public:
                        using value_type = Body;
                        using reference = std::conditional_t <Const, const Body&, Body&>;
                        using pointer = std::conditional_t <Const, const Body*, Body*>;
                        using difference_type = std::ptrdiff_t;
                        using iterator_category = std::forward_iterator_tag;

                        iterator_t() = default;

                        iterator_t(pool_t* pool, std::size_t idx)
                            : m_pool(pool), m_idx(idx) { skip_dead(); }

                        [[nodiscard]] reference operator*() const { return (*m_pool)[m_idx]; }
                        [[nodiscard]] pointer operator->() const { return &(*m_pool)[m_idx]; }

                        // Slot index of the current element -- pass to deallocate() to kill it.
                        [[nodiscard]] uint32_t index() const { return static_cast <uint32_t>(m_idx); }

                        iterator_t& operator++() {
                            ++m_idx;
                            skip_dead();
                            return *this;
                        }

                        iterator_t operator++(int) {
                            auto tmp = *this;
                            ++(*this);
                            return tmp;
                        }

                        [[nodiscard]] bool operator==(const iterator_t& o) const { return m_idx == o.m_idx; }
                        [[nodiscard]] bool operator!=(const iterator_t& o) const { return m_idx != o.m_idx; }
                };

                using iterator = iterator_t <false>;
                using const_iterator = iterator_t <true>;

                [[nodiscard]] iterator begin() { return iterator(&m_pool, 0); }
                [[nodiscard]] iterator end() { return iterator(&m_pool, m_pool.size()); }
                [[nodiscard]] const_iterator begin() const { return const_iterator(&m_pool, 0); }
                [[nodiscard]] const_iterator end() const { return const_iterator(&m_pool, m_pool.size()); }
                [[nodiscard]] const_iterator cbegin() const { return begin(); }
                [[nodiscard]] const_iterator cend() const { return end(); }

            private:
                static void init(Body& b) {
                    if constexpr (has_generation <Body>) {
                        const uint32_t gen = b.generation; // the one thing that must survive
                        b = Body{}; // clear shape/velocity/proxy/kind/...
                        b.generation = gen;
                        b.alive = true;
                    } else {
                        b = Body{}; // clear shape/velocity/proxy/kind/...
                        b.alive = true;
                    }
                }

            private:
                std::vector <Body> m_pool;
                std::vector <uint32_t> m_free;
        };

        using bodies_storage = internal_storage <resident_body>;
        using bullets_storage = internal_storage <nonresident_body>;

        // shape_t -> moving_shape_t. The segment case is unreachable for movers (the typed
        // DTOs guarantee it at construction); ENFORCE is a never-fires safety net.
        constexpr moving_shape_t narrow(const shape_t& s) {
            return std::visit([]<typename T0>(const T0& shp) -> moving_shape_t {
                using S = std::decay_t <T0>;
                if constexpr (std::is_same_v <S, aabb> || std::is_same_v <S, circle>) {
                    return moving_shape_t{shp};
                } else {
                    // segment or triangle: not a valid mover (movers are aabb | circle)
                    ENFORCE(false)("a moving body/bullet must be an aabb or circle");
                    return moving_shape_t{aabb{}}; // unreachable; satisfies the return type
                }
            }, s);
        }

        // moving_shape_t -> shape_t (widening; always valid).
        constexpr shape_t widen(const moving_shape_t& s) {
            return std::visit([](const auto& shp) -> shape_t { return shp; }, s);
        }

        // Geometric velocity response at a contact: the post-collision velocity after removing
        // the blocked (into-surface) component and applying the surface material.
        //
        //   v_n = dot(v, n) * n        -- component of v along the contact normal
        //   v_t = v - v_n              -- component tangent to the surface
        //   v'  = (1 - friction) * v_t -- friction damps sliding (0 = frictionless "ice", 1 = no slide)
        //       - restitution * v_n    -- restitution reflects the normal part (0 = absorb/slide, 1 = bounce)
        //
        // This is purely the BLOCK / bounce math. WHETHER a contact is resolved at all
        // (BLOCK vs ONE_WAY vs SENSOR/IGNORE) is the caller's decision via material.response;
        // and `material` must be the SURFACE's (the body hit), per the surface-owned response
        // model. Position sliding is handled separately (move_and_slide projects the leftover
        // displacement); this function affects velocity only.
        //
        // Preconditions / notes:
        //   * `n` MUST be unit length -- swept_hit::entry_normal is. A non-unit `n` mis-scales
        //     the projection (by |n|^2).
        //   * `n == {0,0}` (swept_hit's "undefined normal") degrades to (1 - friction) * v: no
        //     normal removal, no bounce. Callers should skip the response on a zero normal.
        //   * `friction` and `restitution` are expected in [0, 1]. friction > 1 reverses the
        //     tangential motion; restitution > 1 injects energy.
        //
        // NB: the intermediates are `vec`, not `auto` -- euler is an expression-template library,
        // so `auto` would capture a lazy expression that can dangle past this scope.
        inline
        units::velocity eval_velocity_response(units::velocity vel, const vec& n,
                                               const material_props& material) {
            const vec v = vel.value;
            const vec v_n = euler::dot(v, n) * n; // component along the contact normal
            const vec v_t = v - v_n; // component tangent to the surface
            return units::velocity{(1.0f - material.friction) * v_t - material.restitution * v_n};
        }
    }

    struct collider_id {
        enum type {
            BODY,
            BULLET,
            TILE // a static grid cell; `value` is the linear cell handle (eid lives in the payload)
        };

        static constexpr uint32_t INVALID = 0xFFFFFFFFu;

        // Defaults make a value-initialised handle (collider_id{}) the null sentinel rather than
        // aliasing slot 0 generation 0. world::is_valid() also rejects it (INVALID is out of
        // range for any pool). Aggregate init collider_id{idx, gen, type} is unaffected.
        uint32_t value = INVALID;
        uint32_t generation = 0;
        type type_id = BODY;

        // True unless this is the null sentinel. (Distinct from world::is_valid, which also
        // checks liveness + generation against the storage.)
        [[nodiscard]] bool valid() const { return value != INVALID; }

        // Full-identity comparison (value, generation, type_id in that order). Defaulting these
        // keeps collider_id an aggregate (no user-provided ctor), so collider_id{idx, gen, type}
        // still works. == / != are usable as map keys; <=> gives ordering for std::set / sorting.
        [[nodiscard]] friend bool operator==(const collider_id&, const collider_id&) = default;
        [[nodiscard]] friend std::strong_ordering operator<=>(const collider_id&, const collider_id&) = default;
    };

    struct contact {
        collider_id who; // the resident hit
        vec normal; // outward surface normal at impact (for slide / ricochet)
        float toi; // time of impact, normalized [0,1] along delta
    };

    // The result of world::ground_support (§19 #4): the WALKABLE ground found under each of three
    // footprint probes -- left edge, centre, right edge (Sonic's twin floor sensors are the
    // left/right pair). Each is the probe's contact, or nullopt when that foot hangs over nothing
    // walkable within reach (a void, a sensor, or a too-steep face). A STATE the game reads to drive
    // teeter/balance, edge-stop, coyote-time and ledge-grab -- not a solver behaviour.
    struct footing {
        std::optional <contact> left;
        std::optional <contact> centre;
        std::optional <contact> right;

        [[nodiscard]] bool grounded() const noexcept { return left || centre || right; }
        [[nodiscard]] bool fully_supported() const noexcept { return left && centre && right; }
        // At a ledge: standing on something, but part of the footprint hangs off (the teeter cue).
        [[nodiscard]] bool at_ledge() const noexcept { return grounded() && !fully_supported(); }
        // The drop is off the unsupported side: true iff the left foot hangs (mirror for right).
        [[nodiscard]] bool ledge_left() const noexcept { return !left && (centre || right); }
        [[nodiscard]] bool ledge_right() const noexcept { return !right && (centre || left); }
    };

    enum class event_kind {
        COLLISION,
        BULLET_HIT,
        BULLET_EXPIRED, // a bullet left the world bounds; the game should despawn it
        TRIGGER_BEGIN,
        TRIGGER_END,
        CRUSH // a carrier pinned an actor against other solid geometry (the actor couldn't move clear)
    };

    struct world_event {
        world_event(event_kind kind_, const collider_id& mover_, const collider_id& target_, const vec& normal_,
                    float toi_)
            : kind(kind_),
              mover(mover_),
              target(target_),
              normal(normal_),
              toi(toi_) {
        }

        event_kind kind{event_kind::COLLISION};
        collider_id mover{};
        collider_id target{};
        vec normal{};
        float toi{-1.0f}; // time to impact
    };

    struct world_config {
        float fatten_margin = 0.1f;
        float skin = 0.01f; // back-off so the mover never quite touches (anti-jitter)
        int max_slide_iter = 4; // corner passes (floor+wall needs 2)
        vec up = {0, 1}; // for grounded detection (matches block_normal default)
        std::optional <aabb> bounds{}; // play-field extent; unset = unbounded. When set, a bullet
        // that leaves it gets a BULLET_EXPIRED event (the game despawns it). Bullets only --
        // kinematic bodies are never auto-expired. This is the LEVEL extent, NOT the camera
        // (active_region): a bullet scrolling off-camera is culled, not expired.

        // Static tile grid. Unset = pure-dynamic world (no grid; add(tile_body) is rejected). When
        // set, the grid's extent IS `bounds` (one shared world box -- no separate, mismatched grid
        // box), divided into cells of `tile_size`. `bounds` must therefore be present and an integer
        // multiple of `tile_size` (enforced at construction).
        struct grid_config {
            vec tile_size{1, 1};
        };
        std::optional <grid_config> grid{};
    };

    // Test-only access to the world's internal (non-public) query helpers. Defined only in the
    // test TU; lets the brute-force suite reach `cast`/`overlap` without widening the API.
    struct world_test_access;

    namespace detail {
        // Tight (un-fattened) bounding box of a shape -- replaces the repeated
        // std::visit([](const auto& s){ return enclose(s); }, shape) across the world.
        [[nodiscard]] inline aabb tight_box(const shape_t& s) {
            return std::visit([](const auto& x) { return enclose(x); }, s);
        }
        [[nodiscard]] inline aabb tight_box(const moving_shape_t& s) {
            return std::visit([](const auto& x) { return enclose(x); }, s);
        }
    }
}
