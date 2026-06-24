//
// Created by igor on 24/06/2026.
//

#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
#include <limits>

#include <simplex/collide/shapes.hh>
#include <failsafe/enforce.hh>

namespace simplex::collide {
    namespace detail {
        struct grid_coord {
            static constexpr auto INVALID = std::numeric_limits <uint32_t>::max();
            uint32_t x;
            uint32_t y;

            grid_coord()
                : x(INVALID), y(INVALID) {
            }

            grid_coord(uint32_t x_, uint32_t y_)
                : x(x_), y(y_) {
            }

            explicit operator bool() const noexcept {
                return !(x == INVALID || y == INVALID);
            }
        };

        template<typename T>
        class grid_storage {
            public:
                grid_storage(uint32_t w, uint32_t h)
                    : m_grid_with(w),
                      m_grid_height(h) {
                    m_grid.resize(m_grid_with * m_grid_height);
                }

                T& operator[](const grid_coord& p) {
                    return m_grid[p.y * m_grid_with + p.x];
                }

                const T& operator[](const grid_coord& p) const {
                    return m_grid[p.y * m_grid_with + p.x];
                }

                [[nodiscard]] uint32_t get_width() const noexcept {
                    return m_grid_with;
                }

                [[nodiscard]] uint32_t get_height() const noexcept {
                    return m_grid_height;
                }

            private:
                std::vector <T> m_grid;
                uint32_t m_grid_with;
                uint32_t m_grid_height;
        };
    }

    // Test-only access to the grid's internal mapping. Defined only in the test TU.
    struct grid_test_access;

    template<typename T>
    class grid {
        public:
            friend struct grid_test_access;

            grid(uint32_t w, uint32_t h, const vec& grid_min, const vec& grid_max)
                : m_grid(w, h),
                  m_physical_bounds(grid_min, grid_max),
                  m_dx(grid_max.x() - grid_min.x()),
                  m_dy(grid_max.y() - grid_min.y()),
                  m_cell_dim{
                      m_dx / static_cast <float>(w),
                      m_dy / static_cast <float>(h)
                  } {
            }

        private:
            detail::grid_coord physical_to_grid(const vec& v) {
                if (!contains(m_physical_bounds, v)) {
                    return {};
                }
                const vec p = (v - m_physical_bounds.min) / m_cell_dim;
                const auto cx = std::min(static_cast <uint32_t>(p.x()), m_grid.get_width() - 1u);
                const auto cy = std::min(static_cast <uint32_t>(p.y()), m_grid.get_height() - 1u);
                return {cx, cy};
            }

        private:
            detail::grid_storage <T> m_grid;
            aabb m_physical_bounds;
            const float m_dx;
            const float m_dy;
            const vec m_cell_dim;
    };
};
