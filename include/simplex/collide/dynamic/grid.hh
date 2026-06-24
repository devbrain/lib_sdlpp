//
// Created by igor on 24/06/2026.
//

#pragma once

#include <algorithm>
#include <utility>
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
                    m_coords.resize(m_grid_with * m_grid_height, grid_coord::INVALID);
                }

                template <typename ... Args>
                void set(const grid_coord& c, Args&&...args) {
                    ENFORCE(is_valid(c));
                    if (const auto idx = index(c); m_coords[idx] == grid_coord::INVALID) {
                        if (m_free_list.empty()) {
                            m_grid.emplace_back(std::forward<Args>(args)...);
                            m_coords[idx] = static_cast<uint32_t>(m_grid.size() - 1);
                        } else {
                            const auto pos = m_free_list.back();
                            m_free_list.pop_back();
                            m_grid[pos] = T(std::forward<Args>(args)...);
                            m_coords[idx] = pos;
                        }
                    } else {
                        m_grid[m_coords[idx]] = T(std::forward<Args>(args)...);
                    }
                }

                void clear(const grid_coord& c) {
                    if (!is_valid(c)) {
                        return;
                    }
                    const auto idx = index(c);
                    if (m_coords[idx] != grid_coord::INVALID) {
                        m_free_list.push_back(m_coords[idx]);
                        m_coords[idx] = grid_coord::INVALID;
                    }
                }

                void reset() {
                    std::fill(m_coords.begin(), m_coords.end(), grid_coord::INVALID);
                    m_free_list.clear();
                    m_grid.clear();
                }

                [[nodiscard]] const T* get(const grid_coord& c) const noexcept {
                    if (!is_valid(c)) {
                        return nullptr;
                    }
                    const auto idx = index(c);
                    if (m_coords[idx] == grid_coord::INVALID) {
                        return nullptr;
                    }
                    return &m_grid[m_coords[idx]];
                }

                [[nodiscard]] T* get(const grid_coord& c) noexcept {
                    if (!is_valid(c)) {
                        return nullptr;
                    }
                    const auto idx = index(c);
                    if (m_coords[idx] == grid_coord::INVALID) {
                        return nullptr;
                    }
                    return &m_grid[m_coords[idx]];
                }

                [[nodiscard]] bool is_valid(const grid_coord& p) const noexcept {
                    if (!p) {
                        return false;
                    }
                    return (p.x < m_grid_with) && (p.y < m_grid_height);
                }

                [[nodiscard]] uint32_t get_width() const noexcept {
                    return m_grid_with;
                }

                [[nodiscard]] uint32_t get_height() const noexcept {
                    return m_grid_height;
                }
            private:
                [[nodiscard]] uint32_t index(const grid_coord& p) const noexcept {
                    return p.y * m_grid_with + p.x;
                }
            private:
                std::vector <T> m_grid;
                std::vector<uint32_t> m_coords;
                std::vector<uint32_t> m_free_list;

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
                  m_cell_dim{
                      (grid_max.x() - grid_min.x()) / static_cast <float>(w),
                      (grid_max.y() - grid_min.y()) / static_cast <float>(h)
                  } {
            }

            template <typename ... Args>
            void set(const vec& v, Args&&... args) {
                m_grid.set(physical_to_grid(v), std::forward<Args>(args)...);
            }

            const T* get(const vec& v) const {
                return m_grid.get(physical_to_grid(v));
            }

            T* get(const vec& v) {
                return m_grid.get(physical_to_grid(v));
            }

            void clear(const vec& v) {
                m_grid.clear(physical_to_grid(v));
            }

            void reset() {
                m_grid.reset();
            }

            template <typename Fn>
            void query(const aabb& region, Fn&& callback) const {
                if (!intersects(region, m_physical_bounds)) {
                    return;
                }
                const auto min_x = std::max(m_physical_bounds.min.x(), region.min.x());
                const auto max_x = std::min(m_physical_bounds.max.x(), region.max.x());

                const auto min_y = std::max(m_physical_bounds.min.y(), region.min.y());
                const auto max_y = std::min(m_physical_bounds.max.y(), region.max.y());

                const auto min_c = physical_to_grid(vec{min_x, min_y});
                const auto max_c = physical_to_grid(vec{max_x, max_y});


                for (uint32_t y = min_c.y; y <= max_c.y; ++y) {
                    for (uint32_t x = min_c.x; x <= max_c.x; ++x) {
                        detail::grid_coord c(detail::grid_coord{x, y});
                        if (const auto* e = m_grid.get(c)) {
                            callback(*e, cell_box(c));
                        }
                    }
                }
            }

        private:
            [[nodiscard]] detail::grid_coord physical_to_grid(const vec& v) const {
                if (!contains(m_physical_bounds, v)) {
                    return {};
                }
                const vec p = (v - m_physical_bounds.min) / m_cell_dim;
                const auto cx = std::min(static_cast <uint32_t>(p.x()), m_grid.get_width() - 1u);
                const auto cy = std::min(static_cast <uint32_t>(p.y()), m_grid.get_height() - 1u);
                return {cx, cy};
            }

            [[nodiscard]] aabb cell_box(const detail::grid_coord& c) const {
                ENFORCE(c);
                const vec p{
                    static_cast <float>(c.x) * m_cell_dim.x(),
                    static_cast <float>(c.y) * m_cell_dim.y()
                };
                const vec corner = p + m_physical_bounds.min;
                return {corner, corner + m_cell_dim};
            }

        private:
            detail::grid_storage <T> m_grid;
            aabb m_physical_bounds;
            const vec m_cell_dim;
    };
};
