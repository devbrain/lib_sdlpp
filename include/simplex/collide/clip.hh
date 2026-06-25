//
// Created by igor on 25/06/2026.
//

#pragma once

#include <optional>
#include <algorithm>
#include <simplex/collide/sweep.hh>

namespace simplex::collide {
    inline std::optional <segment> clip(const aabb& box, const segment& s) {
        // Liang-Barsky
        const auto hit = intersect_param(box, s);
        if (!hit || !hit->segment_overlaps()) {
            return std::nullopt;
        }

        const float t0 = std::clamp(hit->entry_param, 0.0f, 1.0f);
        const float t1 = std::clamp(hit->exit_param, 0.0f, 1.0f);

        return segment{
            s.point_in_time(t0),
            s.point_in_time(t1),
        };
    }
}
