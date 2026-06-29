#pragma once

/**
 * @file bridge.hh
 * @brief The view transform bridging physics world space <-> display space (dp) -- the ONE file
 *        that knows both coordinate systems.
 *
 * The collision core (@ref shapes.hh) stays in pure world-space floats; the geometry types
 * (@c <simplex/geometry.hh>) stay in display dp. Everything that crosses between them goes through
 * the @c to_world() / @c to_design() adapters defined here.
 *
 * The crossing is a real view transform, not a type relabel: world units are scaled to design
 * pixels, the physics origin need not be the window's top-left, and physics is conventionally
 * y-up while the screen is y-down.
 *
 * @note @c view{} is a true identity (no scale, no offset, no flip), so display space and world
 *       space coincide numerically -- convenient for screen-space hit tests while a real camera
 *       does not yet exist. Flip @c y_up / set @c pixels_per_unit / move @c origin to introduce an
 *       actual physics world; nothing in @ref shapes.hh or in callers' collision logic changes
 *       when you do.
 */

#include <algorithm>

#include <simplex/dp.hh>
#include <simplex/geometry.hh>
#include <simplex/collide/shapes.hh>

namespace simplex {
    /**
     * @brief The parameters of the world-space <-> display-space view transform.
     *
     * Captures the scale, origin offset, and optional y-axis flip that every @c to_world /
     * @c to_design adapter in this file applies. A default-constructed @c view is the identity
     * transform.
     */
    struct view {
        /// @brief World coordinate displayed at design-space (0, 0).
        collide::vec origin{};
        /**
         * @brief World units -> design pixels.
         * @note Distinct from dp's display scale, which maps design pixels -> physical pixels at
         *       render time.
         */
        float pixels_per_unit = 1.0f;
        /**
         * @brief Physics y-up (@c true) vs screen y-down (@c false).
         * @note Default @c false keeps @c view{} an identity transform.
         */
        bool y_up = false;
    };

    // -------------------------------------------------------------------------
    // point  <->  world vec
    // -------------------------------------------------------------------------

    /**
     * @brief Convert a world-space point to a display-space @ref point (full view transform:
     *        scale, origin offset, and y-flip).
     * @param world The world-space point.
     * @param v     The active view.
     * @return The corresponding display-space point.
     */
    [[nodiscard]] inline point to_design(const collide::vec& world, const view& v) noexcept {
        const float dx = (world.x() - v.origin.x()) * v.pixels_per_unit;
        const float wy = world.y() - v.origin.y();
        const float dy = (v.y_up ? -wy : wy) * v.pixels_per_unit;
        return {dp{dx}, dp{dy}};
    }

    /**
     * @brief Convert a display-space @ref point to a world-space point (inverse of
     *        @ref to_design(const collide::vec&, const view&)).
     * @param design The display-space point.
     * @param v      The active view.
     * @return The corresponding world-space point.
     */
    [[nodiscard]] inline collide::vec to_world(const point& design, const view& v) noexcept {
        const float wx = design.x.design() / v.pixels_per_unit + v.origin.x();
        const float dy = design.y.design() / v.pixels_per_unit;
        const float wy = (v.y_up ? -dy : dy) + v.origin.y();
        return collide::vec{wx, wy};
    }

    // -------------------------------------------------------------------------
    // displacement/direction vector (translation-invariant)
    // -------------------------------------------------------------------------

    /**
     * @brief Convert a world-space displacement/direction vector to display space.
     *
     * Translation-invariant: the @c origin offset is NOT applied (only scale and y-flip), so a
     * delta/direction maps correctly rather than being shifted like a position.
     *
     * @param world_vec The world-space vector (a displacement or direction, not a position).
     * @param v         The active view.
     * @return The corresponding display-space vector.
     */
    [[nodiscard]] inline point to_design_vector(const collide::vec& world_vec, const view& v) noexcept {
        const float dx = world_vec.x() * v.pixels_per_unit;
        const float wy = world_vec.y();
        const float dy = (v.y_up ? -wy : wy) * v.pixels_per_unit;
        return {dp{dx}, dp{dy}};
    }

