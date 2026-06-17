#define EULER_DISABLE_ENFORCE
#include <simplex/effects.hh>
#include <cmath>
#include <algorithm>
#include <euler/euler.hh>

namespace simplex {

    // wave_horizontal_effect
    void wave_horizontal_effect::apply(sprite_mesh& mesh) {
        auto& verts = mesh.vertices();
        int cols = mesh.cols();
        int rows = mesh.rows();
        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(rows);
            float offset = std::sin(v * m_frequency * 2.0f * 3.14159265f + m_phase) * m_amplitude;
            for (int x = 0; x <= cols; ++x) {
                verts[idx].position.x += offset;
                idx++;
            }
        }
    }

    // wave_vertical_effect
    void wave_vertical_effect::apply(sprite_mesh& mesh) {
        auto& verts = mesh.vertices();
        int cols = mesh.cols();
        int rows = mesh.rows();

        m_scratch_offsets.resize(static_cast<std::size_t>(cols + 1));
        for (int x = 0; x <= cols; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(cols);
            m_scratch_offsets[static_cast<std::size_t>(x)] = std::sin(u * m_frequency * 2.0f * 3.14159265f + m_phase) * m_amplitude;
        }

        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            for (int x = 0; x <= cols; ++x) {
                verts[idx].position.y += m_scratch_offsets[static_cast<std::size_t>(x)];
                idx++;
            }
        }
    }

    // skew_horizontal_effect
    void skew_horizontal_effect::apply(sprite_mesh& mesh) {
        auto& verts = mesh.vertices();
        int cols = mesh.cols();
        int rows = mesh.rows();
        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(rows);
            float offset = (v - 0.5f) * m_skew_x * 2.0f;
            for (int x = 0; x <= cols; ++x) {
                verts[idx].position.x += offset;
                idx++;
            }
        }
    }

    // skew_vertical_effect
    void skew_vertical_effect::apply(sprite_mesh& mesh) {
        auto& verts = mesh.vertices();
        int cols = mesh.cols();
        int rows = mesh.rows();

        m_scratch_offsets.resize(static_cast<std::size_t>(cols + 1));
        for (int x = 0; x <= cols; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(cols);
            m_scratch_offsets[static_cast<std::size_t>(x)] = (u - 0.5f) * m_skew_y * 2.0f;
        }

        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            for (int x = 0; x <= cols; ++x) {
                verts[idx].position.y += m_scratch_offsets[static_cast<std::size_t>(x)];
                idx++;
            }
        }
    }

    // perspective_effect
    void perspective_effect::apply(sprite_mesh& mesh) {
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
            float taper = (1.0f - v) * m_top_taper + v * m_bottom_taper;

            for (int x = 0; x <= cols; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(cols);
                verts[idx].position.x = row_center + (u - 0.5f) * half_width * 2.0f * taper;
                idx++;
            }
        }
    }

    // pinch_punch_effect
    void pinch_punch_effect::apply(sprite_mesh& mesh) {
        if (m_radius <= 0.0f) return;

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
        float center_px = min_x + m_center_u * width;
        float center_py = min_y + m_center_v * height;

        int cols = mesh.cols();
        int rows = mesh.rows();
        std::size_t idx = 0;
        for (int y = 0; y <= rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(rows);
            float dv = v - m_center_v;
            for (int x = 0; x <= cols; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(cols);
                float du = u - m_center_u;
                float dist = std::sqrt(du * du + dv * dv);
                if (dist < m_radius) {
                    float influence = (1.0f - dist / m_radius);
                    float factor = 1.0f + m_amount * influence * influence;
                    verts[idx].position.x = center_px + (verts[idx].position.x - center_px) * factor;
                    verts[idx].position.y = center_py + (verts[idx].position.y - center_py) * factor;
                }
                idx++;
            }
        }
    }

    // rotate_3d_effect
    void rotate_3d_effect::apply(sprite_mesh& mesh) {
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

        float pivot_px = min_x + m_pivot_u * width;
        float pivot_py = min_y + m_pivot_v * height;
        float max_dim = std::max(width, height);
        if (max_dim <= 0.0f) max_dim = 1.0f;
        float camera_distance = m_perspective_factor * max_dim;

        auto m_x = euler::rotation_matrix3_x(m_pitch);
        auto m_y = euler::rotation_matrix3_y(m_yaw);
        auto m_z = euler::rotation_matrix3_z(m_roll);
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

} // namespace simplex
