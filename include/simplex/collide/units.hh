//
// Created by igor on 23/06/2026.
//
// Minimal strong types for the physical quantities the collision world's inner math juggles,
// so the compiler rejects unit confusions (the class of bug where a time-of-impact fraction
// is multiplied in where a duration belongs, or a velocity is used as a displacement).
//
// Deliberately MINIMAL -- only the four quantities that actually interact in the bug-prone
// arithmetic, with only the legal combinations defined:
//
//   velocity     * duration  = displacement     (v * dt)
//   displacement * fraction  = displacement     (delta * toi)
//   displacement +/- displacement = displacement
//
// Positions, directions/normals, and scalar distances/speeds are intentionally left as plain
// vec/float -- typing them too would balloon into an ~8-type algebra for little extra safety
// (those were never the bug sites). The public world API also stays vec/float; these types are
// used inside the engine, converting at the boundary.
//
// They live in a nested `units` namespace because `velocity` would otherwise clash with the
// `vec velocity;` member on the body records.
//
// NB: all results are materialised component-wise into vec (not via euler's expression
// templates), so a strong type never holds a lazy expression that could dangle.
//
#pragma once

#include <simplex/collide/types.hh>

namespace simplex::collide::units {
    // A time span: a frame's dt, or a swept-query window. Unit-agnostic on purpose -- the
    // engine never commits to seconds; dt is whatever the caller integrates in.
    struct duration {
        float value{0.0f};
    };

    // A dimensionless parameter, typically in [0, 1]: a time-of-impact or interpolation t.
    struct fraction {
        float value{0.0f};
    };

    // A rate: world units per unit duration.
    struct velocity {
        vec value{};
    };

    // A world-space offset (delta / remaining / leftover). A position stays a plain vec.
    struct displacement {
        vec value{};
    };

    // ---- dimensional algebra: only the legal combinations exist --------------------

    // velocity * duration = displacement   (integrate a rate over a time span)
    [[nodiscard]] inline displacement operator*(velocity v, duration d) {
        return displacement{vec{v.value.x() * d.value, v.value.y() * d.value}};
    }
    [[nodiscard]] inline displacement operator*(duration d, velocity v) { return v * d; }

    // displacement * fraction = displacement   (a sub-span of a move, e.g. up to the TOI)
    [[nodiscard]] inline displacement operator*(displacement s, fraction f) {
        return displacement{vec{s.value.x() * f.value, s.value.y() * f.value}};
    }
    [[nodiscard]] inline displacement operator*(fraction f, displacement s) { return s * f; }

    // displacement +/- displacement = displacement
    [[nodiscard]] inline displacement operator+(displacement a, displacement b) {
        return displacement{vec{a.value.x() + b.value.x(), a.value.y() + b.value.y()}};
    }
    [[nodiscard]] inline displacement operator-(displacement a, displacement b) {
        return displacement{vec{a.value.x() - b.value.x(), a.value.y() - b.value.y()}};
    }
}
