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
#include <optional>
#include <euler/vector/vector.hh>

namespace simplex::collide {
    /**
     * @brief Type alias for 2D float vector
     */
    using vec = euler::vec2f;

    /**
     * @brief Axis-aligned bounding box stored as opposite corners (min <= max component-wise).
     *
     * In standard coordinates, `min` and `max` correspond to the minimum and maximum
     * coordinate components of the box. The geometry assumes `min.x() <= max.x()` and
     * `min.y() <= max.y()` are maintained by the caller.
     */
    struct aabb {
        vec min{}; ///< Component-wise minimum corner of the box (e.g., bottom-left in y-up, top-left in y-down)
        vec max{}; ///< Component-wise maximum corner of the box (e.g., top-right in y-up, bottom-right in y-down)

        /**
         * @brief Get the size of the box (width, height)
         */
        [[nodiscard]] vec size() const noexcept {
            return vec{max.x() - min.x(), max.y() - min.y()};
        }

        /**
         * @brief Get the center point of the box
         */
        [[nodiscard]] vec center() const noexcept {
            return vec{(min.x() + max.x()) * 0.5f, (min.y() + max.y()) * 0.5f};
        }
    };

    /**
     * @brief Circle shape defined by a center point and a radius.
     */
    struct circle {
        vec center{};      ///< Center position vector of the circle in world space
        float radius{0.0f}; ///< Radius of the circle in world units
    };

    /**
     * @brief Directed line segment from a start point to an end point.
     */
    struct segment {
        vec from; ///< Starting point of the segment in world space
        vec to;   ///< Ending point of the segment in world space

        /**
         * @brief Interpolate a point along the segment at a given normalized fraction.
         * @param t Interpolation fraction. Typically in the range [0.0, 1.0], but values
         *          outside this range can be used to extrapolate along the infinite line.
         * @return The interpolated position vector.
         */
        [[nodiscard]] vec point_in_time(float t) const {
            return from + t*(to - from);
        }
    };

    // -------------------------------------------------------------------------
    // Queries
    // -------------------------------------------------------------------------

    /**
     * @brief Test if an AABB contains a point
     * @param b Bounding box
     * @param p Point vector to test
     * @return true if point is inside or on the boundary of the box
     */
    [[nodiscard]] constexpr bool contains(const aabb& b, const vec& p) noexcept {
        return p.x() >= b.min.x() && p.x() <= b.max.x() &&
               p.y() >= b.min.y() && p.y() <= b.max.y();
    }

    /**
     * @brief Test if a circle contains a point
     * @param c Circle shape
     * @param p Point vector to test
     * @return true if point is inside or on the boundary of the circle
     */
    [[nodiscard]] constexpr bool contains(const circle& c, const vec& p) noexcept {
        const float dx = p.x() - c.center.x();
        const float dy = p.y() - c.center.y();
        return dx * dx + dy * dy <= c.radius * c.radius;
    }

    /**
     * @brief Test if two static AABBs intersect
     * @param a First bounding box
     * @param b Second bounding box
     * @return true if the boxes overlap or touch
     */
    [[nodiscard]] constexpr bool intersects(const aabb& a, const aabb& b) noexcept {
        return a.min.x() <= b.max.x() && a.max.x() >= b.min.x() &&
               a.min.y() <= b.max.y() && a.max.y() >= b.min.y();
    }

    /**
     * @brief Test if two static circles intersect
     * @param a First circle
     * @param b Second circle
     * @return true if the circles overlap or touch
     */
    [[nodiscard]] constexpr bool intersects(const circle& a, const circle& b) noexcept {
        const float dx = a.center.x() - b.center.x();
        const float dy = a.center.y() - b.center.y();
        const float r = a.radius + b.radius;
        return dx * dx + dy * dy <= r * r;
    }

