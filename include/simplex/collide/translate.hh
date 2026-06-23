//
// Created by igor on 23/06/2026.
//

#pragma once

#include <simplex/collide/types.hh>

namespace simplex::collide {
    [[nodiscard]] constexpr vec translate(const vec& x, const vec& v) {
        return vec{x.x() + v.x(), x.y() + v.y()};
    }

    [[nodiscard]] constexpr aabb translate(const aabb& x, const vec& v) {
        return {translate(x.min, v), translate(x.max, v) };
    }

    [[nodiscard]] constexpr circle translate(const circle& x, const vec& v) {
        return {translate(x.center, v), x.radius };
    }

    [[nodiscard]] constexpr segment translate(const segment& x, const vec& v) {
        return {translate(x.from, v), translate(x.to, v) };
    }

}