    /**
     * @brief Convert a display-space displacement/direction vector to world space (inverse of
     *        @ref to_design_vector).
     *
     * Translation-invariant: the @c origin offset is NOT applied (only scale and y-flip).
     *
     * @param design_vec The display-space vector (a displacement or direction, not a position).
     * @param v          The active view.
     * @return The corresponding world-space vector.
     */
    [[nodiscard]] inline collide::vec to_world_vector(const point& design_vec, const view& v) noexcept {
        const float wx = design_vec.x.design() / v.pixels_per_unit;
        const float dy = design_vec.y.design() / v.pixels_per_unit;
        const float wy = v.y_up ? -dy : dy;
        return collide::vec{wx, wy};
    }

    // -------------------------------------------------------------------------
    // rect  <->  aabb   (corners re-derived: a y-flip swaps top/bottom)
    // -------------------------------------------------------------------------

    /**
     * @brief Convert a display-space @ref rect to a world-space @ref collide::aabb.
     *
     * Both corners are transformed and the min/max are re-derived, because a y-flip swaps which
     * corner is top vs bottom -- the result is always a well-formed (min <= max) AABB.
     *
     * @param r The display-space rectangle.
     * @param v The active view.
     * @return The corresponding world-space axis-aligned box.
     */
    [[nodiscard]] inline collide::aabb to_world(const rect& r, const view& v) noexcept {
        const collide::vec a = to_world(r.top_left(), v);
        const collide::vec b = to_world(r.bottom_right(), v);
        return {
            collide::vec{std::min(a.x(), b.x()), std::min(a.y(), b.y())},
            collide::vec{std::max(a.x(), b.x()), std::max(a.y(), b.y())}
        };
    }

    /**
     * @brief Convert a world-space @ref collide::aabb to a display-space @ref rect (inverse of
     *        @ref to_world(const rect&, const view&)).
     *
     * Both corners are transformed and the min corner + positive width/height are re-derived, so a
     * y-flip that swaps top/bottom still yields a normalized rectangle.
     *
     * @param box The world-space axis-aligned box.
     * @param v   The active view.
     * @return The corresponding display-space rectangle.
     */
    [[nodiscard]] inline rect to_design(const collide::aabb& box, const view& v) noexcept {
        const point a = to_design(box.min, v);
        const point b = to_design(box.max, v);
        const dp x = a.x < b.x ? a.x : b.x;
        const dp y = a.y < b.y ? a.y : b.y;
        const dp w = (a.x < b.x ? b.x - a.x : a.x - b.x);
        const dp h = (a.y < b.y ? b.y - a.y : a.y - b.y);
        return {x, y, w, h};
    }

    // -------------------------------------------------------------------------
    // circle  <->  circle   (radius is a length: scale only, no offset/flip)
    // -------------------------------------------------------------------------

    /**
     * @brief Convert a display-space @ref circle to a world-space @ref collide::circle.
     *
     * The center goes through the full point transform; the radius is a length, so it is only
     * rescaled (no origin offset, no y-flip).
     *
     * @param c The display-space circle.
     * @param v The active view.
     * @return The corresponding world-space circle.
     */
    [[nodiscard]] inline collide::circle to_world(const circle& c, const view& v) noexcept {
        return {to_world(c.center(), v), c.radius.design() / v.pixels_per_unit};
    }

    /**
     * @brief Convert a world-space @ref collide::circle to a display-space @ref circle (inverse of
     *        @ref to_world(const circle&, const view&)).
     *
     * The center goes through the full point transform; the radius is only rescaled.
     *
     * @param c The world-space circle.
     * @param v The active view.
     * @return The corresponding display-space circle.
     */
    [[nodiscard]] inline circle to_design(const collide::circle& c, const view& v) noexcept {
        return {to_design(c.center, v), dp{c.radius * v.pixels_per_unit}};
    }

    // -------------------------------------------------------------------------
    // line  <->  segment
    // -------------------------------------------------------------------------

    /**
     * @brief Convert a display-space @ref line to a world-space @ref collide::segment (both
     *        endpoints through the full point transform).
     * @param l The display-space line.
     * @param v The active view.
     * @return The corresponding world-space segment.
     */
    [[nodiscard]] inline collide::segment to_world(const line& l, const view& v) noexcept {
        return {to_world(l.start(), v), to_world(l.end(), v)};
    }

    /**
     * @brief Convert a world-space @ref collide::segment to a display-space @ref line (inverse of
     *        @ref to_world(const line&, const view&)).
     * @param s The world-space segment.
     * @param v The active view.
     * @return The corresponding display-space line.
     */
    [[nodiscard]] inline line to_design(const collide::segment& s, const view& v) noexcept {
        return {to_design(s.from, v), to_design(s.to, v)};
    }
} // namespace simplex
