#pragma once

// =============================================================================
// Bridge: physics world space  <->  display space (dp).
//
// This is the ONE file that knows both coordinate systems. The collision core
// (<simplex/collide/shapes.hh>) stays in pure world-space floats; the geometry
// types (<simplex/geometry.hh>) stay in display dp. Everything that crosses
// between them goes through to_world() / to_design() here.
//
// The crossing is a real view transform, not a type relabel: world units are
// scaled to design pixels, the physics origin need not be the window's
// top-left, and physics is conventionally y-up while the screen is y-down.
//
// `view{}` is a true identity (no scale, no offset, no flip), so display space
// and world space coincide numerically — convenient for screen-space hit tests
// while a real camera does not yet exist. Flip y_up / set pixels_per_unit /
// move origin to introduce an actual physics world; nothing in shapes.hh or in
// callers' collision logic changes when you do.
// =============================================================================

#include <algorithm>

#include <simplex/dp.hh>
#include <simplex/geometry.hh>
#include <simplex/collide/shapes.hh>

namespace simplex {
    struct view {
        // World coordinate displayed at design-space (0, 0).
        collide::vec origin{};
        // World units -> design pixels. (Distinct from dp's display scale,
        // which maps design pixels -> physical pixels at render time.)
        float pixels_per_unit = 1.0f;
        // physics y-up (true) vs screen y-down (false). Default false keeps
        // view{} an identity transform.
        bool y_up = false;
    };

    // -------------------------------------------------------------------------
    // point  <->  world vec
    // -------------------------------------------------------------------------

    [[nodiscard]] inline point to_design(const collide::vec& world, const view& v) noexcept {
        const float dx = (world.x() - v.origin.x()) * v.pixels_per_unit;
        const float wy = world.y() - v.origin.y();
        const float dy = (v.y_up ? -wy : wy) * v.pixels_per_unit;
        return {dp{dx}, dp{dy}};
    }

    [[nodiscard]] inline collide::vec to_world(const point& design, const view& v) noexcept {
        const float wx = design.x.design() / v.pixels_per_unit + v.origin.x();
        const float dy = design.y.design() / v.pixels_per_unit;
        const float wy = (v.y_up ? -dy : dy) + v.origin.y();
        return collide::vec{wx, wy};
    }

    // -------------------------------------------------------------------------
    // rect  <->  aabb   (corners re-derived: a y-flip swaps top/bottom)
    // -------------------------------------------------------------------------

    [[nodiscard]] inline collide::aabb to_world(const rect& r, const view& v) noexcept {
        const collide::vec a = to_world(r.top_left(), v);
        const collide::vec b = to_world(r.bottom_right(), v);
        return {
            collide::vec{std::min(a.x(), b.x()), std::min(a.y(), b.y())},
            collide::vec{std::max(a.x(), b.x()), std::max(a.y(), b.y())}
        };
    }

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

    [[nodiscard]] inline collide::circle to_world(const circle& c, const view& v) noexcept {
        return {to_world(c.center(), v), c.radius.design() / v.pixels_per_unit};
    }

    [[nodiscard]] inline circle to_design(const collide::circle& c, const view& v) noexcept {
        return {to_design(c.center, v), dp{c.radius * v.pixels_per_unit}};
    }

    // -------------------------------------------------------------------------
    // line  <->  segment
    // -------------------------------------------------------------------------

    [[nodiscard]] inline collide::segment to_world(const line& l, const view& v) noexcept {
        return {to_world(l.start(), v), to_world(l.end(), v)};
    }

    [[nodiscard]] inline line to_design(const collide::segment& s, const view& v) noexcept {
        return {to_design(s.from, v), to_design(s.to, v)};
    }
} // namespace simplex
