#pragma once

// =============================================================================
// Collision shapes — pure world space.
//
// This header knows nothing about display, dp, scale, or SDL. Everything here
// operates on a plain numeric field (float world units) so the collision math
// can use products of lengths freely (dot, cross, squared distance, SAT
// projections) and can be unit-tested with no window or global scale state.
//
// Conversions to/from display-space (simplex::point / rect / circle, which are
// dp-based) live in <simplex/collide/bridge.hh> and are the *only* place that
// bridges the two coordinate systems.
// =============================================================================

#include <limits>
#include <tuple>
#include <algorithm>
#include <euler/vector/vector.hh>

namespace simplex::collide {
    using vec = euler::vec2f;

    // Axis-aligned bounding box, stored as opposite corners (min <= max
    // componentwise in the convention callers maintain).
    struct aabb {
        vec min{};
        vec max{};

        [[nodiscard]] vec size() const noexcept {
            return vec{max.x() - min.x(), max.y() - min.y()};
        }

        [[nodiscard]] vec center() const noexcept {
            return vec{(min.x() + max.x()) * 0.5f, (min.y() + max.y()) * 0.5f};
        }
    };

    struct circle {
        vec center{};
        float radius{0.0f};
    };

    // line segment from p1 to p2
    struct segment {
        vec from;
        vec to;

        [[nodiscard]] vec point_in_time(float t) const {
            return from + t*(to - from);
        }
    };

    // -------------------------------------------------------------------------
    // Queries
    // -------------------------------------------------------------------------

    [[nodiscard]] constexpr bool contains(const aabb& b, const vec& p) noexcept {
        return p.x() >= b.min.x() && p.x() <= b.max.x() &&
               p.y() >= b.min.y() && p.y() <= b.max.y();
    }

    [[nodiscard]] constexpr bool contains(const circle& c, const vec& p) noexcept {
        const float dx = p.x() - c.center.x();
        const float dy = p.y() - c.center.y();
        return dx * dx + dy * dy <= c.radius * c.radius;
    }

    [[nodiscard]] constexpr bool intersects(const aabb& a, const aabb& b) noexcept {
        return a.min.x() <= b.max.x() && a.max.x() >= b.min.x() &&
               a.min.y() <= b.max.y() && a.max.y() >= b.min.y();
    }

    [[nodiscard]] constexpr bool intersects(const circle& a, const circle& b) noexcept {
        const float dx = a.center.x() - b.center.x();
        const float dy = a.center.y() - b.center.y();
        const float r = a.radius + b.radius;
        return dx * dx + dy * dy <= r * r;
    }

    struct entry_exit_times {
        float entry;
        float exit;
    };

    /*
     * Slab method visual representation (for dx > 0, dy > 0):
     *
     *                     - - - - - - - - - - - - - - - - - - - * ty_far (t_max_y)
     *                                                           /
     *                                                AABB (a)  /
     *                                                         /
     *                     +----------------------------------+
     *                     |                                 /|
     *                     |                                / |
     *                     |                               /  |
     *                     |                              /   |
     *                     |                             /    |
     *                     |                            /     |
     *                     |                           /      |
     *                     +--------------------------/-------+
     *                                               /|       x = a.max.x (tx_far / t_max_x)
     *                                              / |
     *                                             /  |
     *                     - - - - - - - - - - - -* - - - - - - - - - - -
     *                                           /    |
     *                                 ty_near  /     |
     *                                (t_min_y)       x = a.min.x (tx_near / t_min_x)
     *                                       /
     *                                      / segment (b)
     *                                     /
     *
     *  The overlap of X-interval [t_min_x, t_max_x] and Y-interval [t_min_y, t_max_y]
     *  yields the entry and exit times for the AABB:
     *    t_entry = max(t_min_x, t_min_y)
     *    t_exit  = min(t_max_x, t_max_y)
     */
    [[nodiscard]] constexpr entry_exit_times intersect_times(const aabb& a, const segment& b) noexcept {
        // direction vector, by component (avoid materializing an euler
        // expression template here so this stays usable in a constant
        // expression).
        const float dx = b.to.x() - b.from.x();
        const float dy = b.to.y() - b.from.y();
        auto safe_div = [](const float p, const float q) {
            constexpr auto inf = std::numeric_limits <float>::infinity();
            if (q != 0) {
                return p / q;
            }
            if (p <= 0) {
                return -inf;
            }
            return inf;
        };

        // times of intersection of the segment with aabb
        const float tx_near = safe_div(a.min.x() - b.from.x(), dx);
        const float tx_far  = safe_div(a.max.x() - b.from.x(), dx);
        const float ty_near = safe_div(a.min.y() - b.from.y(), dy);
        const float ty_far  = safe_div(a.max.y() - b.from.y(), dy);

        const float t_min_x = std::min(tx_near, tx_far);
        const float t_max_x = std::max(tx_near, tx_far);
        const float t_min_y = std::min(ty_near, ty_far);
        const float t_max_y = std::max(ty_near, ty_far);

        const float t_entry = std::max(t_min_x, t_min_y);
        const float t_exit = std::min(t_max_x, t_max_y);

        return {t_entry, t_exit};
    }

    [[nodiscard]] constexpr bool line_intersects(const entry_exit_times& e) {
        return  e.entry <= e.exit;
    }

    [[nodiscard]] constexpr bool segment_intersects(const entry_exit_times& e) {
        return line_intersects(e) && (e.entry <= 1.0f) && (e.exit >= 0.0f);
    }

} // namespace simplex::collide
