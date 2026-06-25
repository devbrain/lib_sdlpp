//
// Created by igor on 21/06/2026.
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
    };

    namespace detail {
        // The static grid's cell payload: the tile's geometry stored verbatim (so get_shape is a
        // straight read), plus its material/filter and identity eid (for events, like residents).
        struct tile {
            shape_t shape{aabb{}};
            material_props material;
            filter_props filter;
            entity_id_t eid{};
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

    enum class event_kind {
        COLLISION,
        BULLET_HIT,
        BULLET_EXPIRED, // a bullet left the world bounds; the game should despawn it
        TRIGGER_BEGIN,
        TRIGGER_END
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

    class world {
        public:
            world() = default;

            explicit world(const world_config& cfg)
                : m_cfg(cfg) {
                if (m_cfg.grid) {
                    // The grid shares the world's extent (one coordinate frame). Require bounds and
                    // an exact tiling -- a non-dividing extent is a config error, caught here loudly
                    // rather than silently clamping tiles to a mismatched box.
                    ENFORCE(m_cfg.bounds)("world_config.grid requires world_config.bounds (the shared extent)");
                    const aabb& b = *m_cfg.bounds;
                    const vec ts = m_cfg.grid->tile_size;
                    ENFORCE(ts.x() > 0.0f && ts.y() > 0.0f)("grid tile_size must be positive");
                    const float fcols = (b.max.x() - b.min.x()) / ts.x();
                    const float frows = (b.max.y() - b.min.y()) / ts.y();
                    const auto cols = static_cast <uint32_t>(std::lround(fcols));
                    const auto rows = static_cast <uint32_t>(std::lround(frows));
                    ENFORCE(cols > 0 && rows > 0)("grid extent (bounds / tile_size) must be at least one cell");
                    ENFORCE(std::abs(fcols - static_cast <float>(cols)) < 1e-3f
                            && std::abs(frows - static_cast <float>(rows)) < 1e-3f)
                            ("world_config.bounds must be an integer multiple of grid tile_size");
                    m_static_grid.emplace(grid <detail::tile>::from_tile_size(b.min, ts, cols, rows));
                }
            }

            friend struct world_test_access;

            collider_id add(entity_id_t eid, const static_body& body) {
                auto idx = m_bodies_storage.allocate();
                auto& stored = m_bodies_storage[idx];

                stored.shape = body.shape;
                stored.kind = detail::body_kind::STATIC;
                stored.filter = body.filter;
                stored.material = body.material;
                stored.eid = eid;

                auto box = fatten(stored);
                stored.proxy = insert_leaf(m_space_partition, idx, box);
                return {idx, m_bodies_storage.generation(idx), collider_id::BODY};
            }

            collider_id add(entity_id_t eid, const kinematic_body& body) {
                auto idx = m_bodies_storage.allocate();
                auto& stored = m_bodies_storage[idx];

                stored.shape = detail::widen(body.shape); // moving_shape_t -> shape_t (always valid)
                stored.kind = detail::body_kind::KINEMATIC;
                stored.filter = body.filter;
                stored.material = body.material;
                stored.velocity = body.velocity;
                stored.eid = eid;

                auto box = fatten(stored);
                stored.proxy = insert_leaf(m_space_partition, idx, box);
                return {idx, m_bodies_storage.generation(idx), collider_id::BODY};
            }

            collider_id add(entity_id_t eid, const carrier_body& body) {
                auto idx = m_bodies_storage.allocate();
                auto& stored = m_bodies_storage[idx];

                stored.shape = detail::widen(body.shape);
                stored.kind = detail::body_kind::CARRIER;
                stored.filter = body.filter;
                stored.material = body.material;
                stored.velocity = body.velocity;
                stored.surface_velocity = body.surface_velocity;
                stored.eid = eid;

                auto box = fatten(stored);
                stored.proxy = insert_leaf(m_space_partition, idx, box);
                return {idx, m_bodies_storage.generation(idx), collider_id::BODY};
            }

            collider_id add(entity_id_t eid, const bullet& body) {
                auto idx = m_bullets_storage.allocate();
                auto& stored = m_bullets_storage[idx];

                stored.shape = body.shape;
                stored.filter = body.filter;
                stored.material = body.material;
                stored.velocity = body.velocity;
                stored.eid = eid;
                return {idx, m_bullets_storage.generation(idx), collider_id::BULLET};
            }

            // Add a static tile into the grid. The tile is bucketed into the single cell containing
            // its shape's centre (so the shape must fit within one cell, enforced below). Overwrites
            // any tile already in that cell (loader-friendly); the overwrite bumps the cell's
            // generation, so a handle to the previous tile goes invalid (no silent aliasing). The
            // returned handle's `value` is the linear cell index, `generation` is the cell's; `eid`
            // is recovered from the payload.
            collider_id add(entity_id_t eid, const tile_body& body) {
                ENFORCE(m_static_grid)("add(tile_body) requires a grid (world_config.grid)");
                const aabb bound = std::visit([](const auto& s) { return enclose(s); }, body.shape);
                const vec centre = bound.center();
                const uint32_t cell = m_static_grid->to_cell(centre);
                ENFORCE(cell != grid<detail::tile>::INVALID_CELL)("tile centre is outside the grid bounds");
                // A tile is only stored in (and only found via) its centre's cell, so it must fit
                // within that cell -- otherwise queries through the overhang would silently miss it.
                ENFORCE(detail::contains(m_static_grid->cell_box_at(cell), bound))
                        ("tile shape must fit within a single grid cell");
                m_static_grid->set(centre, detail::tile{body.shape, body.material, body.filter, eid});
                // Stamp the post-set generation: a later overwrite/clear of this cell bumps it,
                // so this handle then reads as invalid instead of silently aliasing the new tile.
                const collider_id id{cell, m_static_grid->cell_generation(cell), collider_id::TILE};
                // Track sensor tiles so the trigger pass can scan them (bodies-only loop can't).
                // Stale entries (cell overwritten/cleared) are pruned lazily in the trigger pass.
                if (body.material.response == response_mode::SENSOR) {
                    m_sensor_tiles.push_back(id);
                }
                return id;
            }

            void remove(collider_id cid) {
                if (!is_valid(cid)) {
                    return;
                }
                if (cid.type_id == collider_id::BODY) {
                    auto& stored = m_bodies_storage[cid.value];
                    remove_leaf(m_space_partition, stored.proxy);
                    m_bodies_storage.deallocate(cid.value);
                } else if (cid.type_id == collider_id::BULLET) {
                    m_bullets_storage.deallocate(cid.value);
                } else { // TILE
                    m_static_grid->clear_at(cid.value);
                }
            }

            // Empty the world for a level reload: drops every body, bullet and tile, the broadphase
            // tree, and the trigger/event buffers, keeping the configuration (bounds, grid, skin,
            // ...) intact. All storages keep their generation counters monotonic, so any handle held
            // across clear() reads as invalid afterwards rather than aliasing a reused slot/cell.
            // Capacity is retained for reuse.
            void clear() {
                m_bodies_storage.clear();
                m_bullets_storage.clear();
                m_space_partition.reset();
                if (m_static_grid) {
                    m_static_grid->reset();
                }
                m_events.clear();
                m_triggers_curr.clear();
                m_triggers_prev.clear();
                m_sensor_tiles.clear();
            }

            // resize/teleport. Takes the wide shape_t; a moving body cannot become a segment,
            // which (unlike construction) can only be enforced at runtime here since `cid`'s
            // kind is not known at compile time.
            void set_shape(collider_id cid, const shape_t& shape) {
                ENFORCE(is_valid(cid));
                if (cid.type_id == collider_id::BODY) {
                    auto& stored = m_bodies_storage[cid.value];
                    if (stored.kind != detail::body_kind::STATIC) {
                        // any mover (KINEMATIC actor or CARRIER) must stay an aabb | circle -- a
                        // segment/triangle would later trip detail::narrow() in the move/carrier pass.
                        ENFORCE(std::holds_alternative<aabb>(shape) || std::holds_alternative<circle>(shape));
                    }
                    stored.shape = shape;
                    auto box = fatten(stored);
                    update_leaf(m_space_partition, stored.proxy, box);
                } else if (cid.type_id == collider_id::BULLET) {
                    auto& stored = m_bullets_storage[cid.value];
                    stored.shape = detail::narrow(shape); // ENFORCE non-segment + shape_t -> moving_shape_t
                } else { // TILE: reshape in place. The new shape must still fit the same cell
                         // (no re-bucketing) -- else it could overhang into a cell that won't find it.
                    const aabb nb = std::visit([](const auto& s) { return enclose(s); }, shape);
                    ENFORCE(detail::contains(m_static_grid->cell_box_at(cid.value), nb))
                            ("set_shape: a tile's new shape must fit within its cell");
                    m_static_grid->at(cid.value)->shape = shape;
                }
            }

            // kinematic intent for next run()
            void set_velocity(collider_id cid, const vec& v) {
                ENFORCE(is_valid(cid));
                // Tiles are static -- no velocity. (A moving tile is a kinematic body, not a tile.)
                ENFORCE(cid.type_id != collider_id::TILE)("a tile has no velocity");
                if (cid.type_id == collider_id::BODY) {
                    auto& stored = m_bodies_storage[cid.value];
                    // A static body has no velocity; kinematic actors AND carriers (scripted paths) do.
                    ENFORCE(stored.kind != detail::body_kind::STATIC)("a static body has no velocity");
                    stored.velocity = v;
                } else {
                    auto& stored = m_bullets_storage[cid.value];
                    stored.velocity = v;
                }
            }

            // Carrier-only: the tangential drag imparted to riders (a conveyor's belt speed; set to
            // {0,0} to switch the belt off). The carrier's own motion is set via set_velocity.
            void set_surface_velocity(collider_id cid, const vec& v) {
                ENFORCE(is_valid(cid) && cid.type_id == collider_id::BODY);
                auto& stored = m_bodies_storage[cid.value];
                ENFORCE(stored.kind == detail::body_kind::CARRIER)("surface_velocity is carrier-only");
                stored.surface_velocity = v;
            }

            [[nodiscard]] bool is_valid(collider_id cid) const {
                if (cid.type_id == collider_id::BODY) {
                    return m_bodies_storage.is_alive(cid.value)
                           && m_bodies_storage.generation(cid.value) == cid.generation;
                }
                if (cid.type_id == collider_id::BULLET) {
                    return m_bullets_storage.is_alive(cid.value)
                           && m_bullets_storage.generation(cid.value) == cid.generation;
                }
                // TILE: live iff the grid exists, the cell is occupied, AND the cell has not been
                // mutated (overwritten/cleared) since this handle was made -- the generation check
                // closes the stale-handle alias.
                return m_static_grid && m_static_grid->at(cid.value) != nullptr
                       && m_static_grid->cell_generation(cid.value) == cid.generation;
            }

            // ---- read-back getters (state after run()/move) -------------------------
            // A character controller reads these each frame: the resolved shape/position and the
            // post-move velocity. All ENFORCE a live handle. Returned by value -- the underlying
            // record may be recycled later. Shapes come back as the wide shape_t (a bullet's
            // moving_shape_t is widened) so the caller has one type to visit.

            [[nodiscard]] shape_t get_shape(collider_id cid) const {
                ENFORCE(is_valid(cid));
                if (cid.type_id == collider_id::BODY) {
                    return m_bodies_storage[cid.value].shape;
                }
                if (cid.type_id == collider_id::BULLET) {
                    return detail::widen(m_bullets_storage[cid.value].shape);
                }
                return m_static_grid->at(cid.value)->shape; // TILE: stored verbatim
            }

            [[nodiscard]] vec get_velocity(collider_id cid) const {
                ENFORCE(is_valid(cid));
                if (cid.type_id == collider_id::BODY) {
                    return m_bodies_storage[cid.value].velocity;
                }
                if (cid.type_id == collider_id::BULLET) {
                    return m_bullets_storage[cid.value].velocity;
                }
                return vec{0, 0}; // TILE: static
            }

            // The game's entity id carried as payload (the reverse of add()'s eid argument).
            [[nodiscard]] entity_id_t get_eid(collider_id cid) const {
                ENFORCE(is_valid(cid));
                if (cid.type_id == collider_id::BODY) {
                    return m_bodies_storage[cid.value].eid;
                }
                if (cid.type_id == collider_id::BULLET) {
                    return m_bullets_storage[cid.value].eid;
                }
                return m_static_grid->at(cid.value)->eid; // TILE: from the cell payload
            }

            [[nodiscard]] const std::vector <world_event>& run(const aabb& active_region, float dt) {
                m_events.clear();

                // Carrier pass (actors-and-solids): each carrier moves RIGIDLY on its scripted path
                // and transports the actors riding it. Runs BEFORE the actor movement pass, so actors
                // then see carriers at their resolved positions. A rider inherits
                // (velocity + surface_velocity)*dt -- the platform's own delta plus any conveyor drag
                // -- collision-aware against everything but its carrier. (MP1: carrying only; pushing
                // and crushing are follow-ups.) Carriers are body_kind::CARRIER, so the movement pass
                // below (which only processes KINEMATIC) leaves them alone.
                for (auto it = m_bodies_storage.begin(); it != m_bodies_storage.end(); ++it) {
                    if (it->kind != detail::body_kind::CARRIER) {
                        continue;
                    }
                    const vec body_delta{it->velocity.x() * dt, it->velocity.y() * dt};
                    const vec rider_delta{(it->velocity.x() + it->surface_velocity.x()) * dt,
                                          (it->velocity.y() + it->surface_velocity.y()) * dt};
                    if (near_zero(body_delta) && near_zero(rider_delta)) {
                        continue; // a stationary carrier with no belt: nothing to do
                    }
                    if (!intersects(swept_bound(detail::narrow(it->shape), units::displacement{body_delta}),
                                    active_region)) {
                        continue; // off-region carrier: dormant (like off-region movers)
                    }
                    const uint32_t carrier_idx = it.index();
                    // Collect riders at the carrier's CURRENT position, before it moves. (Linear scan
                    // -- carriers are few; a tree query is a perf follow-up.)
                    m_rider_scratch.clear();
                    for (auto jt = m_bodies_storage.begin(); jt != m_bodies_storage.end(); ++jt) {
                        if (jt->kind == detail::body_kind::KINEMATIC && is_riding(jt.index(), carrier_idx)) {
                            m_rider_scratch.push_back(jt.index());
                        }
                    }
                    move_carrier_rigid(carrier_idx, body_delta);
                    for (const uint32_t r : m_rider_scratch) {
                        carry_translate(r, carrier_idx, rider_delta);
                    }
                }

                // Movement pass: resolve each kinematic mover via move-and-slide against the
                // solid residents, in fixed slot order (deterministic). Off-region movers are
                // culled (skipped, so they stay dormant off-screen); statics and zero-velocity
                // bodies never move. Each recorded contact becomes a COLLISION event. Runs BEFORE
                // the bullet pass so bullets sweep against movers at their resolved positions.
                for (auto it = m_bodies_storage.begin(); it != m_bodies_storage.end(); ++it) {
                    if (it->kind != detail::body_kind::KINEMATIC) {
                        continue;
                    }
                    const units::displacement delta = units::velocity{it->velocity} * units::duration{dt};
                    if (near_zero(delta.value)) {
                        continue; // not moving this frame
                    }
                    if (!intersects(swept_bound(detail::narrow(it->shape), delta), active_region)) {
                        continue; // off-region: dormant
                    }
                    const uint32_t mover_idx = it.index();
                    const slide_result res = move_and_slide(mover_idx, units::duration{dt}, solid_acceptor());
                    for (int i = 0; i < res.count; ++i) {
                        m_events.emplace_back(
                            event_kind::COLLISION,
                            collider_id{mover_idx, m_bodies_storage.generation(mover_idx), collider_id::BODY},
                            res.contacts[i].who,
                            res.contacts[i].normal,
                            res.contacts[i].toi);
                    }
                }

                // Bullet pass: integrate every live bullet, but only pay for the cast when its
                // swept bound touches the active region (off-region bullets keep flying so they
                // can re-enter, they just skip the expensive tree query). The game despawns
                // out-of-bounds bullets so they do not accumulate. toi is reported normalized.
                for (auto bullet_itr = m_bullets_storage.begin(); bullet_itr != m_bullets_storage.end(); ++bullet_itr) {
                    units::displacement delta_s = units::velocity{bullet_itr->velocity} * units::duration{dt};
                    if (intersects(swept_bound(bullet_itr->shape, delta_s), active_region)) {
                        // Bullets hit only solids -- a sensor/ignored body must not stop them
                        // (sensors detect via the trigger pass; bullets are not in the tree anyway).
                        if (auto hit = cast(bullet_itr.index(), collider_id::BULLET, delta_s, solid_acceptor())) {
                            m_events.emplace_back(
                                event_kind::BULLET_HIT,
                                collider_id{bullet_itr.index(), bullet_itr->generation, collider_id::BULLET},
                                hit->who,
                                hit->normal,
                                hit->toi);
                            delta_s = delta_s * units::fraction{hit->toi}; // toi is the fraction ALONG delta_s
                        }
                    }
                    translate(*bullet_itr, delta_s.value);

                    // Out-of-bounds: a bullet that no longer overlaps the world bounds has left
                    // the level -> report it (the game despawns in reaction; the handle is still
                    // live this frame). Only checked when bounds are configured.
                    if (m_cfg.bounds
                        && !intersects(swept_bound(bullet_itr->shape, units::displacement{}), *m_cfg.bounds)) {
                        m_events.emplace_back(
                            event_kind::BULLET_EXPIRED,
                            collider_id{bullet_itr.index(), bullet_itr->generation, collider_id::BULLET},
                            collider_id{}, vec{}, -1.0f);
                    }
                }

                // Trigger pass: detect SENSOR overlaps and diff against last frame to emit only
                // the begin/end EDGES. (response_mode is the classifier: solids were handled by
                // the movement pass as COLLISION; sensors are reported here -- there is no generic
                // per-frame "touch" event, continuous effects are the game's job between edges.)
                // Sensors are NOT region-culled: culling would spuriously fire end/begin as zones
                // scroll. The sensor's own filter decides what it senses.
                m_triggers_curr.clear();
                for (auto it = m_bodies_storage.begin(); it != m_bodies_storage.end(); ++it) {
                    if (it->material.response != response_mode::SENSOR) {
                        continue;
                    }
                    const uint32_t sidx = it.index();
                    const collider_id sensor_id{sidx, m_bodies_storage.generation(sidx), collider_id::BODY};
                    overlap(sidx, [&](collider_id other) {
                        m_triggers_curr.push_back({pair_key(sensor_id, other), sensor_id, other});
                    });
                }
                // Sensor TILES: the bodies loop above can't see them. Scan the side-list, pruning
                // stale entries (cell overwritten/cleared since -> generation mismatch) by swap-pop.
                // A sensor tile senses bodies only (query_tiles=false): tiles are static, so
                // tile-vs-tile overlaps never change frame to frame and carry no trigger meaning.
                for (std::size_t i = 0; i < m_sensor_tiles.size();) {
                    const collider_id st = m_sensor_tiles[i];
                    if (!is_valid(st)) {
                        m_sensor_tiles[i] = m_sensor_tiles.back();
                        m_sensor_tiles.pop_back();
                        continue;
                    }
                    const detail::tile& t = *m_static_grid->at(st.value);
                    overlap_core(t.shape, t.filter, collider_id::INVALID, /*query_tiles=*/false,
                                 [&](collider_id other) {
                                     m_triggers_curr.push_back({pair_key(st, other), st, other});
                                 });
                    ++i;
                }
                std::sort(m_triggers_curr.begin(), m_triggers_curr.end(),
                          [](const sensor_pair& a, const sensor_pair& b) { return a.key < b.key; });
                // Dedup: a sensor-vs-sensor overlap is produced once from each side (same key).
                m_triggers_curr.erase(
                    std::unique(m_triggers_curr.begin(), m_triggers_curr.end(),
                                [](const sensor_pair& a, const sensor_pair& b) { return a.key == b.key; }),
                    m_triggers_curr.end());

                // Linear merge of the two key-sorted sets: curr-only -> BEGIN, prev-only -> END,
                // in-both -> still inside (no event). END uses the PREVIOUS frame's stored ids, so
                // a pair that vanished because a body was removed still reports a clean end.
                std::size_t i = 0, j = 0;
                while (i < m_triggers_curr.size() && j < m_triggers_prev.size()) {
                    if (m_triggers_curr[i].key < m_triggers_prev[j].key) {
                        emit_trigger(event_kind::TRIGGER_BEGIN, m_triggers_curr[i++]);
                    } else if (m_triggers_prev[j].key < m_triggers_curr[i].key) {
                        emit_trigger(event_kind::TRIGGER_END, m_triggers_prev[j++]);
                    } else {
                        ++i;
                        ++j;
                    }
                }
                for (; i < m_triggers_curr.size(); ++i) {
                    emit_trigger(event_kind::TRIGGER_BEGIN, m_triggers_curr[i]);
                }
                for (; j < m_triggers_prev.size(); ++j) {
                    emit_trigger(event_kind::TRIGGER_END, m_triggers_prev[j]);
                }
                std::swap(m_triggers_curr, m_triggers_prev); // this frame becomes "previous"

                return m_events;
            }

        private:
            [[nodiscard]] aabb fatten(const detail::resident_body& body) const {
                auto box = std::visit([](const auto& shape) {
                    return enclose(shape);
                }, body.shape);

                if (body.kind == detail::body_kind::STATIC) {
                    return box;
                }

                const auto vx = body.velocity.x();
                const auto vy = body.velocity.y();

                auto min_x = box.min.x();
                auto min_y = box.min.y();
                auto max_x = box.max.x();
                auto max_y = box.max.y();

                if (vx < 0) {
                    min_x -= m_cfg.fatten_margin;
                } else if (vx > 0) {
                    max_x += m_cfg.fatten_margin;
                } else {
                    const auto epsilon = m_cfg.fatten_margin / 2.0f;
                    min_x -= epsilon;
                    max_x += epsilon;
                }

                if (vy < 0) {
                    min_y -= m_cfg.fatten_margin;
                } else if (vy > 0) {
                    max_y += m_cfg.fatten_margin;
                } else {
                    const auto epsilon = m_cfg.fatten_margin / 2.0f;
                    min_y -= epsilon;
                    max_y += epsilon;
                }

                return {{min_x, min_y}, {max_x, max_y}};
            }

            static bool should_collide(const filter_props& a, const filter_props& b) {
                return ((a.category & b.mask) && (b.category & a.mask));
            }

            // AABB enclosing a mover's shape over the whole sweep (start box ∪ end box). Used as
            // the broadphase envelope for cast() and as the active-region cull test.
            static aabb swept_bound(const moving_shape_t& shape, units::displacement delta) {
                const vec d = delta.value;
                return std::visit([&d](const auto& s) {
                    const auto start = enclose(s);
                    return aabb{
                        {
                            std::min(start.min.x() + d.x(), start.min.x()),
                            std::min(start.min.y() + d.y(), start.min.y())
                        },
                        {
                            std::max(start.max.x() + d.x(), start.max.x()),
                            std::max(start.max.y() + d.y(), start.max.y())
                        }
                    };
                }, shape);
            }

            // Cast acceptors run per candidate AFTER its swept contact is computed and receive the
            // CONTACT NORMAL, so a direction-dependent rule (ONE_WAY) decides on the face actually
            // crossed -- not on velocity guessed before the geometry is known.
            //
            // "Is this surface solid for the mover, given it was hit on `hit_normal`?"
            //   BLOCK   -> always solid.
            //   ONE_WAY -> solid only when the mover crossed the blocked face, i.e. the contact
            //              normal aligns with the surface's block_normal (dot > threshold). A
            //              jump-through floor (block_normal = up): landing on top -> normal up ->
            //              blocked; entering from below or the side -> normal != up -> passes.
            //   SENSOR / IGNORE -> never solid.
            // Functor (concrete type, not a lambda) so it is usable in run() above its definition.
            // Acceptors take the target's MATERIAL (not the whole resident_body) so they apply
            // uniformly to a resident or a grid tile -- both carry material_props.
            struct solid_pred {
                bool operator()(const material_props& m, const vec& hit_normal) const {
                    switch (m.response) {
                        case response_mode::BLOCK:
                            return true;
                        case response_mode::ONE_WAY:
                            return euler::dot(hit_normal, m.block_normal) > ONE_WAY_DOT;
                        case response_mode::SENSOR:
                        case response_mode::IGNORE:
                        default:
                            return false;
                    }
                }
            };

            static solid_pred solid_acceptor() { return solid_pred{}; }

            // Accept-all (used by unrestricted casts). Two-arg to match the post-hit acceptor shape.
            static bool accept_any(const material_props&, const vec&) { return true; }

            // Target material by handle -- dispatches BODY / BULLET / TILE. Lets move_and_slide's
            // velocity response read the surface material uniformly, tile or resident.
            [[nodiscard]] const material_props& material_of(const collider_id& id) const {
                if (id.type_id == collider_id::BODY) {
                    return m_bodies_storage[id.value].material;
                }
                if (id.type_id == collider_id::BULLET) {
                    return m_bullets_storage[id.value].material;
                }
                return m_static_grid->at(id.value)->material; // TILE
            }

            // The TILE handle for a grid candidate. The enumerators hand back the cell_box (geometry)
            // but hide the cell index (identity); recover it from the box centre, which maps back to
            // the same cell. value = linear cell index, generation = the cell's current generation
            // (so the handle stays valid until that cell is next mutated). Decodes via material_of/at.
            [[nodiscard]] collider_id tile_handle(const aabb& cell_box) const {
                const uint32_t cell = m_static_grid->to_cell(cell_box.center());
                return {cell, m_static_grid->cell_generation(cell), collider_id::TILE};
            }

            // Report every collider whose shape overlaps `self_shape` (passing `self_filter`),
            // across the BVH residents and -- when `query_tiles` -- the static grid. `exclude_body`
            // skips one body slot (the querying body itself; INVALID = none). Source-agnostic so it
            // serves both a body sensor (query_tiles = true, exclude self) and a tile sensor
            // (query_tiles = false -- tiles do not sense other tiles, and there is no body self).
            template<class Fn>
            void overlap_core(const shape_t& self_shape, const filter_props& self_filter,
                              uint32_t exclude_body, bool query_tiles, Fn&& on_hit) const {
                const aabb envelope = std::visit([](const auto& s) { return enclose(s); }, self_shape);

                auto consider = [&](const shape_t& target_shape, const filter_props& target_filter,
                                    const collider_id& id) {
                    if (!should_collide(self_filter, target_filter)) {
                        return;
                    }
                    const bool rc = std::visit([&](const auto& my_shape) {
                        return std::visit([&](const auto& other_shape) {
                            return collide::intersects(my_shape, other_shape);
                        }, target_shape);
                    }, self_shape);
                    if (rc) {
                        on_hit(id);
                    }
                };

                query(m_space_partition, envelope, [&](entity_id_t other_idx, [[maybe_unused]] const aabb& box) {
                    if (other_idx == exclude_body) {
                        return;
                    }
                    const auto& other = m_bodies_storage[other_idx];
                    consider(other.shape, other.filter,
                             collider_id{other_idx, other.generation, collider_id::BODY});
                });

                if (query_tiles && m_static_grid) {
                    m_static_grid->query(envelope, [&](const detail::tile& t, const aabb& cb) {
                        consider(t.shape, t.filter, tile_handle(cb));
                    });
                }
            }

            // A body's overlaps (the body senses bodies AND tiles).
            template<class Fn>
            void overlap(uint32_t idx, Fn&& on_hit) const {
                const auto& self = m_bodies_storage[idx];
                overlap_core(self.shape, self.filter, idx, /*query_tiles=*/true, std::forward<Fn>(on_hit));
            }

            // Swept-query core: sweep `mover` by `delta` and return the earliest accepted resident
            // hit (normalized toi in [0,1]), or nullopt. `mover` is a moving_shape_t (aabb|circle)
            // so the swept dispatch covers every target with no guard. `exclude_idx` skips one
            // body slot (the mover itself, when it lives in the tree; collider_id::INVALID = none).
            // `accept(const resident_body&, const vec& hit_normal)` runs INSIDE the candidate loop,
            // AFTER the swept contact is computed, so a direction rule (ONE_WAY) sees the real
            // contact normal; it must be per-candidate (cast keeps only the earliest hit, so a
            // post-filter could not recover a farther accepted target behind a rejected near one).
            // Residents are treated as stationary (per-mover CCD, §8c).
            template<class Accept>
            [[nodiscard]] std::optional <contact> cast_core(const moving_shape_t& mover,
                                                            units::displacement delta,
                                                            filter_props self_filter, Accept&& accept,
                                                            uint32_t exclude_idx,
                                                            uint32_t exclude_idx2 = collider_id::INVALID) const {
                const aabb envelope = swept_bound(mover, delta);
                const vec dv = delta.value;

                std::optional <contact> out;

                // Source-agnostic per-candidate sweep: filter, swept narrow-phase, post-hit accept,
                // keep the earliest. Identical for residents and grid tiles -- only the candidate's
                // (shape, filter, material, identity) differ. Reused by the grid fan-out.
                auto consider = [&](const shape_t& target_shape, const filter_props& target_filter,
                                    const material_props& target_material, const collider_id& id) {
                    if (!should_collide(self_filter, target_filter)) {
                        return;
                    }
                    std::visit([&](const auto& mv) {
                        std::visit([&](const auto& tgt) {
                            // unit-time normalization: entry_time is the [0,1] toi, and the
                            // time=1 window is the anti-tunneling bound. (delta fed as the
                            // mover's velocity over a unit window.)
                            const auto hit = swept_intersection(mv, dv, tgt, vec{0, 0}, 1.0f);
                            if (!hit) {
                                return;
                            }
                            if (!accept(target_material, hit->entry_normal)) {
                                return; // caller-rejected on the actual contact (e.g. one-way side)
                            }
                            if (!out || hit->entry_time < out->toi) {
                                out = contact{id, hit->entry_normal, hit->entry_time};
                            }
                        }, target_shape);
                    }, mover);
                };

                query(m_space_partition, envelope, [&](entity_id_t other_idx, [[maybe_unused]] const aabb& box) {
                    if (other_idx == exclude_idx || other_idx == exclude_idx2) {
                        return; // self, and (for a carried rider) the carrier it is locked to
                    }
                    const auto& other = m_bodies_storage[other_idx];
                    consider(other.shape, other.filter, other.material,
                             collider_id{other_idx, other.generation, collider_id::BODY});
                });

                // Static tiles overlapping the swept band (envelope == the swept bound). Each is
                // swept narrow-phased in consider(); the earliest TOI across bodies AND tiles wins,
                // so move_and_slide resolves against tile floors/walls/slopes for free.
                if (m_static_grid) {
                    m_static_grid->query(envelope, [&](const detail::tile& t, const aabb& cb) {
                        consider(t.shape, t.filter, t.material, tile_handle(cb));
                    });
                }
                return out;
            }

            // Internal: cast a *stored* mover (kinematic resident or bullet) by `delta`. A BODY
            // mover excludes itself; a BULLET is not in the tree, so nothing to exclude.
            template<class Accept>
            [[nodiscard]] std::optional <contact> cast(uint32_t idx, collider_id::type type_id,
                                                       units::displacement delta, Accept&& accept) const {
                if (type_id == collider_id::BODY) {
                    const auto& self = m_bodies_storage[idx];
                    return cast_core(detail::narrow(self.shape), delta, self.filter,
                                     std::forward<Accept>(accept), idx);
                }
                const auto& self = m_bullets_storage[idx];
                return cast_core(self.shape, delta, self.filter,
                                 std::forward<Accept>(accept), collider_id::INVALID);
            }

            // Convenience: cast against every filtered candidate (no acceptor restriction).
            [[nodiscard]] std::optional <contact> cast(uint32_t idx, collider_id::type type_id,
                                                       units::displacement delta) const {
                return cast(idx, type_id, delta, &world::accept_any);
            }

        public:
            // Game-facing aiming cast: sweep an arbitrary `mover` shape by `delta` and return the
            // earliest hit passing `filter`, or nullopt -- across BOTH dynamic residents and static
            // tiles (the result's collider_id may be BODY or TILE). The shape is transient (not in
            // the world), so nothing is excluded. Use for aim previews, lobbed-shot prediction,
            // "is this move clear" probes -- the swept counterpart to raycast. (Public API takes a
            // plain vec delta; it is wrapped as a displacement for the internal swept math.)
            [[nodiscard]] std::optional <contact> cast(const moving_shape_t& mover, vec delta,
                                                       filter_props filter = {}) const {
                return cast_core(mover, units::displacement{delta}, filter, &world::accept_any,
                                 collider_id::INVALID);
            }
            // First collider the finite segment `s` crosses (nearest along the ray), or nullopt --
            // across BOTH dynamic residents and static tiles (the result's collider_id may be BODY
            // or TILE). Bullets are excluded (not in the tree). Targets may be any shape incl.
            // segments (walls/slopes), visited as the full shape_t -- never narrowed. toi is clamped
            // to >= 0 (an origin already inside a shape reads as 0).
            [[nodiscard]] std::optional <contact> raycast(const segment& s, filter_props filter = {}) const {
                std::optional <contact> out;

                // Source-agnostic per-candidate ray test: filter, intersect the ray with the target
                // shape, keep the nearest. Reused by the grid fan-out.
                auto consider = [&](const shape_t& target_shape, const filter_props& target_filter,
                                    const collider_id& id) {
                    if (!should_collide(filter, target_filter)) {
                        return;
                    }
                    std::visit([&]<typename T1>(const T1& shape) {
                        using S = std::decay_t <T1>;
                        // intersect_param reports the parameter along its SEGMENT argument: ray is the
                        // 2nd arg for aabb/circle targets, but the 1st arg for a segment target (params
                        // are along `a`). Put the ray where the parameter lands so toi is "fraction
                        // along ray".
                        const std::optional <line_hit> hit = [&] {
                            if constexpr (std::is_same_v <S, segment>) {
                                return intersect_param(s, shape); // ray first
                            } else {
                                return intersect_param(shape, s); // aabb/circle first
                            }
                        }();
                        // segment_overlaps() restricts the infinite-line crossing to the finite ray;
                        // entry_param is otherwise unclamped.
                        if (hit && hit->segment_overlaps()) {
                            const float toi = std::max(0.0f, hit->entry_param); // origin-inside -> 0
                            if (!out || toi < out->toi) {
                                out = contact{id, hit->entry_normal, toi};
                            }
                        }
                    }, target_shape);
                };

                // Tree raycast: visits only the boxes the ray crosses and clips farther boxes to
                // the nearest confirmed hit. The float-returning callback feeds back the best toi
                // so far as the new ray-clip fraction (the tree does t_max = min(t_max, ret)).
                collide::raycast(m_space_partition, s, // qualified: the member `raycast` would otherwise hide it
                                 [&](entity_id_t other_idx, [[maybe_unused]] const aabb& box,
                                     [[maybe_unused]] const line_hit& box_hit) -> float {
                                     const auto& other = m_bodies_storage[other_idx];
                                     consider(other.shape, other.filter,
                                              collider_id{other_idx, other.generation, collider_id::BODY});
                                     // Clip the ray to the nearest confirmed hit so farther boxes prune;
                                     // 1.0 (no clip) until we have one.
                                     return out ? out->toi : 1.0f;
                                 });

                // Static tiles along the ray. The grid DDA is near-to-far and reports the cell-entry
                // parameter t_entry; the precise narrow-phase toi is >= t_entry, so once t_entry
                // exceeds the best hit so far, no farther tile can beat it -> stop (bool early-out).
                if (m_static_grid) {
                    m_static_grid->raycast(s.from, s.to,
                                           [&](const detail::tile& t, const aabb& cb, float t_entry) -> bool {
                                               if (out && t_entry > out->toi) {
                                                   return false; // farther than the best hit -> done
                                               }
                                               consider(t.shape, t.filter, tile_handle(cb));
                                               return true;
                                           });
                }
                return out;
            }

            // Is `to` visible from `from` -- i.e. nothing in `blockers` lies strictly between?
            // A convenience over raycast: clear when there is no hit, or the first blocker is at
            // or beyond the target endpoint.
            [[nodiscard]] bool line_of_sight(vec from, vec to, filter_props blockers = {}) const {
                const auto hit = raycast(segment{from, to}, blockers);
                return !hit || hit->toi >= 1.0f;
            }

        private:
            struct slide_result {
                vec velocity; // post-slide velocity (also written back to the body)
                bool grounded = false;
                std::array <contact, 4> contacts{}; // surfaces hit this move (for run() -> events)
                int count = 0;
            };

            // Move kinematic body `idx` by velocity*dt this frame, resolving against the residents
            // the `acceptor` deems solid (BLOCK, or ONE_WAY from the blocked side). It sweeps,
            // stops a `skin` short of each contact, slides the leftover along the surface, and
            // damps the velocity via the surface material -- up to `max_slide_iter` passes (a
            // floor+wall corner needs 2). The body's stored shape/velocity and its broadphase
            // proxy are updated in place; the returned slide_result carries the post-slide
            // velocity, grounded flag, and the contacts hit (for run() to emit as events).
            //
            // `acceptor(const resident_body&, const vec& hit_normal) -> bool` selects solid
            // surfaces, given each candidate's actual contact normal (so ONE_WAY decides on the
            // face crossed). Sensors/ignored bodies return false so they never block movement
            // (run() reports them via its separate overlap/trigger passes).
            template<typename Fn>
            slide_result move_and_slide(uint32_t idx, units::duration dt, Fn&& acceptor) {
                auto& self = m_bodies_storage[idx];
                ENFORCE(self.kind == detail::body_kind::KINEMATIC);

                slide_result res;
                res.velocity = self.velocity;
                units::displacement remaining = units::velocity{self.velocity} * dt; // v * dt
                for (int iter = 0; iter < m_cfg.max_slide_iter; ++iter) {
                    if (near_zero(remaining.value)) {
                        break;
                    }
                    auto hit = cast(idx, collider_id::BODY, remaining, acceptor);
                    if (!hit) {
                        translate(self, remaining.value); // clear path: take the whole step
                        break;
                    }
                    // Advance to just short of the surface (the skin keeps a permanent gap so the
                    // next iteration's cast does not re-hit at toi 0).
                    const float len = euler::length(remaining.value);
                    const float skin_frac = (len > constants::POINT_EPS) ? (m_cfg.skin / len) : 0.0f;
                    const float advance = std::max(0.0f, hit->toi - skin_frac);
                    translate(self, (remaining * units::fraction{advance}).value);

                    const vec n = hit->normal;
                    if (near_zero(n)) {
                        break; // undefined normal -> can't slide; bail
                    }

                    // Slide: the leftover budget is (1 - toi) of the step (NOT 1 - advance --
                    // the skin is a physical cushion, not part of the motion budget); remove its
                    // into-surface component so the rest glides along the surface.
                    const units::displacement leftover = remaining * units::fraction{1.0f - hit->toi};
                    remaining = units::displacement{leftover.value - euler::dot(leftover.value, n) * n};

                    // Velocity response -- material-driven (friction/restitution from the SURFACE),
                    // applied to velocity only; position sliding above is pure geometry. material_of
                    // dispatches on the contact's handle, so a tile surface works like a resident.
                    res.velocity = detail::eval_velocity_response(units::velocity{res.velocity}, n,
                                                                  material_of(hit->who)).value;

                    // Grounded if the contact faces up enough to stand on (~45 deg max slope).
                    if (euler::dot(n, m_cfg.up) > GROUND_THRESHOLD) {
                        res.grounded = true;
                    }
                    if (res.count < static_cast <int>(res.contacts.size())) {
                        res.contacts[res.count++] = *hit;
                    }
                }

                // Persist: write back the projected velocity, and re-fit the proxy ONLY when the
                // moved tight box has escaped the stored fat box. Passing a freshly fattened box
                // every frame would defeat update_leaf's containment short-circuit (the margin
                // shifts with the body, so it never stays contained) and re-graft every frame.
                self.velocity = res.velocity;
                const aabb tight = std::visit([](const auto& s) { return enclose(s); }, self.shape);
                if (!detail::contains(m_space_partition[self.proxy].box, tight)) {
                    update_leaf(m_space_partition, self.proxy, fatten(self)); // escaped -> fresh fat box
                }
                return res;
            }

            // A persistent sensor-overlap pair. `key` is the diff identity: the two handles in a
            // canonical order, INCLUDING generation -- so a slot reused by a different body (new
            // generation) is a distinct pair and the old end / new begin both fire correctly,
            // rather than the diff silently treating them as "still overlapping". The collider_ids
            // are kept so an END can name the pair even after a body is removed.
            struct sensor_pair {
                // {lo.value, lo.gen, lo.type, hi.value, hi.gen, hi.type}; std::array < is lexicographic.
                // type_id is part of the key: a BODY and a TILE can share value+generation (a cell
                // index can equal a body slot), so omitting it would alias their pairs.
                std::array<uint32_t, 6> key{};
                collider_id sensor{};
                collider_id other{};
            };

            // Canonical, full-identity pair key (order the two handles by the whole collider_id:
            // value, then generation, then type_id -- via the defaulted operator<=>).
            static std::array<uint32_t, 6> pair_key(const collider_id& a, const collider_id& b) {
                const collider_id& lo = (a < b) ? a : b;
                const collider_id& hi = (a < b) ? b : a;
                return {lo.value, lo.generation, static_cast<uint32_t>(lo.type_id),
                        hi.value, hi.generation, static_cast<uint32_t>(hi.type_id)};
            }

            void emit_trigger(event_kind kind, const sensor_pair& p) {
                m_events.emplace_back(kind, p.sensor, p.other, vec{}, -1.0f);
            }

            // A contact counts as "ground" when its normal points up enough to stand on:
            // dot(n, up) > cos(45 deg) ~ 0.707 (max walkable slope ~45 degrees).
            static constexpr float GROUND_THRESHOLD = 0.707f;

            // A ONE_WAY surface blocks only when the contact normal aligns with its block_normal
            // (dot above this). 0.5 (~cos 60 deg) blocks a head-on crossing of the blocked face
            // while letting side/grazing contacts pass.
            static constexpr float ONE_WAY_DOT = 0.5f;

            static constexpr bool near_zero(const vec& v) {
                return euler::length_squared(v) < constants::POINT_EPS * constants::POINT_EPS;
            }

            static void translate(detail::resident_body& body, const vec& v) {
                shape_t new_shape{aabb{}};
                std::visit([&v, &new_shape](const auto& s) {
                    new_shape = collide::translate(s, v);
                }, body.shape);
                body.shape = new_shape;
            }

            static void translate(detail::nonresident_body& body, const vec& v) {
                moving_shape_t new_shape{aabb{}};
                std::visit([&v, &new_shape](const auto& s) {
                    new_shape = collide::translate(s, v);
                }, body.shape);
                body.shape = new_shape;
            }

            // ---- carriers (moving platforms / conveyors / crushers) -----------------------------

            // True if actor `actor_idx` rides carrier `carrier_idx`: a short downward (opposite `up`)
            // swept probe with the ACTOR's filter + the solid acceptor finds, as its NEAREST solid
            // contact, the carrier itself with an up-facing normal. This (unlike a bare overlap test)
            // respects contact DIRECTION (a side touch is not "riding"), FILTERS (a rider whose layer
            // rejects the carrier is not carried), and MATERIAL (a SENSOR/IGNORE carrier is not solid,
            // so it carries nothing). The probe length bridges the move_and_slide skin gap.
            [[nodiscard]] bool is_riding(uint32_t actor_idx, uint32_t carrier_idx) const {
                const auto& a = m_bodies_storage[actor_idx];
                const auto& c = m_bodies_storage[carrier_idx];
                if (!should_collide(a.filter, c.filter)) {
                    return false; // layers don't interact -> not carried
                }
                if (!solid_pred{}(c.material, m_cfg.up)) {
                    return false; // carrier isn't solid from above (SENSOR/IGNORE, or one-way wrong side)
                }

                // Geometric "on top" over the enclosing boxes, general over `up`: project both boxes
                // onto the up axis and its perpendicular. The actor rides iff its UNDERSIDE (lowest
                // along up) rests at/just above the carrier's TOP (within the skin gap) AND they
                // STRICTLY overlap perpendicular (genuinely over the top -- not edge-touching a side).
                // Uses boxes (not a swept cast) so an exact top-contact (gap 0) rides cleanly and a
                // boundary touch never produces a degenerate contact normal.
                struct span { float lo, hi; };
                const auto proj = [](const aabb& b, const vec& ax) -> span {
                    const float c0 = b.min.x() * ax.x() + b.min.y() * ax.y();
                    const float c1 = b.max.x() * ax.x() + b.min.y() * ax.y();
                    const float c2 = b.min.x() * ax.x() + b.max.y() * ax.y();
                    const float c3 = b.max.x() * ax.x() + b.max.y() * ax.y();
                    return span{std::min({c0, c1, c2, c3}), std::max({c0, c1, c2, c3})};
                };
                const aabb ra = std::visit([](const auto& s) { return enclose(s); }, a.shape);
                const aabb rc = std::visit([](const auto& s) { return enclose(s); }, c.shape);
                const vec u = m_cfg.up;
                const vec perp{u.y(), -u.x()};
                const span au = proj(ra, u), cu = proj(rc, u);
                const span ap = proj(ra, perp), cp = proj(rc, perp);
                const float eps = constants::POINT_EPS;
                const float gap = m_cfg.skin * 2.0f + eps; // touching .. within the move_and_slide skin gap
                const bool on_top = (au.lo >= cu.hi - eps) && (au.lo <= cu.hi + gap);
                const bool perp_overlap = (ap.hi > cp.lo + eps) && (ap.lo < cp.hi - eps);
                return on_top && perp_overlap;
            }

            // Re-fit a resident's broadphase proxy after it moved, only when its tight box escaped the
            // stored fat box (same containment short-circuit as move_and_slide).
            void refit_proxy(detail::resident_body& b) {
                const aabb tight = std::visit([](const auto& s) { return enclose(s); }, b.shape);
                if (!detail::contains(m_space_partition[b.proxy].box, tight)) {
                    update_leaf(m_space_partition, b.proxy, fatten(b));
                }
            }

            // Move a carrier rigidly by `d` (it is never blocked -- it pushes/carries, §MP2/3).
            void move_carrier_rigid(uint32_t idx, const vec& d) {
                auto& c = m_bodies_storage[idx];
                translate(c, d);
                refit_proxy(c);
            }

            // Carry rider `j` (locked to carrier `carrier_idx`) by `d`: a collision-aware move that
            // hits every solid EXCEPT its carrier, so it rides freely but still stops at walls /
            // ceilings / other carriers. Stops a skin short of the first hit. (MP1: a blocked rider
            // simply stops; MP3 will turn "still pinned" into a CRUSH event.)
            void carry_translate(uint32_t j, uint32_t carrier_idx, const vec& d) {
                if (near_zero(d)) {
                    return;
                }
                auto& a = m_bodies_storage[j];
                const auto hit = cast_core(detail::narrow(a.shape), units::displacement{d}, a.filter,
                                           solid_acceptor(), j, carrier_idx);
                float frac = 1.0f;
                if (hit) {
                    const float len = euler::length(d);
                    const float skin_frac = (len > constants::POINT_EPS) ? (m_cfg.skin / len) : 0.0f;
                    frac = std::max(0.0f, hit->toi - skin_frac);
                }
                if (frac <= 0.0f) {
                    return;
                }
                translate(a, vec{d.x() * frac, d.y() * frac});
                refit_proxy(a);
            }

        private:
            world_config m_cfg;
            detail::bodies_storage m_bodies_storage;
            detail::bullets_storage m_bullets_storage;

            tree m_space_partition;
            std::optional <grid <detail::tile>> m_static_grid; // statics (tiles); unset = none

            std::vector <world_event> m_events;          // reused per-frame event buffer
            std::vector <uint32_t> m_rider_scratch;      // reused per-carrier rider list (carrier pass)
            std::vector <sensor_pair> m_triggers_curr;   // this frame's sensor overlaps
            std::vector <sensor_pair> m_triggers_prev;   // last frame's (for the begin/end diff)
            std::vector <collider_id> m_sensor_tiles;    // SENSOR tile handles (lazily pruned)
    };
}

// Lets collider_id be a key in std::unordered_map / unordered_set. Mixes all three identity
// fields so handles to different slots/generations/types hash apart.
template <>
struct std::hash<simplex::collide::collider_id> {
    [[nodiscard]] std::size_t operator()(const simplex::collide::collider_id& id) const noexcept {
        const std::size_t h1 = std::hash<std::uint32_t>{}(id.value);
        const std::size_t h2 = std::hash<std::uint32_t>{}(id.generation);
        const std::size_t h3 = std::hash<std::uint32_t>{}(static_cast<std::uint32_t>(id.type_id));
        std::size_t h = h1;
        h ^= h2 + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        h ^= h3 + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        return h;
    }
};
