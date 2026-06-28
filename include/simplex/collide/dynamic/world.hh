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
#include <simplex/collide/dynamic/world_types.hh>

namespace simplex::collide {

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
                const aabb bound = detail::tight_box(body.shape);
                const vec centre = bound.center();
                const uint32_t cell = m_static_grid->to_cell(centre);
                ENFORCE(cell != grid<detail::tile>::INVALID_CELL)("tile centre is outside the grid bounds");
                // A tile is only stored in (and only found via) its centre's cell, so it must fit
                // within that cell -- otherwise queries through the overhang would silently miss it.
                ENFORCE(detail::contains(m_static_grid->cell_box_at(cell), bound))
                        ("tile shape must fit within a single grid cell");
                // Mergeable (static) geometry is baked once on the first run(); it cannot be added
                // afterwards (the bake is destructive and not re-run). Non-mergeable tiles are free.
                ENFORCE(!(body.mergeable && m_compiled))
                        ("mergeable tiles must be added before the first run() (static geometry is baked once)");
                m_static_grid->set(centre,
                                   detail::tile{body.shape, body.material, body.filter, eid, body.mergeable});
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
                m_compiled = false; // a fresh level may bake again
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
                    const aabb nb = detail::tight_box(shape);
                    ENFORCE(detail::contains(m_static_grid->cell_box_at(cid.value), nb))
                            ("set_shape: a tile's new shape must fit within its cell");
                    // A mergeable tile is frozen after the one-shot bake -- otherwise a mergeable
                    // tile that survived the bake (e.g. a slope, not an aabb) could be reshaped into a
                    // cell-filling BLOCK aabb and smuggle un-baked static geometry past add()'s guard.
                    ENFORCE(!(m_compiled && m_static_grid->at(cid.value)->mergeable))
                            ("a mergeable tile cannot be reshaped after the first run() (static geometry is baked once)");
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

