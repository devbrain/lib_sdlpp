#include <simplex/effects.hh>
#include <cmath>
#include <algorithm>

namespace simplex::effects {
    void wave_horizontal(sprite_mesh& mesh, float time, float amplitude, float frequency) noexcept {
        mesh.transform_vertices([time, amplitude, frequency](int col, int row, float u, float v, float& px, float& py) {
            (void)col;
            (void)row;
            (void)u;
            (void)py;
            px += std::sin(v * frequency * 2.0f * 3.14159265f + time) * amplitude;
        });
    }

    void wave_vertical(sprite_mesh& mesh, float time, float amplitude, float frequency) noexcept {
        mesh.transform_vertices([time, amplitude, frequency](int col, int row, float u, float v, float& px, float& py) {
            (void)col;
            (void)row;
            (void)v;
            (void)px;
            py += std::sin(u * frequency * 2.0f * 3.14159265f + time) * amplitude;
        });
    }

    void skew_horizontal(sprite_mesh& mesh, float skew_x) noexcept {
        mesh.transform_vertices([skew_x](int col, int row, float u, float v, float& px, float& py) {
            (void)col;
            (void)row;
            (void)u;
            (void)py;
            px += (v - 0.5f) * skew_x * 2.0f;
        });
    }

    void skew_vertical(sprite_mesh& mesh, float skew_y) noexcept {
        mesh.transform_vertices([skew_y](int col, int row, float u, float v, float& px, float& py) {
            (void)col;
            (void)row;
            (void)v;
            (void)px;
            py += (u - 0.5f) * skew_y * 2.0f;
        });
    }

    void perspective(sprite_mesh& mesh, float top_taper, float bottom_taper) noexcept {
        int cols = mesh.cols();
        auto& verts = mesh.vertices();

        mesh.transform_vertices([&verts, cols, top_taper, bottom_taper](int col, int row, float u, float v, float& px, float& py) {
            (void)col;
            (void)py;
            float px_left = verts[static_cast<std::size_t>(row * (cols + 1))].position.x;
            float px_right = verts[static_cast<std::size_t>(row * (cols + 1) + cols)].position.x;
            float row_center = (px_left + px_right) * 0.5f;
            float half_width = (px_right - px_left) * 0.5f;

            float taper = (1.0f - v) * top_taper + v * bottom_taper;
            px = row_center + (u - 0.5f) * half_width * 2.0f * taper;
        });
    }

    void pinch_punch(sprite_mesh& mesh, float center_u, float center_v, float radius, float amount) noexcept {
        if (radius <= 0.0f) return;

        auto& verts = mesh.vertices();
        if (verts.empty()) return;

        float min_x = verts[0].position.x;
        float max_x = verts[0].position.x;
        float min_y = verts[0].position.y;
        float max_y = verts[0].position.y;

        for (const auto& vert : verts) {
            min_x = std::min(min_x, vert.position.x);
            max_x = std::max(max_x, vert.position.x);
            min_y = std::min(min_y, vert.position.y);
            max_y = std::max(max_y, vert.position.y);
        }

        float width = max_x - min_x;
        float height = max_y - min_y;
        float center_px = min_x + center_u * width;
        float center_py = min_y + center_v * height;

        mesh.transform_vertices([center_u, center_v, radius, amount, center_px, center_py](int col, int row, float u, float v, float& px, float& py) {
            (void)col;
            (void)row;
            float du = u - center_u;
            float dv = v - center_v;
            float dist = std::sqrt(du * du + dv * dv);
            if (dist < radius) {
                float influence = (1.0f - dist / radius);
                float factor = 1.0f + amount * influence * influence;
                px = center_px + (px - center_px) * factor;
                py = center_py + (py - center_py) * factor;
            }
        });
    }
} // namespace simplex::effects
