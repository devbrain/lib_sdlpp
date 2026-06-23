//
// Created by igor on 22/06/2026.
//

#pragma once

#include <algorithm>
#include <simplex/collide/types.hh>

namespace simplex::collide {
    constexpr
    aabb enclose(const segment& s) {
        auto min_x = std::min(s.from.x(), s.to.x());
        auto min_y = std::min(s.from.y(), s.to.y());

        auto max_x = std::max(s.from.x(), s.to.x());
        auto max_y = std::max(s.from.y(), s.to.y());

        return {{min_x, min_y}, {max_x, max_y}};
    }

    constexpr
    aabb enclose(const aabb& b) {
        return b;
    }

    constexpr
    aabb enclose(const circle& c) {
        auto min_x = c.center.x() - c.radius;
        auto min_y = c.center.y() - c.radius;
        auto max_x = c.center.x() + c.radius;
        auto max_y = c.center.y() + c.radius;

        return {{min_x, min_y}, {max_x, max_y}};
    }
}

