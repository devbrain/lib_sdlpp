//
// Created by igor on 27/05/2026.
//

#include <simplex/dp.hh>

namespace simplex::detail {
    float& current_scale() {
        static float s = 1.0f;
        return s;
    }
}