                // One-shot boundary-bake of opted-in tiles into merged residents, on the first run()
                // (before anything queries), so the seamless geometry is in place.
                compile_static_grid();

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
                        emit_crush_if_pinned(r, carrier_idx, rider_delta); // MP3: carried into a wall
                    }

                    // MP2 -- pushing: a carrier that MOVED (body_delta != 0) shoves any non-rider
                    // actor its sweep RAN INTO clear along its motion. (A conveyor with body_delta 0
                    // does not push -- it only drags riders.) The candidate gate is a swept test of
                    // the carrier (from its start, by body_delta) vs the actor: this is anti-tunnel
                    // (a fast carrier still catches a thin actor anywhere in the band) AND directional
                    // (a carrier moving AWAY from a trailing actor never hits it). Each hit is shoved
                    // to the carrier's FINAL leading edge via the collision-aware carry_translate
                    // (blocked at a wall it stops, leaving a residual overlap MP3 reads as a crush).
                    // Collect-then-push -- pushing mutates the tree and must not run during the query.
                    if (!near_zero(body_delta)) {
                        const vec back{-body_delta.x(), -body_delta.y()};
                        const aabb fbox = detail::tight_box(m_bodies_storage[carrier_idx].shape);
                        const aabb band = aabb::combine(fbox, collide::translate(fbox, back)); // start .. final sweep
                        const moving_shape_t cstart = detail::narrow(std::visit([&](const auto& s) {
                            return shape_t{collide::translate(s, back)};
                        }, m_bodies_storage[carrier_idx].shape));

                        m_push_scratch.clear();
                        query(m_space_partition, band, [&](entity_id_t actor_idx, const aabb&) {
                            if (actor_idx == carrier_idx) {
                                return;
                            }
                            const auto& act = m_bodies_storage[actor_idx];
                            if (act.kind != detail::body_kind::KINEMATIC
                                || !should_collide(act.filter, m_bodies_storage[carrier_idx].filter)) {
                                return; // only actors that interact with the carrier are pushed
                            }
                            for (const uint32_t r : m_rider_scratch) {
                                if (r == actor_idx) {
                                    return; // already handled as a rider (carried, not pushed)
                                }
                            }
                            // Directional gate: ignore an actor entirely BEHIND the carrier's start
                            // along the motion (the carrier moves away from it). This drops a body
                            // merely touching the carrier's trailing edge, which the swept test below
                            // would otherwise report as a toi-0 initial-overlap hit regardless of direction.
                            const aabb sbox = collide::translate(fbox, back);
                            const aabb abox0 = detail::tight_box(act.shape);
                            const float e = constants::POINT_EPS;
                            if ((body_delta.x() > e && abox0.max.x() <= sbox.min.x() + e)
                                || (body_delta.x() < -e && abox0.min.x() >= sbox.max.x() - e)
                                || (body_delta.y() > e && abox0.max.y() <= sbox.min.y() + e)
                                || (body_delta.y() < -e && abox0.min.y() >= sbox.max.y() - e)) {
                                return;
                            }
                            const auto swept = swept_vs_shape(cstart, body_delta, act.shape);
                            if (!swept) {
                                return; // the carrier's sweep doesn't reach the actor
                            }
                            // The carrier pushes only if it is SOLID for this contact -- same rule as
                            // riding (a SENSOR/IGNORE carrier pushes nothing; a one-way carrier only
                            // from its blocked face). Use the ACTUAL swept contact normal, oriented to
                            // point in the carrier's motion hemisphere (its push side): this normalizes
                            // the per-shape-pair convention difference (swept's outward side is the aabb
                            // for aabb-vs-circle but the target for aabb-vs-aabb) while keying one-way on
                            // the real contact face -- a motion-direction guess mis-evaluates diagonal
                            // carriers (a face hit can differ from the velocity direction).
                            vec n = swept->entry_normal;
                            if (euler::dot(n, body_delta) < 0.0f) {
                                n = vec{-n.x(), -n.y()};
                            }
                            if (solid_pred{}(m_bodies_storage[carrier_idx].material, n)) {
                                m_push_scratch.push_back(actor_idx);
                            }
                        });
                        for (const uint32_t aidx : m_push_scratch) {
                            const aabb abox = detail::tight_box(m_bodies_storage[aidx].shape);
                            const vec push = clear_push(abox, fbox, body_delta);
                            carry_translate(aidx, carrier_idx, push);
                            emit_crush_if_pinned(aidx, carrier_idx, push); // MP3: pushed into a wall
                        }
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

            // ---- query plumbing: narrow-phase wrappers + candidate fan-out -----------------------
            // These collapse the six near-identical "fan out over residents + tiles, narrow-phase each
            // candidate, accumulate" copies into one primitive per traversal kind. Each public query
            // is then a thin visitor: the fan-out owns exclusion / tile_handle / ray-clipping (one
            // place to get right), the visitor owns filtering + narrow-phase + accumulation.

            // Swept narrow-phase: the mover (aabb|circle) swept by `dv` against any target shape.
            // Hides the nested std::visit double-dispatch. entry_time is the [0,1] toi; time=1 is the
            // anti-tunnel window (delta is the mover's velocity over a unit step).
            [[nodiscard]] static std::optional <swept_hit> swept_vs_shape(const moving_shape_t& mover,
                                                                         const vec& dv, const shape_t& target) {
                return std::visit([&](const auto& mv) {
                    return std::visit([&](const auto& tgt) -> std::optional <swept_hit> {
                        return swept_intersection(mv, dv, tgt, vec{0, 0}, 1.0f);
                    }, target);
                }, mover);
            }

            // Ray narrow-phase: the finite segment `s` against any target shape, restricted to the
            // segment span. Hides the segment-first/aabb-first intersect_param dispatch (params run
            // along the ray either way). Returns the crossing's line_hit, or nullopt if it does not
            // cross within the segment; the caller clamps entry_param >= 0 (origin-inside -> 0).
            [[nodiscard]] static std::optional <line_hit> ray_vs_shape(const segment& s, const shape_t& target) {
                return std::visit([&]<typename T>(const T& shape) -> std::optional <line_hit> {
                    const std::optional <line_hit> hit = [&] {
                        if constexpr (std::is_same_v <std::decay_t <T>, segment>) {
                            return intersect_param(s, shape); // ray first
                        } else {
                            return intersect_param(shape, s); // aabb/circle first
                        }
                    }();
                    if (hit && hit->segment_overlaps()) {
                        return hit;
                    }
                    return std::nullopt;
                }, target);
            }

            // Visit every candidate (resident +, when `query_tiles`, tile) overlapping `env`, skipping
            // the two excluded body slots (INVALID = none). `visit(shape, filter, material, id)`. The
            // broadphase query visits ALL candidates (no clipping), so the visitor/accumulator decides
            // what to keep -- nearest-hit, all-hits, and sensor variants all share this fan-out.
            template<class Visit>
            void for_each_in_envelope(const aabb& env, uint32_t exclude_idx, uint32_t exclude_idx2,
                                      bool query_tiles, Visit&& visit) const {
                query(m_space_partition, env, [&](entity_id_t other_idx, [[maybe_unused]] const aabb& box) {
                    if (other_idx == exclude_idx || other_idx == exclude_idx2) {
                        return;
                    }
                    const auto& other = m_bodies_storage[other_idx];
                    visit(other.shape, other.filter, other.material,
                          collider_id{other_idx, other.generation, collider_id::BODY});
                });
                if (query_tiles && m_static_grid) {
                    m_static_grid->query(env, [&](const detail::tile& t, const aabb& cb) {
                        visit(t.shape, t.filter, t.material, tile_handle(cb));
                    });
                }
            }

            // Visit every candidate the segment `s` crosses (resident + tile). `visit(shape, filter,
            // id)` narrow-phases and accumulates; `clip()` returns the current ray-clip toi -- the
            // nearest-hit best so far, or a constant 1.0 for an all-hits query that never prunes. The
            // tree clips farther boxes to clip(); the grid DDA stops once a cell's entry exceeds it.
            template<class Clip, class Visit>
            void for_each_along_ray(const segment& s, Clip&& clip, Visit&& visit) const {
                collide::raycast(m_space_partition, s, // qualified: member `raycast` would otherwise hide it
                                 [&](entity_id_t other_idx, [[maybe_unused]] const aabb& box,
                                     [[maybe_unused]] const line_hit& box_hit) -> float {
                                     const auto& other = m_bodies_storage[other_idx];
                                     visit(other.shape, other.filter,
                                           collider_id{other_idx, other.generation, collider_id::BODY});
                                     return clip();
                                 });
                if (m_static_grid) {
                    m_static_grid->raycast(s.from, s.to,
                                           [&](const detail::tile& t, const aabb& cb, float t_entry) -> bool {
                                               if (t_entry > clip()) {
                                                   return false; // beyond the current best -> stop
                                               }
                                               visit(t.shape, t.filter, tile_handle(cb));
                                               return true;
                                           });
                }
            }

            // Report every collider whose shape overlaps `self_shape` (passing `self_filter`),
            // across the BVH residents and -- when `query_tiles` -- the static grid. `exclude_body`
            // skips one body slot (the querying body itself; INVALID = none). Source-agnostic so it
            // serves both a body sensor (query_tiles = true, exclude self) and a tile sensor
            // (query_tiles = false -- tiles do not sense other tiles, and there is no body self).
            template<class Fn>
            void overlap_core(const shape_t& self_shape, const filter_props& self_filter,
                              uint32_t exclude_body, bool query_tiles, Fn&& on_hit) const {
                for_each_in_envelope(
                    detail::tight_box(self_shape), exclude_body, collider_id::INVALID, query_tiles,
                    [&](const shape_t& target_shape, const filter_props& target_filter,
                        const material_props&, const collider_id& id) {
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
                    });
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
                const vec dv = delta.value;
                std::optional <contact> out;
                // Keep the earliest accepted hit across residents AND tiles (so move_and_slide resolves
                // tile floors/walls/slopes for free). accept() sees the real contact normal, so it can
                // reject per-candidate on the actual face (one-way side) -- and the fan-out visits all,
                // so a farther accepted target behind a rejected near one is still found.
                for_each_in_envelope(
                    swept_bound(mover, delta), exclude_idx, exclude_idx2, /*query_tiles=*/true,
                    [&](const shape_t& target_shape, const filter_props& target_filter,
                        const material_props& target_material, const collider_id& id) {
                        if (!should_collide(self_filter, target_filter)) {
                            return;
                        }
                        const auto hit = swept_vs_shape(mover, dv, target_shape);
                        if (!hit || !accept(target_material, hit->entry_normal)) {
                            return;
                        }
                        if (!out || hit->entry_time < out->toi) {
                            out = contact{id, hit->entry_normal, hit->entry_time};
                        }
                    });
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

            // Multi-hit swept shape cast (cross-cutting roadmap): EVERY collider that `mover` swept by
            // `delta` would touch, ordered nearest-first by toi -- the swept counterpart to
            // raycast_all, where `cast` returns only the nearest. `max_hits` > 0 keeps just the nearest
            // N (0 = all). Across residents AND tiles; the transient shape excludes nothing. Reports
            // ALL filtered colliders regardless of material response (a beam passes through, so the
            // caller decides what stops it via `filter`); a body with several colliders reports once
            // PER collider -- dedup by eid is the caller's job (see the entity-grouping roadmap item).
            [[nodiscard]] std::vector <contact> cast_all(const moving_shape_t& mover, vec delta,
                                                         filter_props filter = {},
                                                         std::size_t max_hits = 0) const {
                const units::displacement d{delta};
                const vec dv = d.value;
                std::vector <contact> hits;
                for_each_in_envelope(
                    swept_bound(mover, d), collider_id::INVALID, collider_id::INVALID, /*query_tiles=*/true,
                    [&](const shape_t& ts, const filter_props& tf,
                        const material_props&, const collider_id& id) {
                        if (!should_collide(filter, tf)) {
                            return;
                        }
                        if (const auto hit = swept_vs_shape(mover, dv, ts)) {
                            hits.push_back(contact{id, hit->entry_normal, hit->entry_time});
                        }
                    });
                return nearest_first(std::move(hits), max_hits);
            }

            // Swept / crossing triggers (cross-cutting roadmap): the SENSOR colliders the shape
            // `mover` would touch while sweeping by `delta` -- INCLUDING ones it passes entirely
            // through between the endpoints. run()'s trigger pass diffs sensor overlaps at frame
            // BOUNDARIES, so a fast body can skip a thin pickup / checkpoint / tripwire between two
            // sampled frames without ever overlapping it at an endpoint; this swept query catches
            // those crossings. Reports SENSORS only (solids are the movement pass's job), ordered
            // nearest-first by `toi` (a sensor already overlapped at the start reads as toi 0).
            //
            // The shape is transient (not in the world): pass the mover's PRE-move shape and its frame
            // delta. Complements the begin/end diff -- use TRIGGER_BEGIN/END for staying inside a zone,
            // this for a momentary fast crossing. A body with several sensor colliders reports once PER
            // collider (eid-dedup is the caller's job, as with cast_all).
            [[nodiscard]] std::vector <contact> swept_triggers(const moving_shape_t& mover, vec delta,
                                                               filter_props filter = {}) const {
                const units::displacement dd{delta};
                const vec dv = dd.value;
                std::vector <contact> hits;
                for_each_in_envelope(
                    swept_bound(mover, dd), collider_id::INVALID, collider_id::INVALID, /*query_tiles=*/true,
                    [&](const shape_t& ts, const filter_props& tf,
                        const material_props& tm, const collider_id& id) {
                        if (tm.response != response_mode::SENSOR) {
                            return; // triggers are sensors; solids are resolved by the movement pass
                        }
                        if (!should_collide(filter, tf)) {
                            return;
                        }
                        if (const auto hit = swept_vs_shape(mover, dv, ts)) {
                            hits.push_back(contact{id, hit->entry_normal, hit->entry_time});
                        }
                    });
                return nearest_first(std::move(hits));
            }

            // Entity-level collider grouping (cross-cutting roadmap): collapse multi-hit results to
            // one contact PER ENTITY (eid), keeping the NEAREST (smallest toi) collider of each. One
            // entity often owns several colliders by role -- hurtbox, hitbox, weak points, shield
            // zones -- all sharing its eid; without this a beam reports the same enemy once per
            // collider. Operates on the output of raycast_all / cast_all / swept_triggers; the result
            // stays ordered nearest-first. Keys purely on eid, so colliders sharing an eid (INCLUDING
            // the default 0 on un-keyed colliders) merge -- assign eids per logical entity. Handles
            // must still be live (call on fresh query results, same frame).
            [[nodiscard]] std::vector <contact> dedup_by_entity(std::vector <contact> hits) const {
                std::vector <contact> out;
                out.reserve(hits.size());
                std::vector <entity_id_t> kept; // eid parallel to `out`, so the nearest per entity wins
                for (const auto& h : hits) {
                    const entity_id_t e = get_eid(h.who);
                    bool merged = false;
                    for (std::size_t k = 0; k < kept.size(); ++k) {
                        if (kept[k] == e) {
                            merged = true;
                            if (h.toi < out[k].toi) {
                                out[k] = h; // a nearer collider of the same entity supersedes
                            }
                            break;
                        }
                    }
                    if (!merged) {
                        out.push_back(h);
                        kept.push_back(e);
                    }
                }
                return nearest_first(std::move(out));
            }

            // First collider the finite segment `s` crosses (nearest along the ray), or nullopt --
            // across BOTH dynamic residents and static tiles (the result's collider_id may be BODY
            // or TILE). Bullets are excluded (not in the tree). Targets may be any shape incl.
            // segments (walls/slopes), visited as the full shape_t -- never narrowed. toi is clamped
            // to >= 0 (an origin already inside a shape reads as 0).
            [[nodiscard]] std::optional <contact> raycast(const segment& s, filter_props filter = {}) const {
                std::optional <contact> out;
                // Keep the nearest crossing; clip the ray to the best hit so far so the tree prunes
                // farther boxes and the grid DDA stops once a cell's entry exceeds it.
                for_each_along_ray(
                    s, [&] { return out ? out->toi : 1.0f; },
                    [&](const shape_t& ts, const filter_props& tf, const collider_id& id) {
                        if (!should_collide(filter, tf)) {
                            return;
                        }
                        if (const auto hit = ray_vs_shape(s, ts)) {
                            const float toi = std::max(0.0f, hit->entry_param); // origin-inside -> 0
                            if (!out || toi < out->toi) {
                                out = contact{id, hit->entry_normal, toi};
                            }
                        }
                    });
                return out;
            }

            // Multi-hit ray (cross-cutting roadmap): EVERY collider the finite segment `s` crosses,
            // ordered nearest-first by toi -- not just the nearest like raycast. `max_hits` > 0 keeps
            // just the nearest N (0 = all). Across residents AND tiles; bullets excluded (not in the
            // tree). Covers beams, piercing shots, melee/sword arcs, explosion sweeps, boss
            // multi-hurtbox scans. A body with several colliders reports once PER collider -- dedup by
            // eid is the caller's job (see the entity-grouping roadmap item).
            [[nodiscard]] std::vector <contact> raycast_all(const segment& s, filter_props filter = {},
                                                            std::size_t max_hits = 0) const {
                std::vector <contact> hits;
                // Constant clip 1.0 -> never prune: collect every crossing (toi in [0,1]).
                for_each_along_ray(
                    s, [] { return 1.0f; },
                    [&](const shape_t& ts, const filter_props& tf, const collider_id& id) {
                        if (!should_collide(filter, tf)) {
                            return;
                        }
                        if (const auto hit = ray_vs_shape(s, ts)) {
                            hits.push_back(contact{id, hit->entry_normal, std::max(0.0f, hit->entry_param)});
                        }
                    });
                return nearest_first(std::move(hits), max_hits);
            }

            // Is `to` visible from `from` -- i.e. nothing in `blockers` lies strictly between?
            // A convenience over raycast: clear when there is no hit, or the first blocker is at
            // or beyond the target endpoint.
            [[nodiscard]] bool line_of_sight(vec from, vec to, filter_props blockers = {}) const {
                const auto hit = raycast(segment{from, to}, blockers);
                return !hit || hit->toi >= 1.0f;
            }

            // §19 #2 -- ground snapping. Keep a grounded actor glued to a floor that receded beneath
            // it this frame, so it hugs a downhill slope / staircase instead of launching off each
            // lip. Sweeps the actor's OWN shape straight down (-up) by at most `max_drop`; if it
            // lands on a WALKABLE surface within reach (the same solids it slides on -- ONE_WAY
            // floors included, from above), it translates the actor down onto that surface, leaving
            // the standard skin gap so the next frame's cast does not re-hit at toi 0.
            //
            // Returns the ground contact it snapped to (normal usable for slope-aligned velocity),
            // or nullopt when there is no walkable ground within `max_drop` -- the actor is over a
            // real ledge/cliff (or a >45 deg face) and should fall. A flush floor (toi 0) returns the
            // contact with no move.
            //
            // POLICY is the caller's: call AFTER run(), only when the actor was grounded last frame
            // and is not rising. Size `max_drop` to the tallest one-frame step down (a few px +
            // slope*speed*dt), small enough that a true cliff (drop > max_drop) still lets it fall.
            [[nodiscard]] std::optional <contact> snap_to_ground(collider_id cid, float max_drop) {
                ENFORCE(is_valid(cid) && cid.type_id == collider_id::BODY);
                auto& self = m_bodies_storage[cid.value];
                ENFORCE(self.kind == detail::body_kind::KINEMATIC)("snap_to_ground: actor must be kinematic");
                ENFORCE(max_drop >= 0.0f)("snap_to_ground: max_drop must be non-negative");

                // Sweep the actor's shape straight down; cast(idx, BODY, ...) excludes self and uses
                // the actor's own filter, so we snap onto exactly the solids it would slide on.
                const units::displacement probe{-m_cfg.up * max_drop};
                const auto hit = cast(cid.value, collider_id::BODY, probe, solid_acceptor());
                if (!hit) {
                    return std::nullopt; // nothing below within reach -> fall
                }
                // Only a WALKABLE surface counts, keeping snapped <=> grounded coherent: a >45 deg
                // face below reads as a cliff/wall, not a floor.
                if (!is_walkable(hit->normal)) {
                    return std::nullopt;
                }
                // Land `skin` short of the surface (the same anti-jitter cushion move_and_slide keeps).
                const float dist = std::max(0.0f, hit->toi * max_drop - m_cfg.skin);
                translate(self, -m_cfg.up * dist);
                refit_proxy(self);
                return hit;
            }

            // §19 #3 -- step-up / ledge forgiveness. A mover walking into a small lip (a low step
            // riser, a curb, a tile edge) should ride up over it rather than jam against it.
            // step_up(cid, step, max_step) lifts the actor up to `max_step` (capped by headroom),
            // re-casts the horizontal `step` from the raised position, and -- if the path is now
            // clear AND a WALKABLE tread waits below -- carries it forward over the lip and settles it
            // down onto that tread. A riser taller than `max_step`, no headroom to lift, a ledge with
            // no tread, or a non-walkable (steep/side) tread leaves the actor untouched (nullopt).
            //
            // CONTRACT: step_up performs the WHOLE stepped move from the actor's CURRENT position by
            // `step` -- it IS the move, not a nudge layered on top of one already taken. So `step` is
            // the horizontal displacement to apply FROM WHERE THE ACTOR IS NOW (its component along
            // `up` is ignored), and the caller must not also apply that same displacement elsewhere:
            //   * as an alternative to a normal slide: call from the pre-move position with the full
            //     frame delta (velocity*dt); on success the horizontal move is DONE for this frame --
            //     do not also run the ordinary slide.
            //   * after a slide that hit a lip: pass the REMAINING delta (intended - already-moved),
            //     NOT the original frame delta, or the actor is advanced twice (move_and_slide has
            //     already carried it up to the lip).
            // Returns the tread contact it settled on (normal usable for slope-aligned velocity), or
            // nullopt when nothing was stepped over. POLICY is the caller's: enable it for characters
            // that should climb steps, not for crates/projectiles. Built on the self-excluding swept
            // cast + translate -- no new physics primitive.
            //
            // Call it on a body resting at the SKIN-SHORT gap the solver leaves (the post-run state):
            // a body flush against its support reads every cast as a toi-0 contact (the support masks
            // the obstruction), so a flush actor must be nudged off-surface first.
            [[nodiscard]] std::optional <contact> step_up(collider_id cid, vec step, float max_step) {
                ENFORCE(is_valid(cid) && cid.type_id == collider_id::BODY);
                auto& self = m_bodies_storage[cid.value];
                ENFORCE(self.kind == detail::body_kind::KINEMATIC)("step_up: actor must be kinematic");
                ENFORCE(max_step >= 0.0f)("step_up: max_step must be non-negative");

                // Only the horizontal part carries the actor over a lip (vertical intent is gravity/jump).
                const vec horiz = step - euler::dot(step, m_cfg.up) * m_cfg.up;
                if (near_zero(horiz)) {
                    return std::nullopt;
                }
                const units::displacement fwd{horiz};

                // Nothing in the way: not a step-up situation -- leave the move to the normal solver.
                if (!cast(cid.value, collider_id::BODY, fwd, solid_acceptor())) {
                    return std::nullopt;
                }

                // Probe straight up for headroom, then lift by as much of max_step as actually clears.
                const auto head = cast(cid.value, collider_id::BODY,
                                       units::displacement{m_cfg.up * max_step}, solid_acceptor());
                const float lift = head ? std::max(0.0f, head->toi * max_step - m_cfg.skin) : max_step;
                if (lift <= constants::POINT_EPS) {
                    return std::nullopt; // pinned below a ceiling -- no room to step up
                }
                translate(self, m_cfg.up * lift);

                // From the raised position, must the horizontal step now clear the lip entirely; a
                // remaining hit means the obstruction is taller than we lifted -> a true wall.
                if (cast(cid.value, collider_id::BODY, fwd, solid_acceptor())) {
                    translate(self, -m_cfg.up * lift); // undo the lift
                    refit_proxy(self);
                    return std::nullopt;
                }
                translate(self, horiz); // carry forward over the lip

                // Settle down onto the tread. We climbed `lift`, so we fall at most that far (never
                // past the original floor). The tread must be WALKABLE (normal faces up past the
                // ground threshold, exactly as snap_to_ground requires) -- a clear forward path does
                // NOT prove a standable surface waits below it. A missing tread (a ledge) OR a steep
                // /side/vertical face means this was not a clean step: undo the whole move so the
                // actor is never left stranded mid-air or "standing" on a wall.
                const auto tread = cast(cid.value, collider_id::BODY,
                                        units::displacement{-m_cfg.up * lift}, solid_acceptor());
                if (!tread || !is_walkable(tread->normal)) {
                    translate(self, -horiz);
                    translate(self, -m_cfg.up * lift);
                    refit_proxy(self);
                    return std::nullopt;
                }
                const float drop = std::max(0.0f, tread->toi * lift - m_cfg.skin);
                translate(self, -m_cfg.up * drop);
                refit_proxy(self);
                return tread;
            }

            // §19 #4 -- footing / edge sensors. Reports the solid ground under three points of the
            // actor's footprint -- left edge, centre, right edge -- by probing straight down (-up) by
            // `max_drop` from each. A STATE the game reads to drive teeter/balance, edge-stop,
            // coyote-time and ledge-grab; the library's job is the query, not the response (a teetering
            // character does not fall -- that is animation, not dynamics). The left/right pair is
            // exactly Sonic's twin floor sensors (A/B). `footing` exposes the raw per-foot contacts
            // plus grounded()/fully_supported()/at_ledge()/ledge_left()/ledge_right() convenience.
            //
            // Each probe is a self-excluding point cast gated to WALKABLE SOLID ground -- a sensor/ignore
            // body is not support, and neither is a steep slope / side / vertical face (normal.up must
            // exceed GROUND_THRESHOLD), so footing agrees with move_and_slide/snap_to_ground/step_up on
            // what "ground" is; a ONE_WAY platform counts only from above. `max_drop` is the reach below
            // the feet that still counts as support -- small for a
            // touching "am I at the edge" check, larger to also catch a step within stride. Pure query
            // (const): it never moves the actor. Built on the existing swept cast -- no new primitive.
            [[nodiscard]] footing ground_support(collider_id cid, float max_drop) const {
                ENFORCE(is_valid(cid) && cid.type_id == collider_id::BODY);
                ENFORCE(max_drop >= 0.0f)("ground_support: max_drop must be non-negative");
                const auto& self = m_bodies_storage[cid.value];

                // Footprint = the actor's enclosing box; "down" is -up, "tangent" the perpendicular
                // (={1,0} for the default up={0,1}). The bottom-face centre and half-width follow from
                // the box's support along each axis, so this is correct for any axis-aligned up.
                const aabb fp = detail::tight_box(self.shape);
                const vec up = m_cfg.up;
                const vec down = -up;
                const vec tangent{up.y(), -up.x()};
                const vec half = (fp.max - fp.min) * 0.5f;
                const vec mid = (fp.min + fp.max) * 0.5f;
                const float half_w = std::abs(tangent.x()) * half.x() + std::abs(tangent.y()) * half.y();
                const float half_d = std::abs(down.x()) * half.x() + std::abs(down.y()) * half.y();
                const vec foot = mid + down * half_d; // bottom-face centre

                const units::displacement reach{down * max_drop};
                // "Support" means WALKABLE ground, not merely a solid: a steep slope / side face /
                // vertical segment underfoot is not something you stand on, and the rest of the system
                // (move_and_slide's grounded, snap_to_ground, step_up) only counts normal.up >
                // GROUND_THRESHOLD as ground. Gate the probe the same way so footing agrees. A near
                // steep face does not mask a walkable surface farther down: cast_core skips rejected
                // candidates, so a flat floor below a steep lip (within max_drop) is still found.
                auto walkable_solid = [this](const material_props& m, const vec& n) {
                    return solid_pred{}(m, n) && is_walkable(n);
                };
                auto probe = [&](const vec& p) -> std::optional <contact> {
                    // a zero-size aabb point swept down: self-excluded + walkable-solid gated, like a
                    // raycast that respects material AND slope (a plain raycast would report sensors,
                    // steep walls, and the actor itself).
                    return cast_core(moving_shape_t{aabb{p, p}}, reach, self.filter,
                                     walkable_solid, cid.value);
                };

                footing f;
                f.left = probe(foot - tangent * half_w);
                f.centre = probe(foot);
                f.right = probe(foot + tangent * half_w);
                return f;
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
                    if (is_walkable(n)) {
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
                const aabb tight = detail::tight_box(self.shape);
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

            // THE single definition of "is this surface standable ground?" -- used by move_and_slide's
            // grounded flag, snap_to_ground, step_up's tread, and footing, so they cannot drift apart
            // (every review of this cluster caught a copy of this check spelled inconsistently).
            [[nodiscard]] bool is_walkable(const vec& contact_normal) const {
                return euler::dot(contact_normal, m_cfg.up) > GROUND_THRESHOLD;
            }

            // Finalize a multi-hit result: order nearest-first by toi, then (max_hits > 0) keep only
            // the nearest N. Shared by raycast_all / cast_all / swept_triggers / dedup_by_entity.
            [[nodiscard]] static std::vector <contact> nearest_first(std::vector <contact> hits,
                                                                     std::size_t max_hits = 0) {
                std::sort(hits.begin(), hits.end(),
                          [](const contact& a, const contact& b) { return a.toi < b.toi; });
                if (max_hits > 0 && hits.size() > max_hits) {
                    hits.resize(max_hits);
                }
                return hits;
            }

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

            // Boundary-compile the static grid (§19 #4): merge adjacent opted-in solid tiles into
            // bigger AABB residents so a flat/run of tiles has no internal seams to snag fast movers.
            // ONE-SHOT: runs once, lazily, on the first run(); add all mergeable (static) tiles before
            // that. The compile is destructive (it clears the source cells and installs untracked
            // residents), so it is not re-run -- add(tile_body) rejects a mergeable tile afterwards,
            // and clear() resets the bake for a level reload. Non-mergeable tiles stay editable.
            // Drives the grid's generic compile_runs with the world's "mergeable group" rule.
            void compile_static_grid() {
                if (!m_static_grid || m_compiled) {
                    return;
                }
                m_compiled = true;

                // A cell is mergeable iff it opted in and is a solid BLOCK aabb that fills its cell;
                // two such cells share a group iff they have the same material + filter.
                const auto same_group = [](const detail::tile& seed, const detail::tile& cell, const aabb& cb) {
                    if (!cell.mergeable || cell.material.response != response_mode::BLOCK
                        || !std::holds_alternative<aabb>(cell.shape)) {
                        return false;
                    }
                    const aabb cs = std::get<aabb>(cell.shape);
                    const float e = constants::POINT_EPS;
                    const bool fills = std::abs(cs.min.x() - cb.min.x()) < e && std::abs(cs.min.y() - cb.min.y()) < e
                                       && std::abs(cs.max.x() - cb.max.x()) < e && std::abs(cs.max.y() - cb.max.y()) < e;
                    if (!fills) {
                        return false;
                    }
                    const material_props& a = seed.material;
                    const material_props& b = cell.material;
                    const bool same_mat = a.restitution == b.restitution && a.friction == b.friction
                                          && a.response == b.response
                                          && a.block_normal.x() == b.block_normal.x()
                                          && a.block_normal.y() == b.block_normal.y();
                    const bool same_filter = seed.filter.category == cell.filter.category
                                             && seed.filter.mask == cell.filter.mask;
                    return same_mat && same_filter;
                };
                m_static_grid->compile_runs(same_group, [this](const aabb& region, const detail::tile& sample) {
                    const uint32_t i = m_bodies_storage.allocate();
                    auto& stored = m_bodies_storage[i];
                    stored.shape = shape_t{region};
                    stored.kind = detail::body_kind::STATIC;
                    stored.material = sample.material;
                    stored.filter = sample.filter;
                    stored.eid = sample.eid;
                    stored.velocity = vec{0, 0};
                    stored.proxy = insert_leaf(m_space_partition, i, fatten(stored));
                });
            }

            // ---- carriers (moving platforms / conveyors / crushers) -----------------------------

            // True if actor `actor_idx` rides carrier `carrier_idx`. Three gates (plus filter +
            // material), all shape-aware so they hold for aabb/circle riders on ANY `up`:
            //   (1) VERTICAL: the actor's underside (min extent along `up`) rests at/just above the
            //       carrier's top, within the skin gap -- excludes a body wedged against a side.
            //   (2) PERPENDICULAR: the actor and carrier STRICTLY overlap across `up` -- excludes an
            //       aabb touching only a top edge/corner (zero real support; inclusive contact alone
            //       would carry it).
            //   (3) CONTACT (shape-aware): nudging the actor down by the gap makes its REAL shape
            //       intersect the carrier -- excludes a circle merely near a corner (its enclosing
            //       box overlaps but the circle does not touch), which the box gates cannot see.
            // FILTER (layers interact) and MATERIAL (carrier solid from above -- a SENSOR/IGNORE or
            // wrong-side one-way carrier carries nothing) gate first. An exact zero-gap top contact
            // rides cleanly (no degenerate swept normal). Extents are exact: a circle's is
            // center·axis ± r, so a non-cardinal `up` is handled (an enclosing box would overestimate).
            [[nodiscard]] bool is_riding(uint32_t actor_idx, uint32_t carrier_idx) const {
                const auto& a = m_bodies_storage[actor_idx];
                const auto& c = m_bodies_storage[carrier_idx];
                if (!should_collide(a.filter, c.filter)) {
                    return false; // layers don't interact -> not carried
                }
                if (!solid_pred{}(c.material, m_cfg.up)) {
                    return false; // carrier isn't solid from above (SENSOR/IGNORE, or one-way wrong side)
                }

                const vec u = m_cfg.up;
                const vec perp{u.y(), -u.x()};
                const float eps = constants::POINT_EPS;
                const float gap = m_cfg.skin * 2.0f + eps; // touching .. within the move_and_slide skin gap

                // [lo, hi] extent of a shape projected onto `axis` -- exact per shape (a circle is
                // center·axis ± r; others use their enclosing-box corners).
                struct span { float lo, hi; };
                const auto extent = [](const shape_t& s, const vec& axis) -> span {
                    return std::visit([&](const auto& sh) -> span {
                        using S = std::decay_t<decltype(sh)>;
                        if constexpr (std::is_same_v<S, circle>) {
                            const float c0 = sh.center.x() * axis.x() + sh.center.y() * axis.y();
                            return span{c0 - sh.radius, c0 + sh.radius};
                        } else {
                            const aabb b = enclose(sh);
                            const float p0 = b.min.x() * axis.x() + b.min.y() * axis.y();
                            const float p1 = b.max.x() * axis.x() + b.min.y() * axis.y();
                            const float p2 = b.min.x() * axis.x() + b.max.y() * axis.y();
                            const float p3 = b.max.x() * axis.x() + b.max.y() * axis.y();
                            return span{std::min({p0, p1, p2, p3}), std::max({p0, p1, p2, p3})};
                        }
                    }, s);
                };

                // (1) vertical: actor underside at/within-gap of carrier top.
                const span au = extent(a.shape, u), cu = extent(c.shape, u);
                if (!(au.lo >= cu.hi - eps && au.lo <= cu.hi + gap)) {
                    return false;
                }
                // (2) perpendicular: strict overlap across up (not a mere edge/corner touch).
                const span ap = extent(a.shape, perp), cp = extent(c.shape, perp);
                if (!(ap.hi > cp.lo + eps && ap.lo < cp.hi - eps)) {
                    return false;
                }
                // (3) shape-aware contact: the actor, nudged down by the gap, really touches the carrier.
                const vec down{-u.x() * gap, -u.y() * gap};
                const shape_t nudged = std::visit([&](const auto& s) {
                    return shape_t{collide::translate(s, down)};
                }, a.shape);
                return std::visit([&](const auto& as) {
                    return std::visit([&](const auto& cs) { return collide::intersects(as, cs); }, c.shape);
                }, nudged);
            }

            // Re-fit a resident's broadphase proxy after it moved, only when its tight box escaped the
            // stored fat box (same containment short-circuit as move_and_slide).
            void refit_proxy(detail::resident_body& b) {
                const aabb tight = detail::tight_box(b.shape);
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

            // MP3 -- crush: after a carry/push, if the actor still overlaps the carrier (it could not
            // move clear -- pinned against other solid geometry), emit a CRUSH event. The skin gaps a
            // clean carry/push leaves mean an un-pinned actor does not overlap, so this only fires on a
            // genuine pin. `crush_dir` is the (un-normalized) direction the actor was being moved.
            void emit_crush_if_pinned(uint32_t actor_idx, uint32_t carrier_idx, const vec& crush_dir) {
                // STRICT penetration (overlap -> a positive depth), not inclusive intersects: a clean
                // carry/push leaves the actor at best TOUCHING the carrier (a zero-gap rider rides flush
                // on the top), which must NOT read as a crush. Actors and carriers are always movers
                // (aabb|circle), so narrow both -> overlap()'s aabb/circle overloads cover every combo.
                const moving_shape_t a = detail::narrow(m_bodies_storage[actor_idx].shape);
                const moving_shape_t c = detail::narrow(m_bodies_storage[carrier_idx].shape);
                const bool pinned = std::visit([&](const auto& as) {
                    return std::visit([&](const auto& cs) { return collide::overlap(as, cs).has_value(); }, c);
                }, a);
                if (!pinned) {
                    return;
                }
                const float len = euler::length(crush_dir);
                const vec n = len > constants::POINT_EPS
                                  ? vec{crush_dir.x() / len, crush_dir.y() / len}
                                  : vec{0, 0};
                m_events.emplace_back(
                    event_kind::CRUSH,
                    collider_id{actor_idx, m_bodies_storage.generation(actor_idx), collider_id::BODY},
                    collider_id{carrier_idx, m_bodies_storage.generation(carrier_idx), collider_id::BODY},
                    n, 0.0f);
            }

            // Displacement that shoves actor box `a` clear of carrier box `c` ALONG the carrier's
            // motion `d` (only the axes the carrier moves on), leaving a skin gap. Used by MP2 to
            // push an actor the carrier ran into. Box-level (the actual move is shape-aware).
            [[nodiscard]] vec clear_push(const aabb& a, const aabb& c, const vec& d) const {
                const float eps = constants::POINT_EPS;
                const float s = m_cfg.skin;
                const float px = d.x() > eps ? (c.max.x() - a.min.x() + s)
                                 : d.x() < -eps ? (c.min.x() - a.max.x() - s)
                                 : 0.0f;
                const float py = d.y() > eps ? (c.max.y() - a.min.y() + s)
                                 : d.y() < -eps ? (c.min.y() - a.max.y() - s)
                                 : 0.0f;
                return vec{px, py};
            }

        private:
            world_config m_cfg;
            detail::bodies_storage m_bodies_storage;
            detail::bullets_storage m_bullets_storage;

            tree m_space_partition;
            std::optional <grid <detail::tile>> m_static_grid; // statics (tiles); unset = none
            bool m_compiled = false; // the one-shot tile boundary-bake has run (reset by clear())

            std::vector <world_event> m_events;          // reused per-frame event buffer
            std::vector <uint32_t> m_rider_scratch;      // reused per-carrier rider list (carrier pass)
            std::vector <uint32_t> m_push_scratch;       // reused per-carrier pushed-actor list (MP2)
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
