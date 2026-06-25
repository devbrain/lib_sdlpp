//
// Created by igor on 25/06/2026.
//
// Solid-triangle narrow-phase. A triangle is a 2-D region (its interior is solid), so it
// blocks from every side -- unlike a single `segment`, which only blocks crossing its line.
// Used for free-standing solid slopes/ramps.
//
// Everything here is built by DECOMPOSITION: a triangle is its three edge segments plus a
// solid interior. So each query reuses the already-tested segment machinery (intersects /
// intersect_param / swept_intersection vs a segment) over the three edges, and adds the
// interior ("is this point inside?") cases the edges alone do not cover. No new SAT/CCD math.
//
#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <cmath>

#include <euler/vector/vector_ops.hh>
#include <simplex/collide/types.hh>
#include <simplex/collide/overlap.hh> // intersects(segment,*), contains(aabb/circle,vec)
#include <simplex/collide/sweep.hh>   // intersect_param / swept_intersection / intersects(segment,*)
#include <simplex/collide/enclose.hh>
#include <simplex/collide/translate.hh>

namespace simplex::collide {
    namespace detail {
        // The triangle's three directed edges (a->b, b->c, c->a) as segments.
        [[nodiscard]] inline std::array<segment, 3> tri_edges(const triangle& t) {
            return {segment{t.a, t.b}, segment{t.b, t.c}, segment{t.c, t.a}};
        }

        [[nodiscard]] inline vec tri_centroid(const triangle& t) {
            return vec{(t.a.x() + t.b.x() + t.c.x()) / 3.0f, (t.a.y() + t.b.y() + t.c.y()) / 3.0f};
        }

        // Unit outward normal of edge p->q for a triangle whose interior lies toward `inside`
        // (e.g. the centroid). Perpendicular to the edge, flipped to point AWAY from the interior.
        [[nodiscard]] inline vec edge_outward_normal(const vec& p, const vec& q, const vec& inside) {
            vec n{-(q.y() - p.y()), q.x() - p.x()};
            const vec mid{(p.x() + q.x()) * 0.5f, (p.y() + q.y()) * 0.5f};
            if (euler::dot(n, inside - mid) > 0.0f) {
                n = -n; // points toward the interior -> flip
            }
            const float len = euler::length(n);
            return len > constants::NORMALIZE_EPS ? n / len : vec{0.0f, 0.0f};
        }
    }

    /// @brief Test if a (solid) triangle contains a point (inclusive of the boundary).
    ///
    /// The bounding-box guard also gives a DEGENERATE (collinear) triangle the documented
    /// "behaves like its longest edge" semantics: the same-sign cross test alone is true for the
    /// whole infinite line when the area is zero, but the bbox of three collinear points is exactly
    /// the longest edge's extent, so the two together accept only points on that edge.
    [[nodiscard]] inline bool contains(const triangle& t, const vec& p) noexcept {
        const float min_x = std::min({t.a.x(), t.b.x(), t.c.x()});
        const float max_x = std::max({t.a.x(), t.b.x(), t.c.x()});
        const float min_y = std::min({t.a.y(), t.b.y(), t.c.y()});
        const float max_y = std::max({t.a.y(), t.b.y(), t.c.y()});
        if (p.x() < min_x || p.x() > max_x || p.y() < min_y || p.y() > max_y) {
            return false;
        }
        const auto cross = [](const vec& u, const vec& v, const vec& w) {
            return (v.x() - u.x()) * (w.y() - u.y()) - (v.y() - u.y()) * (w.x() - u.x());
        };
        const float d1 = cross(t.a, t.b, p);
        const float d2 = cross(t.b, t.c, p);
        const float d3 = cross(t.c, t.a, p);
        const bool has_neg = d1 < 0.0f || d2 < 0.0f || d3 < 0.0f;
        const bool has_pos = d1 > 0.0f || d2 > 0.0f || d3 > 0.0f;
        return !(has_neg && has_pos); // all the same sign (or zero) -> inside / on an edge
    }

    // ---- static overlap (intersects) ---------------------------------------------------------

    [[nodiscard]] inline bool intersects(const triangle& t, const segment& s) noexcept {
        for (const auto& e : detail::tri_edges(t)) {
            if (intersects(e, s)) {
                return true;
            }
        }
        return contains(t, s.from); // s lies entirely inside the triangle
    }

    [[nodiscard]] inline bool intersects(const triangle& t, const circle& c) noexcept {
        for (const auto& e : detail::tri_edges(t)) {
            if (intersects(e, c)) {
                return true;
            }
        }
        return contains(t, c.center); // circle entirely inside the triangle
    }

    [[nodiscard]] inline bool intersects(const triangle& t, const aabb& b) noexcept {
        for (const auto& e : detail::tri_edges(t)) {
            if (intersects(e, b)) {
                return true; // an edge crosses or lies inside the box (covers triangle-inside-box too)
            }
        }
        // otherwise overlap only if the box is entirely inside the triangle -> a box corner is inside
        return contains(t, b.min) || contains(t, vec{b.max.x(), b.min.y()})
               || contains(t, vec{b.min.x(), b.max.y()}) || contains(t, b.max);
    }

    [[nodiscard]] inline bool intersects(const triangle& t, const triangle& u) noexcept {
        for (const auto& e : detail::tri_edges(t)) {
            for (const auto& f : detail::tri_edges(u)) {
                if (intersects(e, f)) {
                    return true;
                }
            }
        }
        return contains(t, u.a) || contains(u, t.a); // one entirely inside the other
    }

