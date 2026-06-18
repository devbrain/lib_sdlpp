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

#include <cmath>
#include <limits>
#include <tuple>
#include <algorithm>
#include <optional>
#include <euler/vector/vector.hh>
#include <euler/vector/vector_ops.hh>

namespace simplex::collide {
    namespace constants {
        inline constexpr auto EPSILON = 1e-6f;
        inline constexpr auto INF = std::numeric_limits <float>::infinity();
        inline constexpr auto NEG_INF = -std::numeric_limits <float>::infinity();
    }

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
        vec center{}; ///< Center position vector of the circle in world space
        float radius{0.0f}; ///< Radius of the circle in world units
    };

    /**
     * @brief Directed line segment from a start point to an end point.
     */
    struct segment {
        vec from; ///< Starting point of the segment in world space
        vec to; ///< Ending point of the segment in world space

        /**
         * @brief Interpolate a point along the segment at a given normalized fraction.
         * @param t Interpolation fraction. Typically in the range [0.0, 1.0], but values
         *          outside this range can be used to extrapolate along the infinite line.
         * @return The interpolated position vector.
         */
        [[nodiscard]] vec point_in_time(float t) const {
            return from + t * (to - from);
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
        float entry{}; ///< Time/fraction of entry (overlap start)
        float exit{}; ///< Time/fraction of exit (overlap end)
        vec entry_normal{}; ///< Unit normal pointing outwards from the target obstacle at the entry point
        vec exit_normal{}; ///< Unit normal pointing outwards from the target obstacle at the exit point
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

        float t_min_x, t_max_x;
        if (dx == 0.0f) {
            if (b.from.x() >= a.min.x() && b.from.x() <= a.max.x()) {
                t_min_x = constants::NEG_INF;
                t_max_x = constants::INF;
            } else {
                t_min_x = constants::INF;
                t_max_x = constants::NEG_INF;
            }
        } else {
            const float tx1 = (a.min.x() - b.from.x()) / dx;
            const float tx2 = (a.max.x() - b.from.x()) / dx;
            t_min_x = std::min(tx1, tx2);
            t_max_x = std::max(tx1, tx2);
        }

        float t_min_y, t_max_y;
        if (dy == 0.0f) {
            if (b.from.y() >= a.min.y() && b.from.y() <= a.max.y()) {
                t_min_y = constants::NEG_INF;
                t_max_y = constants::INF;
            } else {
                t_min_y = constants::INF;
                t_max_y = constants::NEG_INF;
            }
        } else {
            const float ty1 = (a.min.y() - b.from.y()) / dy;
            const float ty2 = (a.max.y() - b.from.y()) / dy;
            t_min_y = std::min(ty1, ty2);
            t_max_y = std::max(ty1, ty2);
        }

        const float t_entry = std::max(t_min_x, t_min_y);
        const float t_exit = std::min(t_max_x, t_max_y);

        vec entry_normal{};
        if (t_entry == t_min_x) {
            entry_normal = vec{(dx >= 0.0f) ? -1.0f : 1.0f, 0.0f};
        } else {
            entry_normal = vec{0.0f, (dy >= 0.0f) ? -1.0f : 1.0f};
        }

        vec exit_normal{};
        if (t_exit == t_max_x) {
            exit_normal = vec{(dx >= 0.0f) ? 1.0f : -1.0f, 0.0f};
        } else {
            exit_normal = vec{0.0f, (dy >= 0.0f) ? 1.0f : -1.0f};
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
        return e.entry <= e.exit;
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
    std::optional <entry_exit_times> intersects(const aabb& abox, const vec& avel, const aabb& bbox, const vec& bvel,
                                                float time) {
        const auto rel_speed = avel - bvel;
        const auto from = abox.center();
        const vec to = from + time * rel_speed;

        const vec abox_size = abox.size() / 2.0f;
        auto image = bbox;
        image.min = image.min - abox_size;
        image.max = image.max + abox_size;

        auto it = intersect_times(image, {from, to});
        if (!segment_intersects(it)) {
            return std::nullopt;
        }
        // Convert segment fractions to actual simulation times, clamped to [0, time].
        // Clamp the fraction (which may be +/-inf for an already-overlapping start) to a
        // finite [0, 1] *before* scaling, so time == 0 yields 0 rather than inf * 0 == NaN.
        it.entry = std::max(0.0f, it.entry) * time;
        it.exit = std::min(it.exit, 1.0f) * time;
        return it;
    }

    /**
     * @brief Calculates the projection parameter of a point onto a line segment, clamped to [0.0, 1.0].
     *
     * This function projects the target point onto the line containing the segment and returns the
     * normalized interpolation fraction t. If the segment has zero length (degenerate), it returns 0.0f.
     * The returned value is clamped to the range [0.0f, 1.0f], representing the closest point along the
     * finite segment boundaries.
     *
     * @param dst The query point vector.
     * @param s The line segment.
     * @return The normalized projection parameter in the range [0.0f, 1.0f].
     */
    [[nodiscard]] inline float closest_parameter(const vec& dst, const segment& s) {
        const vec d = s.to - s.from;
        if (euler::approx_zero(d, constants::EPSILON)) {
            return 0.0f;
        }
        const float len_sq = euler::dot(d, d);
        return std::clamp(euler::dot(dst - s.from, d) / len_sq, 0.0f, 1.0f);
    }

    /**
     * @brief Finds the closest point on a line segment to a query point.
     *
     * This function calculates the coordinates of the point on the finite segment `s` that is nearest
     * to the query point `dst`. It evaluates the closest clamped parameter along the segment.
     *
     * @param dst The query point vector.
     * @param s The line segment.
     * @return The coordinate vector of the closest point on the segment.
     */
    [[nodiscard]] inline vec closest_point(const vec& dst, const segment& s) {
        return s.point_in_time(closest_parameter(dst, s));
    }

    /**
     * @brief Finds the closest point on an AABB to a query point.
     *
     * This function projects the target point `dst` onto the boundary or interior
     * of the axis-aligned bounding box `s` by clamping its coordinates.
     *
     * @param dst The query point vector.
     * @param s The axis-aligned bounding box.
     * @return The coordinate vector of the closest point on or inside the AABB.
     */
    [[nodiscard]] constexpr vec closest_point(const vec& dst, const aabb& s) {
        return {
            std::clamp(dst.x(), s.min.x(), s.max.x()),
            std::clamp(dst.y(), s.min.y(), s.max.y())
        };
    }

    /**
     * @brief Calculates the squared distance between a point and a line segment.
     *
     * This function computes the shortest squared Euclidean distance from the query
     * point `p` to any point on the finite segment `s`.
     *
     * @param p The query point vector.
     * @param s The line segment.
     * @return The squared distance as a float.
     */
    [[nodiscard]] inline float squared_distance(const vec& p, const segment& s) {
        const vec ab = s.to - s.from;
        const vec ac = p - s.from;
        const vec bc = p - s.to;

        const float e = euler::dot(ac, ab);
        if (e <= 0.0f) {
            return euler::dot(ac, ac);
        }
        const float f = euler::dot(ab, ab);
        if (e >= f) {
            return euler::dot(bc, bc);
        }
        // Interior case: perpendicular distance squared. Use the cross-product form
        // (|ab x ac|^2 / |ab|^2) rather than |ac|^2 - e^2/f, which loses all precision to
        // catastrophic cancellation when |ac| is large and the perpendicular offset small.
        const float cr = euler::cross(ab, ac);
        return cr * cr / f;
    }

    /**
     * @brief Calculates the squared distance between a point and an AABB.
     *
     * This function computes the shortest squared Euclidean distance from the query
     * point `p` to the boundaries of the axis-aligned bounding box `s`. Returns 0.0f
     * if the point is inside the box.
     *
     * @param p The query point vector.
     * @param s The axis-aligned bounding box.
     * @return The squared distance as a float.
     */
    [[nodiscard]] inline float squared_distance(const vec& p, const aabb& s) {
        float sq_dist = 0.0f;
        auto eval = [&sq_dist](float v, float min, float max) {
            if (v < min) {
                const float d = min - v;
                sq_dist += d * d;
            }
            if (v > max) {
                const float d = max - v;
                sq_dist += d * d;
            }
        };

        eval(p.x(), s.min.x(), s.max.x());
        eval(p.y(), s.min.y(), s.max.y());

        return sq_dist;
    }

    /**
     * @brief Calculates the squared distance between a point and a circle.
     *
     * This function computes the shortest squared Euclidean distance from the query
     * point `p` to the boundaries of the circle `s`. Returns 0.0f if the point is
     * inside or on the boundary of the circle.
     *
     * @param p The query point vector.
     * @param s The circle shape.
     * @return The squared distance as a float.
     */
    [[nodiscard]] inline float squared_distance(const vec& p, const circle& s) {
        const vec dir = s.center - p;
        const float dist_sq = euler::dot(dir, dir);
        if (dist_sq <= s.radius * s.radius) {
            return 0.0f;
        }
        const float d = euler::length(dir) - s.radius;
        return d * d;
    }

    /**
     * @brief Finds the closest point on a circle to a query point.
     *
     * This function calculates the coordinates of the point on the circle `s` that
     * is nearest to the query point `p`. If the point is inside the circle, it returns
     * the point itself.
     *
     * @param p The query point vector.
     * @param s The circle shape.
     * @return The coordinate vector of the closest point on or inside the circle.
     */
    [[nodiscard]] inline vec closest_point(const vec& p, const circle& s) {
        const vec dir = p - s.center;
        if (euler::dot(dir, dir) <= s.radius * s.radius) {
            return p;
        }
        const float len = euler::length(dir);
        return s.center + dir * (s.radius / len);
    }

    /**
     * @brief Computes the intersection times (fractions) and normals between a circle and a segment.
     *
     * This function solves the quadratic equation representing the intersection of the line containing
     * segment `s` with the circle `circ`. It returns the entry and exit times (fractions along the segment)
     * and the outward-pointing unit normals at the intersection points.
     *
     * @note The returned entry and exit times represent normalized fractions along the segment.
     *       - If the return is std::nullopt, the infinite line does not intersect the circle.
     *       - If a value is returned, the segment intersects the circle if the overlap interval
     *         [entry, exit] intersects the segment range [0.0f, 1.0f] (i.e. entry <= 1.0f and exit >= 0.0f).
     *
     * @param circ The circle shape.
     * @param s The line segment.
     * @return std::optional containing the entry/exit times and normals if an intersection exists,
     *         or std::nullopt if the segment's infinite line does not intersect the circle.
     */
    [[nodiscard]] inline std::optional <entry_exit_times>
    intersect_times(const circle& circ, const segment& s) noexcept {
        /*
         * Theory: (all vectors are UPPERCASE)
         * The direction of the segment : D = b.to + b.from
         * Parametric equation of the segment: E(t) = b.from + D*t
         * If the segment intersects the circle at time t, then the point of intersection lies on the boundary of the circle,
         * thus || E(t) - a.center || = r
         *
         * Let X = b.from - a.center. Hence we need to solve equation
         * || X + Dt || = r <-> ||X + Dt||^2 = r^2
         * (X+Dt)*(X+Dt) = r^2
         * X*X + 2(X*D)t + (D*D)t^2 = r^2
         * ||D||^2 t^2 + 2(X*D)t + (||X||^2 - r^2) = 0
         */
        const vec d = s.to - s.from;
        const vec x = s.from - circ.center;
        const auto a = euler::length_squared(d);
        const auto c = euler::length_squared(x) - circ.radius * circ.radius;
        const auto b = 2.0f * euler::dot(x, d);

        // Degenerate (zero-length) segment: the "sweep" is a static point. Handle this
        // first so the static case never falls into the quadratic branch and never pays
        // for the std::sqrt below.
        if (euler::approx_zero(d, constants::EPSILON)) {
            if (c <= 0.0f) {
                // Point is inside or on the circle
                entry_exit_times out;
                out.entry = constants::NEG_INF;
                out.exit = constants::INF;
                // Normals are undefined for a static interior point, leave as zero
                return out;
            }
            return std::nullopt; // Point is outside
        }

        // delta has units of length^4; compare against the magnitude of its constituent
        // terms rather than an absolute epsilon. A tangent (grazing) hit yields delta == 0
        // in exact arithmetic but a tiny negative value after rounding — treat those as a
        // single grazing contact instead of a miss.
        auto delta = b * b - 4.0f * a * c;
        const float delta_scale = std::max(b * b, std::abs(4.0f * a * c));
        if (delta < -constants::EPSILON * delta_scale) {
            return std::nullopt;
        }
        if (delta < 0.0f) {
            delta = 0.0f;
        }
        const auto delta2 = std::sqrt(delta);

        const auto a2 = 2.0f * a;
        const auto t1 = (-b + delta2) / a2; // Standard quadratic formula
        const auto t2 = (-b - delta2) / a2;

        entry_exit_times out;
        out.entry = std::min(t1, t2);
        out.exit = std::max(t1, t2);

        // Outward-pointing unit normal at the circle boundary for a given segment fraction.
        auto outward_normal = [&](float t) {
            const vec v = s.point_in_time(t) - circ.center;
            const float len = euler::length(v);
            return (len > constants::EPSILON) ? (v / len) : vec{0.0f, 0.0f};
        };
        out.entry_normal = outward_normal(out.entry);
        out.exit_normal = outward_normal(out.exit);

        return out;
    }

    /**
     * @brief Test if a static circle and a static AABB intersect.
     *
     * This function determines if the circle `a` overlaps or touches the axis-aligned
     * bounding box `b` by checking if the squared distance from the circle center to
     * the AABB is within the squared radius of the circle.
     *
     * @param a The circle shape.
     * @param b The axis-aligned bounding box.
     * @return true if the shapes overlap or touch, false otherwise.
     */
    [[nodiscard]] inline bool intersects(const circle& a, const aabb& b) noexcept {
        return squared_distance(a.center, b) <= a.radius * a.radius;
    }

    /**
         * @brief Computes the intersection time (fraction) and normal between two segments.
         *
         * This function checks if the finite segment `a` intersects the finite segment `b`.
         * If they intersect, it returns the intersection fraction along segment `a` (as both
         * entry and exit times) and the unit normal of segment `b` pointing opposite to
         * `a`'s direction.
         *
         * @param a The first segment.
         * @param b The second segment.
         * @return std::optional containing the intersection details if they intersect,
         *         or std::nullopt if they are parallel or do not overlap.
         */
    [[nodiscard]] inline std::optional <entry_exit_times> intersect_times(const segment& a, const segment& b) noexcept {
        const vec r = a.to - a.from;
        const vec s_dir = b.to - b.from;
        const float len_sq_r = euler::dot(r, r);
        const float len_sq_s = euler::dot(s_dir, s_dir);

        // Degenerate (point) segments: the general cross-product path is vacuous when a
        // direction vector is ~0, so fall back to point/segment containment. The reported
        // fraction is along segment a, so a point b on a yields its parameter along a,
        // while a degenerate a is the single point at fraction 0.
        const bool a_point = euler::approx_zero(r, constants::EPSILON);
        const bool b_point = euler::approx_zero(s_dir, constants::EPSILON);
        if (a_point || b_point) {
            float frac;
            if (a_point && b_point) {
                if (!euler::approx_equal(a.from, b.from, constants::EPSILON)) {
                    return std::nullopt;
                }
                frac = 0.0f;
            } else if (a_point) {
                if (!euler::approx_equal(closest_point(a.from, b), a.from, constants::EPSILON)) {
                    return std::nullopt;
                }
                frac = 0.0f;
            } else {
                // b is the point: locate it along segment a
                if (!euler::approx_equal(closest_point(b.from, a), b.from, constants::EPSILON)) {
                    return std::nullopt;
                }
                frac = closest_parameter(b.from, a);
            }
            entry_exit_times out;
            out.entry = frac;
            out.exit = frac;
            return out;
        }

        const float H = euler::cross(r, s_dir);

        const vec q = b.from - a.from;

        // Parallel test scaled by the segment magnitudes: |H| <= EPSILON*|r|*|s_dir|.
        // H has units of area, so an absolute epsilon would be scale-dependent.
        if (H * H <= constants::EPSILON * constants::EPSILON * len_sq_r * len_sq_s) {
            // Check if they are collinear, again scaled: |cross(q, r)| <= EPSILON*|q|*|r|.
            const float cross_q_r = euler::cross(q, r);
            const float len_sq_q = euler::dot(q, q);
            if (cross_q_r * cross_q_r <= constants::EPSILON * constants::EPSILON * len_sq_q * len_sq_r) {
                // Project onto r to check interval overlap
                const float t_from = euler::dot(q, r);
                const float t_to = euler::dot(b.to - a.from, r);
                const float t_min = std::min(t_from, t_to);
                const float t_max = std::max(t_from, t_to);

                if (t_min <= len_sq_r && t_max >= 0.0f) {
                    entry_exit_times out;
                    out.entry = std::max(0.0f, t_min / len_sq_r);
                    out.exit = std::min(1.0f, t_max / len_sq_r);

                    vec normal{-r.y(), r.x()};
                    const float n_len = euler::length(normal);
                    if (n_len > constants::EPSILON) {
                        normal = normal / n_len;
                    }
                    // s_dir is parallel to r here, so dot(normal, s_dir) is ~0 and cannot
                    // orient the normal. Use the offset q between the segments instead so
                    // the normal points deterministically away from b toward a.
                    if (euler::dot(normal, q) > 0.0f) {
                        normal = -normal;
                    }
                    out.entry_normal = normal;
                    out.exit_normal = normal;
                    return out;
                }
            }
            return std::nullopt;
        }

        const float t = euler::cross(q, s_dir) / H;
        const float u = euler::cross(q, r) / H;

        // Check if the intersection point lies on both finite segments
        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            entry_exit_times out;
            out.entry = t;
            out.exit = t;

            // Compute the normal of segment b pointing opposite to segment a's direction
            vec normal{-s_dir.y(), s_dir.x()};
            const float len = euler::length(normal);
            if (len > constants::EPSILON) {
                normal = normal / len;
            }
            if (euler::dot(normal, r) > 0.0f) {
                normal = -normal;
            }
            out.entry_normal = normal;
            out.exit_normal = normal;
            return out;
        }

        return std::nullopt;
    }

    /**
     * @brief Test if two segments intersect.
     *
     * @param a The first segment.
     * @param b The second segment.
     * @return true if the finite segments intersect, false otherwise.
     */
    [[nodiscard]] inline bool intersects(const segment& a, const segment& b) noexcept {
        return intersect_times(a, b).has_value();
    }

    /**
     * @brief Continuous collision detection (CCD) query for two moving circles over a time interval.
     *
     * This function checks if two moving circles collide during a given time step. It uses
     * a Minkowski sum formulation by inflating the second circle by the radius of the first
     * and sweeping the center point of the first circle along the relative velocity vector.
     *
     * @param acirc First circle.
     * @param avel Velocity of the first circle (world units / s).
     * @param bcirc Second circle.
     * @param bvel Velocity of the second circle (world units / s).
     * @param time Maximum time duration of the query (s).
     * @return std::optional containing the entry/exit times and normals if a collision occurs
     *         within [0, time], or std::nullopt if they do not collide.
     */
    inline std::optional <entry_exit_times> intersects(
        const circle& acirc, const vec& avel,
        const circle& bcirc, const vec& bvel,
        float time) {
        const vec rel_vel = avel - bvel;
        const circle image{bcirc.center, bcirc.radius + acirc.radius};
        const segment s{acirc.center, acirc.center + rel_vel * time};
        auto it = intersect_times(image, s);
        if (!it || !segment_intersects(*it)) {
            return std::nullopt;
        }

        // Convert segment fractions to actual simulation times in [0, time]. Clamp the
        // fraction (possibly +/-inf for an overlapping start) to finite [0, 1] before
        // scaling, so time == 0 yields 0 rather than inf * 0 == NaN.
        it->entry = std::max(0.0f, it->entry) * time;
        it->exit = std::min(it->exit, 1.0f) * time;
        return it;
    }

    /**
     * @brief Continuous collision detection (CCD) query for a moving circle and a moving AABB.
     *
     * This function checks if a moving circle and a moving AABB collide during a given time step.
     *
     * THEORY:
     * To solve the swept intersection between a circle and an AABB:
     * 1. Transform the query to relative coordinates: AABB is static, circle center moves along
     *    the relative displacement segment: S = acirc.center + t * (avel - bvel) * time.
     * 2. The Minkowski sum of an AABB and a circle of radius R yields a "rounded rectangle"
     *    (a stadium shape). This rounded rectangle is composed of:
     *    - A horizontally inflated box: aabb{min.x - R, min.y, max.x + R, max.y}
     *    - A vertically inflated box:   aabb{min.x, min.y - R, max.x, max.y + R}
     *    - Four circular caps of radius R centered at each corner of the original AABB.
     * 3. The moving point (circle center) intersects the rounded rectangle if and only if it
     *    intersects any of these 6 candidate shapes. We test all 6 and select the earliest
     *    entry time.
     *
     * ILLUSTRATION:
     *                  Rounded Rectangle Minkowski Sum
     *                     +-----------------+
     *                  .  |  Vertical Box   |  .
     *                .    |  (height + 2R)  |    .
     *               +-----+-----------------+-----+
     *               |     |                 |     |
     *   Horizontal  |     |  Original AABB  |     |
     *      Box      |     |                 |     |
     *  (width + 2R) +-----+-----------------+-----+
     *                .    |                 |    .
     *                  .  |                 |  .
     *                     +-----------------+
     *                   (Corner Circles of radius R
     *                    centered at AABB vertices)
     *
     * @param acirc Moving circle.
     * @param avel Velocity of the circle (world units / s).
     * @param bbox Moving axis-aligned bounding box.
     * @param bvel Velocity of the AABB (world units / s).
     * @param time Maximum time duration of the query (s).
     * @return std::optional containing the entry/exit times and normals if a collision occurs
     *         within [0, time], or std::nullopt if they do not collide.
     */
    [[nodiscard]] inline std::optional <entry_exit_times> intersects(
        const circle& acirc, const vec& avel,
        const aabb& bbox, const vec& bvel,
        float time) {
        const auto rel_speed = avel - bvel;
        const auto from = acirc.center;
        const vec to = from + time * rel_speed;
        const segment seg{from, to};
        const float R = acirc.radius;

        // The Minkowski sum (rounded rectangle) is convex, so the swept point's
        // intersection with it is a single interval [min entry, max exit] taken over the
        // sub-shapes the path actually crosses. Track the earliest-entering and
        // latest-exiting candidates separately rather than reading both off one shape.
        std::optional <entry_exit_times> entry_hit = std::nullopt;
        std::optional <entry_exit_times> exit_hit = std::nullopt;

        auto update_best = [&](const entry_exit_times& hit) {
            if (!entry_hit || hit.entry < entry_hit->entry) {
                entry_hit = hit;
            }
            if (!exit_hit || hit.exit > exit_hit->exit) {
                exit_hit = hit;
            }
        };

        // 1. Check Horizontal Expanded Box
        const aabb box_h{{bbox.min.x() - R, bbox.min.y()}, {bbox.max.x() + R, bbox.max.y()}};
        const auto hit_h = intersect_times(box_h, seg);
        if (segment_intersects(hit_h)) {
            update_best(hit_h);
        }

        // 2. Check Vertical Expanded Box
        const aabb box_v{{bbox.min.x(), bbox.min.y() - R}, {bbox.max.x(), bbox.max.y() + R}};
        const auto hit_v = intersect_times(box_v, seg);
        if (segment_intersects(hit_v)) {
            update_best(hit_v);
        }

        // 3. Check the 4 Corner Circles
        const vec corners[4] = {
            {bbox.min.x(), bbox.min.y()}, // Bottom-Left
            {bbox.max.x(), bbox.min.y()}, // Bottom-Right
            {bbox.min.x(), bbox.max.y()}, // Top-Left
            {bbox.max.x(), bbox.max.y()} // Top-Right
        };

        for (const auto& corner : corners) {
            const circle corner_circ{corner, R};
            const auto hit_circ = intersect_times(corner_circ, seg);
            if (hit_circ && segment_intersects(*hit_circ)) {
                update_best(*hit_circ);
            }
        }

        if (!entry_hit) {
            return std::nullopt;
        }

        // Convert segment fractions to actual simulation times in [0, time]. Entry/normal
        // come from the earliest-entering sub-shape; exit/normal from the latest-exiting
        // one (valid because the composite shape is convex).
        // Clamp each fraction (possibly +/-inf for an overlapping start) to finite [0, 1]
        // before scaling, so time == 0 yields 0 rather than inf * 0 == NaN.
        entry_exit_times out;
        out.entry = std::max(0.0f, entry_hit->entry) * time;
        out.entry_normal = entry_hit->entry_normal;
        out.exit = std::min(exit_hit->exit, 1.0f) * time;
        out.exit_normal = exit_hit->exit_normal;
        return out;
    }
} // namespace simplex::collide
