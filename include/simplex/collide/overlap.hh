#pragma once

// =============================================================================
// Static overlap predicates (do two stationary shapes overlap right now?).
// The segment/segment boolean lives in <simplex/collide/sweep.hh> next to its
// parametric implementation.
// =============================================================================

#include <simplex/collide/types.hh>
#include <simplex/collide/distance.hh>

namespace simplex::collide {
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
    [[nodiscard]] constexpr bool intersects(const circle& a, const aabb& b) noexcept {
        return squared_distance(a.center, b) <= a.radius * a.radius;
    }

    /// @brief Argument-order-independent overload of @ref intersects(const circle&, const aabb&).
    [[nodiscard]] constexpr bool intersects(const aabb& a, const circle& b) noexcept {
        return intersects(b, a);
    }
} // namespace simplex::collide