    /**
     * @brief Result of swept / continuous collision detection queries.
     *
     * Depending on the query function used:
     * - In raw segment queries (e.g., @ref intersect_times), `entry` and `exit` represent 
     *   normalized path fractions along the segment (typically in the range [0, 1]).
     * - In continuous collision queries (e.g., @ref intersects), `entry` and `exit` 
     *   represent absolute simulation times (scaled by the maximum query duration).
     */
    struct entry_exit_times {
        float entry{};      ///< Time/fraction of entry (overlap start)
        float exit{};       ///< Time/fraction of exit (overlap end)
        vec entry_normal{}; ///< Unit normal pointing outwards from the target obstacle at the entry point
        vec exit_normal{};  ///< Unit normal pointing outwards from the target obstacle at the exit point
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
    /**
     * @brief Computes swept intersection times/fractions and normals for an AABB and a line segment.
     *
     * This function uses the slab method (Kay and Kajiya) to calculate when a ray/segment
     * intersects the axis-aligned bounding box. It projects the segment onto each axis and
     * determines the entry/exit intervals.
     *
     * @note The returned entry and exit values are normalized path fractions along the segment.
     *       - If `entry <= exit`, the infinite line containing the segment intersects the AABB.
     *       - If `0.0f <= entry <= 1.0f` and `entry <= exit`, the finite segment enters the AABB.
     *       - If `entry < 0.0f` and `exit >= 0.0f`, the segment's starting point is already inside the AABB.
     *
     * @param a The axis-aligned bounding box (target obstacle) to test against.
     * @param b The line segment representing the swept path of a point.
     * @return An @ref entry_exit_times structure containing:
     *         - `entry`: Normalized fraction along the segment where entry occurs (can be < 0 or > 1).
     *         - `exit`: Normalized fraction along the segment where exit occurs (can be < 0 or > 1).
     *         - `entry_normal`: Unit normal pointing outwards from the box side where the segment entered.
     *         - `exit_normal`: Unit normal pointing outwards from the box side where the segment exited.
     */
    [[nodiscard]] constexpr entry_exit_times intersect_times(const aabb& a, const segment& b) noexcept {
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

        vec entry_normal{};
        if (t_entry == t_min_x) {
            entry_normal = vec{ (dx >= 0.0f) ? -1.0f : 1.0f, 0.0f };
        } else {
            entry_normal = vec{ 0.0f, (dy >= 0.0f) ? -1.0f : 1.0f };
        }

        vec exit_normal{};
        if (t_exit == t_max_x) {
            exit_normal = vec{ (dx >= 0.0f) ? 1.0f : -1.0f, 0.0f };
        } else {
            exit_normal = vec{ 0.0f, (dy >= 0.0f) ? 1.0f : -1.0f };
        }

        return {t_entry, t_exit, entry_normal, exit_normal};
    }

    /**
     * @brief Checks if the infinite line containing the swept segment intersects the bounding box.
     *
     * An infinite line intersects the box if and only if the entry time is less than or
     * equal to the exit time (the intervals along all coordinate axes overlap).
     *
     * @param e Swept collision results from @ref intersect_times.
     * @return true if the infinite line intersects the AABB, false otherwise.
     */
    [[nodiscard]] constexpr bool line_intersects(const entry_exit_times& e) {
        return  e.entry <= e.exit;
    }

    /**
     * @brief Checks if the finite line segment [0, 1] intersects the bounding box.
     *
     * A finite segment intersects the box if the infinite line intersects it (entry <= exit)
     * and the intersection overlap interval [entry, exit] overlaps the segment range [0.0, 1.0].
     * Specifically, the entry fraction must be <= 1.0 and the exit fraction must be >= 0.0.
     *
     * @param e Swept collision results from @ref intersect_times.
     * @return true if the finite segment intersects/overlaps the AABB, false otherwise.
     */
    [[nodiscard]] constexpr bool segment_intersects(const entry_exit_times& e) {
        return line_intersects(e) && (e.entry <= 1.0f) && (e.exit >= 0.0f);
    }

    /**
     * @brief Continuous collision detection (CCD) query for two moving AABBs over a time interval.
     *
     * This function checks if two moving axis-aligned bounding boxes (AABBs) collide
     * during a given time step. It uses a Minkowski sum formulation:
     * 1. The first box (`abox`) is shrunk to a single moving point (its center).
     * 2. The second box (`bbox`) is inflated by the half-dimensions of the first box to form a Minkowski "image" box.
     * 3. The relative velocity `avel - bvel` is used to trace a relative displacement segment over the time duration.
     * 4. The query is solved by checking if the moving point's path segment intersects the inflated "image" box.
     *
     * @note Time Clamping (Why `std::max` is used):
     *       If the two AABBs are already overlapping at the start of the time interval (t = 0), the raw swept segment
     *       query will return a negative entry time. Since the collision is already active, this function clamps the
     *       returned `entry` time to `0.0f` using `std::max(0.0f, it.entry * time)`.
     *
     * @note Return Value Units:
     *       Unlike @ref intersect_times which returns normalized fractions in [0, 1], this function returns
     *       absolute collision times scaled by the `time` parameter (e.g., in seconds). The returned `entry`
     *       and `exit` fields will lie within the range [0.0f, time] and do not require further scaling.
     *
     * @param abox First bounding box.
     * @param avel Velocity of the first bounding box (world units / s).
     * @param bbox Second bounding box.
     * @param bvel Velocity of the second bounding box (world units / s).
     * @param time Maximum time duration of the query (s).
     * @return A std::optional containing the entry/exit times and normals if a collision occurs within [0, time],
     *         or std::nullopt if the boxes do not collide during this interval.
     */
    inline
    std::optional<entry_exit_times> intersects(const aabb& abox, const vec& avel, const aabb& bbox, const vec& bvel, float time) {
        const auto rel_speed = avel - bvel;
        const auto from = abox.center();
        const vec  to = from + time*rel_speed;

        const vec abox_size = abox.size() / 2.0f;
        auto image = bbox;
        image.min = image.min - abox_size;
        image.max = image.max + abox_size;

        auto it = intersect_times(image, {from, to});
        if (!segment_intersects(it)) {
            return std::nullopt;
        }
        // Convert segment fractions to actual simulation times
        it.entry = std::max(0.0f, it.entry * time);
        it.exit = it.exit * time;
        return it;
    }
} // namespace simplex::collide
