#include <simplex/mesh.hh>
#include <algorithm>

namespace simplex {
    sprite_mesh::sprite_mesh(int cols, int rows)
        : m_cols(std::max(1, cols)), m_rows(std::max(1, rows)) {
        m_vertices.resize(static_cast<std::size_t>((m_cols + 1) * (m_rows + 1)));
        generate_mesh_indices();
    }

    void sprite_mesh::layout_rect_impl(float dx, float dy, float dw, float dh, float sx, float sy, float sw, float sh) {
        m_vertices.resize(static_cast<std::size_t>((m_cols + 1) * (m_rows + 1)));

        for (int y = 0; y <= m_rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(m_rows);
            for (int x = 0; x <= m_cols; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(m_cols);

                float px = dx + u * dw;
                float py = dy + v * dh;

                float tx = sx + u * sw;
                float ty = sy + v * sh;

                std::size_t idx = static_cast<std::size_t>(y * (m_cols + 1) + x);
                m_vertices[idx] = {
                    .position = { px, py },
                    .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                    .tex_coord = { tx, ty }
                };
            }
        }
    }

    void sprite_mesh::transform_vertices(std::function<void(int col, int row, float u, float v, float& px, float& py)> callback) {
        if (!callback) return;

        for (int y = 0; y <= m_rows; ++y) {
            float v = static_cast<float>(y) / static_cast<float>(m_rows);
            for (int x = 0; x <= m_cols; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(m_cols);
                std::size_t idx = static_cast<std::size_t>(y * (m_cols + 1) + x);

                float px = m_vertices[idx].position.x;
                float py = m_vertices[idx].position.y;

                callback(x, y, u, v, px, py);

                m_vertices[idx].position.x = px;
                m_vertices[idx].position.y = py;
            }
        }
    }

    sdlpp::expected<void, std::string> sprite_mesh::render(sdlpp::renderer& r, const sdlpp::texture& tex) const {
        return r.render_geometry(tex.get(), m_vertices, m_indices);
    }

    void sprite_mesh::generate_mesh_indices() {
        m_indices.clear();
        m_indices.reserve(static_cast<std::size_t>(m_cols * m_rows * 6));

        for (int y = 0; y < m_rows; ++y) {
            for (int x = 0; x < m_cols; ++x) {
                int tl = y * (m_cols + 1) + x;
                int tr = tl + 1;
                int bl = (y + 1) * (m_cols + 1) + x;
                int br = bl + 1;

                // Triangle 1
                m_indices.push_back(tl);
                m_indices.push_back(tr);
                m_indices.push_back(bl);

                // Triangle 2
                m_indices.push_back(tr);
                m_indices.push_back(br);
                m_indices.push_back(bl);
            }
        }
    }
} // namespace simplex