    // Argument-order-independent overloads (a triangle may be either operand).
    [[nodiscard]] inline bool intersects(const segment& s, const triangle& t) noexcept { return intersects(t, s); }
    [[nodiscard]] inline bool intersects(const circle& c, const triangle& t) noexcept { return intersects(t, c); }
    [[nodiscard]] inline bool intersects(const aabb& b, const triangle& t) noexcept { return intersects(t, b); }

    // ---- ray / segment parameter (for raycast) -----------------------------------------------

    /**
     * @brief Where the line through segment `ray` enters/exits the solid triangle.
     *
     * Mirrors @ref intersect_param(const aabb&, const segment&): the returned params are along
     * the ray's infinite line (NOT clamped to [0,1]); a ray that starts inside the triangle has
     * `entry_param < 0 <= exit_param`. `entry_normal` is the triangle's OUTWARD normal at the
     * entry edge. Returns nullopt if the line misses the triangle.
     */
    [[nodiscard]] inline std::optional<line_hit> intersect_param(const triangle& t, const segment& ray) noexcept {
        const vec dir = ray.to - ray.from;
        const float dir_sq = euler::dot(dir, dir);
        if (dir_sq < constants::POINT_EPS * constants::POINT_EPS) {
            return std::nullopt; // degenerate (zero-length) ray
        }
        const vec ctr = detail::tri_centroid(t);

        float entry = constants::INF;
        float exit = constants::NEG_INF;
        vec entry_n{}, exit_n{};
        bool any = false;

        for (const auto& e : detail::tri_edges(t)) {
            const vec ed = e.to - e.from;
            const float H = euler::cross(dir, ed);
            const float ed_sq = euler::dot(ed, ed);
            // Parallel (or collinear) edge: its crossing is a grazing/degenerate touch -- the other
            // two edges carry the entry/exit. Skip, scaled like the segment-segment test.
            if (H * H <= constants::PARALLEL_REL_EPS * constants::PARALLEL_REL_EPS * dir_sq * ed_sq) {
                continue;
            }
            const vec q = e.from - ray.from;
            const float tt = euler::cross(q, ed) / H;  // param along the ray's infinite line
            const float uu = euler::cross(q, dir) / H; // param along the edge
            if (uu < 0.0f || uu > 1.0f) {
                continue; // the crossing is outside this edge
            }
            any = true;
            const vec on = detail::edge_outward_normal(e.from, e.to, ctr);
            if (tt < entry) { entry = tt; entry_n = on; }
            if (tt > exit) { exit = tt; exit_n = on; }
        }
        if (!any) {
            return std::nullopt;
        }
        return line_hit{entry, exit, entry_n, exit_n};
    }

    // ---- swept (continuous) collision: a moving aabb / circle vs a static-ish triangle --------

    namespace detail {
        // Earliest contact of a swept mover with the triangle's boundary (its three edges), or --
        // if the mover never touches an edge but already overlaps the triangle at t=0 -- a toi-0
        // contact with the nearest edge's outward normal (the push-out direction).
        template<class Mover>
        [[nodiscard]] std::optional<swept_hit> swept_tri(const Mover& m, const vec& mv,
                                                         const triangle& t, const vec& tv, float time,
                                                         const vec& mover_centre) {
            // Already overlapping at t=0 -> a toi-0 contact, REGARDLESS of velocity. This must be
            // checked before the edge sweep: a mover inside the solid moving toward an edge would
            // otherwise report that future boundary hit instead of the penetration it starts in.
            // The push-out normal is the nearest edge's outward normal.
            if (intersects(t, m)) {
                const vec ctr = tri_centroid(t);
                float best_d = constants::INF;
                vec n{};
                for (const auto& e : tri_edges(t)) {
                    const float d = squared_distance(mover_centre, e);
                    if (d < best_d) {
                        best_d = d;
                        n = edge_outward_normal(e.from, e.to, ctr);
                    }
                }
                return swept_hit{0.0f, time, n, n};
            }
            // Otherwise: the earliest of the swept-vs-each-edge contacts (first boundary touched).
            std::optional<swept_hit> best;
            for (const auto& e : tri_edges(t)) {
                if (const auto h = swept_intersection(m, mv, e, tv, time)) {
                    if (!best || h->entry_time < best->entry_time) {
                        best = h;
                    }
                }
            }
            return best;
        }
    }

    [[nodiscard]] inline std::optional<swept_hit> swept_intersection(const aabb& m, const vec& mv,
                                                                     const triangle& t, const vec& tv,
                                                                     float time) {
        return detail::swept_tri(m, mv, t, tv, time, m.center());
    }

    [[nodiscard]] inline std::optional<swept_hit> swept_intersection(const circle& m, const vec& mv,
                                                                     const triangle& t, const vec& tv,
                                                                     float time) {
        return detail::swept_tri(m, mv, t, tv, time, m.center);
    }

    // ---- enclose / translate -----------------------------------------------------------------

    [[nodiscard]] inline aabb enclose(const triangle& t) noexcept {
        return aabb{
            vec{std::min({t.a.x(), t.b.x(), t.c.x()}), std::min({t.a.y(), t.b.y(), t.c.y()})},
            vec{std::max({t.a.x(), t.b.x(), t.c.x()}), std::max({t.a.y(), t.b.y(), t.c.y()})}
        };
    }

    [[nodiscard]] constexpr triangle translate(const triangle& t, const vec& v) {
        return {translate(t.a, v), translate(t.b, v), translate(t.c, v)};
    }
} // namespace simplex::collide
