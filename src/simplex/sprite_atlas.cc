#include <simplex/sprite_atlas.hh>
#include <sdlpp/image/image.hh>
#include <algorithm>

namespace simplex {
    // =========================================================================
    // bitmask Implementation
    // =========================================================================

    bitmask::bitmask(int width, int height)
        : m_width(width), m_height(height), m_bits(static_cast<std::size_t>(width * height), false) {
    }

    void bitmask::set(int x, int y, bool val) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_bits[static_cast<std::size_t>(y * m_width + x)] = val;
        }
    }

    bool bitmask::get(int x, int y) const noexcept {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return false;
        }
        return m_bits[static_cast<std::size_t>(y * m_width + x)];
    }

    bool bitmask::overlaps(const bitmask& other, int dx, int dy) const noexcept {
        int x1 = std::max(0, dx);
        int y1 = std::max(0, dy);
        int x2 = std::min(m_width, other.m_width + dx);
        int y2 = std::min(m_height, other.m_height + dy);

        if (x1 >= x2 || y1 >= y2) {
            return false;
        }

        for (int y = y1; y < y2; ++y) {
            for (int x = x1; x < x2; ++x) {
                if (get(x, y) && other.get(x - dx, y - dy)) {
                    return true;
                }
            }
        }
        return false;
    }

    // =========================================================================
    // sprite_atlas Implementation
    // =========================================================================

    sprite_atlas::sprite_atlas(sdlpp::texture tex, int frame_width, int frame_height)
        : m_texture(std::move(tex)) {
        slice_grid(frame_width, frame_height);
    }

    sprite_atlas::sprite_atlas(sdlpp::texture tex, std::vector<sdlpp::rect<int>> frames)
        : m_texture(std::move(tex)), m_frames(std::move(frames)) {
    }

    sprite_atlas::sprite_atlas(
        sdlpp::renderer& r,
        const sdlpp::surface& surf,
        int frame_width,
        int frame_height,
        bool generate_masks) {
        if (surf) {
            // Slicing
            int tex_w = static_cast<int>(surf.width());
            int tex_h = static_cast<int>(surf.height());
            int cols = tex_w / frame_width;
            int rows = tex_h / frame_height;

            m_frames.reserve(static_cast<std::size_t>(rows * cols));
            for (int row = 0; row < rows; ++row) {
                for (int col = 0; col < cols; ++col) {
                    m_frames.emplace_back(
                        col * frame_width,
                        row * frame_height,
                        frame_width,
                        frame_height
                    );
                }
            }

            if (generate_masks) {
                generate_bitmasks_from_surface(surf);
            }

            // Upload
            auto tex_res = sdlpp::texture::create(r, surf);
            if (tex_res) {
                m_texture = std::move(*tex_res);
            }
        }
    }

    sprite_atlas::sprite_atlas(
        sdlpp::renderer& r,
        const sdlpp::surface& surf,
        std::vector<sdlpp::rect<int>> frames,
        bool generate_masks)
        : m_frames(std::move(frames)) {
        if (surf) {
            if (generate_masks) {
                generate_bitmasks_from_surface(surf);
            }

            // Upload
            auto tex_res = sdlpp::texture::create(r, surf);
            if (tex_res) {
                m_texture = std::move(*tex_res);
            }
        }
    }

    void sprite_atlas::slice_grid(int frame_width, int frame_height) {
        if (!m_texture || frame_width <= 0 || frame_height <= 0) {
            return;
        }

        auto size_res = m_texture.get_size();
        if (!size_res) {
            return;
        }

        int tex_w = size_res->width;
        int tex_h = size_res->height;

        int cols = tex_w / frame_width;
        int rows = tex_h / frame_height;

        m_frames.clear();
        m_frames.reserve(static_cast<std::size_t>(rows * cols));

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                m_frames.emplace_back(
                    c * frame_width,
                    r * frame_height,
                    frame_width,
                    frame_height
                );
            }
        }
    }

    void sprite_atlas::generate_bitmasks_from_surface(const sdlpp::surface& surf) {
        if (!surf) return;

        // Ensure surface is locked for reading pixels on CPU
        sdlpp::surface::lock_guard lock(const_cast<sdlpp::surface&>(surf));

        m_masks.clear();
        m_masks.reserve(m_frames.size());

        for (const auto& f : m_frames) {
            bitmask mask(f.w, f.h);
            for (int y = 0; y < f.h; ++y) {
                for (int x = 0; x < f.w; ++x) {
                    auto color_res = surf.get_pixel(f.x + x, f.y + y);
                    if (color_res) {
                        // Consider opaque if alpha value is at least 128 (out of 255)
                        mask.set(x, y, color_res->a >= 128);
                    }
                }
            }
            m_masks.push_back(std::move(mask));
        }
    }

    sdlpp::expected<sprite_atlas, std::string> sprite_atlas::load(
        sdlpp::renderer& r,
        const std::filesystem::path& path,
        int frame_width,
        int frame_height,
        bool generate_masks) {
        
        auto surf_res = sdlpp::image::load(path);
        if (!surf_res) {
            return sdlpp::make_unexpected(surf_res.error());
        }

        return sprite_atlas(r, *surf_res, frame_width, frame_height, generate_masks);
    }

    sdlpp::expected<sprite_atlas, std::string> sprite_atlas::load(
        sdlpp::renderer& r,
        const std::filesystem::path& path,
        std::vector<sdlpp::rect<int>> frames,
        bool generate_masks) {

        auto surf_res = sdlpp::image::load(path);
        if (!surf_res) {
            return sdlpp::make_unexpected(surf_res.error());
        }

        return sprite_atlas(r, *surf_res, std::move(frames), generate_masks);
    }
} // namespace simplex
