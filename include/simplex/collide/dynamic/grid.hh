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
#include <type_traits>

#include <simplex/collide/shapes.hh>
#include <simplex/collide/clip.hh>
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

                template<typename... Args>
                void set(const grid_coord& c, Args&&... args) {
                    ENFORCE(is_valid(c));
                    if (const auto idx = index(c); m_coords[idx] == grid_coord::INVALID) {
                        if (m_free_list.empty()) {
                            m_grid.emplace_back(std::forward <Args>(args)...);
                            m_coords[idx] = static_cast <uint32_t>(m_grid.size() - 1);
                        } else {
                            const auto pos = m_free_list.back();
                            m_free_list.pop_back();
                            m_grid[pos] = T(std::forward <Args>(args)...);
                            m_coords[idx] = pos;
                        }
                    } else {
                        m_grid[m_coords[idx]] = T(std::forward <Args>(args)...);
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
                std::vector <uint32_t> m_coords;
                std::vector <uint32_t> m_free_list;

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

            // Tilemap-friendly construction: an origin, a cell size, and a cell count. The physical
            // extent is DERIVED (max = origin + count * tile_size), so the cell size is exactly
            // tile_size -- there is no separate, possibly-mismatched physical box to specify.
            [[nodiscard]] static grid from_tile_size(const vec& origin, const vec& tile_size,
                                                     uint32_t cols, uint32_t rows) {
                const vec grid_max{
                    origin.x() + static_cast <float>(cols) * tile_size.x(),
                    origin.y() + static_cast <float>(rows) * tile_size.y()
                };
                return grid(cols, rows, origin, grid_max);
            }

            template<typename... Args>
            void set(const vec& v, Args&&... args) {
                m_grid.set(physical_to_grid(v), std::forward <Args>(args)...);
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

            template<typename Fn>
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
                        const detail::grid_coord c{x, y};
                        if (const auto* e = m_grid.get(c)) {
                            // Same void-or-bool callback contract as raycast: a void callback
                            // visits every occupied cell; a bool callback returns false to stop.
                            if constexpr (std::is_void_v <std::invoke_result_t <Fn&, const T&, const aabb&>>) {
                                callback(*e, cell_box(c));
                            } else {
                                if (!callback(*e, cell_box(c))) {
                                    return;
                                }
                            }
                        }
                    }
                }
            }

            // Swept broadphase for a MOVING shape: enumerate the occupied cells the shape can
            // overlap as its bound travels `start_bound` -> `start_bound + delta` over a frame.
            // The swept band is the cell rectangle of the union of the bound at both ends, which
            // covers the whole path (anti-tunnelling at the cell level for per-frame moves). Pure
            // broadphase -- the world narrow-phases (swept) each cell and keeps the earliest TOI.
            // Same callback contract as query: (const T&, cell_box), void-or-bool early-out.
            // `start_bound` is the shape's enclosing aabb at the start (enclose(shape)); a circle
            // sweeps the same way via its bounding box.
            template<typename Fn>
            void swept(const aabb& start_bound, const vec& delta, Fn&& callback) const {
                const aabb band = aabb::combine(start_bound, translate(start_bound, delta));
                query(band, std::forward<Fn>(callback));
            }

            // Amanatides-Woo DDA: visit the cells the segment from->to crosses, in near-to-far
            // order, calling callback(const T&, cell_box, t) for each OCCUPIED cell. `t` is the
            // entry parameter along the ORIGINAL from->to ray (in [0,1]), so it is comparable with
            // the BVH raycast for a grid+BVH merge. The callback may return void (visit every
            // crossed occupied cell) or bool (return false to stop early -- the world's first
            // confirmed narrow-phase hit). Handles non-unit / rectangular cells and axis-aligned
            // rays; an out-of-grid ray is a tolerant no-op (clipped away).
            template<typename Fn>
            void raycast(const vec& from, const vec& to, Fn&& callback) const {
                const auto clipped = clip(m_physical_bounds, segment{from, to});
                if (!clipped) {
                    return; // the ray misses the grid entirely
                }
                const vec cf = clipped->from;
                const vec ct = clipped->to;

                const float dirx = to.x() - from.x();
                const float diry = to.y() - from.y();
                const float len2 = dirx * dirx + diry * diry;
                if (len2 < constants::POINT_EPS * constants::POINT_EPS) {
                    visit_cell(physical_to_grid(cf), 0.0f, callback); // degenerate: use the clipped point
                    return;
                }

                // entry/exit parameters of the clipped portion, measured along the ORIGINAL ray.
                const float t0 = ((cf.x() - from.x()) * dirx + (cf.y() - from.y()) * diry) / len2;
                const float t1 = ((ct.x() - from.x()) * dirx + (ct.y() - from.y()) * diry) / len2;

                const detail::grid_coord start = physical_to_grid(cf);
                int cx = static_cast <int>(start.x);
                int cy = static_cast <int>(start.y);

                const float eps = constants::POINT_EPS; // world-distance tol (direction != 0 test)
                const float inf = std::numeric_limits <float>::infinity();
                // tMax / t1 live in NORMALIZED ray-parameter space (t along from->to), so a world
                // tolerance must be converted: a world distance POINT_EPS spans POINT_EPS/|ray| in
                // t. Using a world eps here would merge far-apart crossings on a long ray into a
                // false "corner" -- emitting cells the ray never touches.
                const float t_eps = constants::POINT_EPS / std::sqrt(len2);

                // Directional start on an exact internal boundary. floor (physical_to_grid) maps a
                // boundary to the upper/right cell, but a ray moving in the negative direction
                // enters the lower/left cell -- pick the cell with positive-length forward overlap,
                // so a wall BEHIND the origin (opposite travel) is never emitted as a spurious hit.
                // (dir == 0 keeps the floored cell; the shadow row/col below covers both sides.)
                if (dirx < -eps) {
                    const float fx = (cf.x() - m_physical_bounds.min.x()) / m_cell_dim.x();
                    const float kx = std::round(fx);
                    if (std::abs(fx - kx) < constants::POINT_EPS / m_cell_dim.x() && kx >= 1.0f
                        && kx <= static_cast <float>(m_grid.get_width() - 1)) {
                        cx = static_cast <int>(kx) - 1;
                    }
                }
                if (diry < -eps) {
                    const float fy = (cf.y() - m_physical_bounds.min.y()) / m_cell_dim.y();
                    const float ky = std::round(fy);
                    if (std::abs(fy - ky) < constants::POINT_EPS / m_cell_dim.y() && ky >= 1.0f
                        && ky <= static_cast <float>(m_grid.get_height() - 1)) {
                        cy = static_cast <int>(ky) - 1;
                    }
                }

                // Per axis: step direction, t to cross one cell (tDelta), and t of the first cell
                // boundary ahead (tMax). dir==0 -> never step that axis (handles axis-aligned rays).
                int stepX = 0, stepY = 0;
                float tDeltaX = inf, tDeltaY = inf, tMaxX = inf, tMaxY = inf;
                if (std::abs(dirx) > eps) {
                    stepX = dirx > 0 ? 1 : -1;
                    tDeltaX = m_cell_dim.x() / std::abs(dirx);
                    const float bx = m_physical_bounds.min.x()
                                     + (static_cast <float>(cx) + (stepX > 0 ? 1.0f : 0.0f)) * m_cell_dim.x();
                    tMaxX = (bx - from.x()) / dirx;
                }
                if (std::abs(diry) > eps) {
                    stepY = diry > 0 ? 1 : -1;
                    tDeltaY = m_cell_dim.y() / std::abs(diry);
                    const float by = m_physical_bounds.min.y()
                                     + (static_cast <float>(cy) + (stepY > 0 ? 1.0f : 0.0f)) * m_cell_dim.y();
                    tMaxY = (by - from.y()) / diry;
                }

                // Supercover for the degenerate "on a grid line" case: an axis-aligned ray lying
                // EXACTLY on an internal grid line touches the cells on BOTH sides along its whole
                // length, but the floor mapping only assigns one side. Track the other ("shadow")
                // row/column so it is visited too. (Only possible for an axis-aligned ray.)
                int shadowX = -1, shadowY = -1;
                if (stepX == 0) {
                    const float fx = (from.x() - m_physical_bounds.min.x()) / m_cell_dim.x();
                    // fx is in CELL units; scale the world tolerance into cell space.
                    if (const float kx = std::round(fx);
                        std::abs(fx - kx) < constants::POINT_EPS / m_cell_dim.x() && kx >= 1.0f
                        && kx <= static_cast <float>(m_grid.get_width() - 1)) {
                        shadowX = static_cast <int>(kx) - 1;
                    }
                }
                if (stepY == 0) {
                    const float fy = (from.y() - m_physical_bounds.min.y()) / m_cell_dim.y();
                    // fy is in CELL units; scale the world tolerance into cell space.
                    if (const float ky = std::round(fy);
                        std::abs(fy - ky) < constants::POINT_EPS / m_cell_dim.y() && ky >= 1.0f
                        && ky <= static_cast <float>(m_grid.get_height() - 1)) {
                        shadowY = static_cast <int>(ky) - 1;
                    }
                }

                // Visit cell (x,y) and, for an on-gridline ray, its shadow neighbour. Out-of-range
                // cells are skipped (visit_cell is tolerant). Returns false if a bool callback stops.
                const auto emit = [&](int x, int y, float t) -> bool {
                    if (x >= 0 && y >= 0
                        && !visit_cell(detail::grid_coord{static_cast <uint32_t>(x), static_cast <uint32_t>(y)}, t,
                                       callback)) {
                        return false;
                    }
                    if (shadowY >= 0 && x >= 0
                        && !visit_cell(detail::grid_coord{static_cast <uint32_t>(x), static_cast <uint32_t>(shadowY)},
                                       t, callback)) {
                        return false;
                    }
                    if (shadowX >= 0 && y >= 0
                        && !visit_cell(detail::grid_coord{static_cast <uint32_t>(shadowX), static_cast <uint32_t>(y)},
                                       t, callback)) {
                        return false;
                    }
                    return true;
                };

                // Visit the start cell (entry t = t0), then step boundary-to-boundary.
                if (!emit(cx, cy, std::max(0.0f, t0))) {
                    return;
                }
                while (true) {
                    if (stepX != 0 && stepY != 0 && std::abs(tMaxX - tMaxY) <= t_eps) {
                        // Exact cell-CORNER crossing: the ray touches BOTH side-adjacent cells, not
                        // just the diagonal one -- visit both (supercover, no corner tunnelling).
                        const float t_cross = tMaxX; // == tMaxY
                        if (t_cross > t1 + t_eps) {
                            break;
                        }
                        if (!emit(cx + stepX, cy, t_cross)) return;
                        if (!emit(cx, cy + stepY, t_cross)) return;
                        cx += stepX;
                        cy += stepY;
                        tMaxX += tDeltaX;
                        tMaxY += tDeltaY;
                        if (cx < 0 || cy < 0) break;
                        if (!emit(cx, cy, t_cross)) return;
                    } else {
                        float t_cross;
                        if (tMaxX < tMaxY) {
                            cx += stepX;
                            t_cross = tMaxX;
                            tMaxX += tDeltaX;
                        } else {
                            cy += stepY;
                            t_cross = tMaxY;
                            tMaxY += tDeltaY;
                        }
                        if (t_cross > t1 + t_eps || cx < 0 || cy < 0) {
                            break; // past the clipped exit / stepped off the grid
                        }
                        if (!emit(cx, cy, t_cross)) return;
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

            // Report cell `c` at entry parameter `t` to the raycast callback. Empty / out-of-range
            // cells (get -> nullptr) are skipped. Returns false only when a bool-returning callback
            // asks to stop; a void callback always continues.
            template<typename Fn>
            bool visit_cell(const detail::grid_coord& c, float t, Fn&& callback) const {
                const T* e = m_grid.get(c); // tolerant: nullptr if empty or out of range
                if (!e) {
                    return true;
                }
                if constexpr (std::is_void_v <std::invoke_result_t <Fn&, const T&, const aabb&, float>>) {
                    callback(*e, cell_box(c), t);
                    return true;
                } else {
                    return callback(*e, cell_box(c), t); // bool callback: false stops the walk
                }
            }

        private:
            detail::grid_storage <T> m_grid;
            aabb m_physical_bounds;
            const vec m_cell_dim;
    };
};
