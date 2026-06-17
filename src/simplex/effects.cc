#define EULER_DISABLE_ENFORCE
#include <simplex/effects.hh>
#include <cmath>
#include <algorithm>
#include <euler/euler.hh>

namespace simplex::effects {
    void wave_horizontal(sprite_mesh& mesh, float time, float amplitude, float frequency) noexcept {
        auto& verts = mesh.vertices();
        int cols = mesh.cols();
        int rows = mesh.rows();
        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(rows);
            float offset = std::sin(v * frequency * 2.0f * 3.14159265f + time) * amplitude;
            for (int x = 0; x <= cols; ++x) {
                verts[idx].position.x += offset;
                idx++;
            }
        }
    }

    void wave_vertical(sprite_mesh& mesh, float time, float amplitude, float frequency) noexcept {
        auto& verts = mesh.vertices();
        int cols = mesh.cols();
        int rows = mesh.rows();
        std::vector<float> offsets(static_cast<std::size_t>(cols + 1));
        for (int x = 0; x <= cols; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(cols);
            offsets[static_cast<std::size_t>(x)] = std::sin(u * frequency * 2.0f * 3.14159265f + time) * amplitude;
        }

        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            for (int x = 0; x <= cols; ++x) {
                verts[idx].position.y += offsets[static_cast<std::size_t>(x)];
                idx++;
            }
        }
    }

    void skew_horizontal(sprite_mesh& mesh, float skew_x) noexcept {
        auto& verts = mesh.vertices();
        int cols = mesh.cols();
        int rows = mesh.rows();
        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(rows);
            float offset = (v - 0.5f) * skew_x * 2.0f;
            for (int x = 0; x <= cols; ++x) {
                verts[idx].position.x += offset;
                idx++;
            }
        }
    }

    void skew_vertical(sprite_mesh& mesh, float skew_y) noexcept {
        auto& verts = mesh.vertices();
        int cols = mesh.cols();
        int rows = mesh.rows();
        std::vector<float> offsets(static_cast<std::size_t>(cols + 1));
        for (int x = 0; x <= cols; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(cols);
            offsets[static_cast<std::size_t>(x)] = (u - 0.5f) * skew_y * 2.0f;
        }

        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            for (int x = 0; x <= cols; ++x) {
                verts[idx].position.y += offsets[static_cast<std::size_t>(x)];
                idx++;
            }
        }
    }

    void perspective(sprite_mesh& mesh, float top_taper, float bottom_taper) noexcept {
        int cols = mesh.cols();
        int rows = mesh.rows();
        auto& verts = mesh.vertices();
        if (verts.empty()) return;

        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(rows);
            float px_left = verts[static_cast<std::size_t>(y * (cols + 1))].position.x;
            float px_right = verts[static_cast<std::size_t>(y * (cols + 1) + cols)].position.x;
            float row_center = (px_left + px_right) * 0.5f;
            float half_width = (px_right - px_left) * 0.5f;
            float taper = (1.0f - v) * top_taper + v * bottom_taper;

            for (int x = 0; x <= cols; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(cols);
                verts[idx].position.x = row_center + (u - 0.5f) * half_width * 2.0f * taper;
                idx++;
            }
        }
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

        int cols = mesh.cols();
        int rows = mesh.rows();
        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(rows);
            float dv = v - center_v;
            for (int x = 0; x <= cols; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(cols);
                float du = u - center_u;
                float dist = std::sqrt(du * du + dv * dv);
                if (dist < radius) {
                    float influence = (1.0f - dist / radius);
                    float factor = 1.0f + amount * influence * influence;
                    verts[idx].position.x = center_px + (verts[idx].position.x - center_px) * factor;
                    verts[idx].position.y = center_py + (verts[idx].position.y - center_py) * factor;
                }
                idx++;
            }
        }
    }

    void rotate_3d(sprite_mesh& mesh, euler::radianf pitch, euler::radianf yaw, euler::radianf roll, float pivot_u, float pivot_v, float perspective_factor) noexcept {
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

        float pivot_px = min_x + pivot_u * width;
        float pivot_py = min_y + pivot_v * height;
        float max_dim = std::max(width, height);
        if (max_dim <= 0.0f) max_dim = 1.0f;
        float camera_distance = perspective_factor * max_dim;

        auto m_x = euler::rotation_matrix3_x(pitch);
        auto m_y = euler::rotation_matrix3_y(yaw);
        auto m_z = euler::rotation_matrix3_z(roll);
        auto rot_matrix = m_x * m_y * m_z;

        for (auto& vert : verts) {
            float local_x = vert.position.x - pivot_px;
            float local_y = vert.position.y - pivot_py;
            float local_z = 0.0f;

            euler::vector<float, 4> local_pos{local_x, local_y, local_z, 1.0f};
            euler::vector<float, 4> rotated_pos = rot_matrix * local_pos;

            float rx = rotated_pos[0];
            float ry = rotated_pos[1];
            float rz = rotated_pos[2];

            float denom = 1.0f - (rz / camera_distance);
            if (denom < 0.01f) {
                denom = 0.01f;
            }

            vert.position.x = pivot_px + rx / denom;
            vert.position.y = pivot_py + ry / denom;
        }
    }
} // namespace simplex::effects
