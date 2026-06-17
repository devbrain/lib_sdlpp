#include <simplex/sprite_atlas.hh>
#include <sdlpp/image/image.hh>
#include <algorithm>
#include <fstream>
#include <string_view>

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

    sdlpp::expected<sprite_atlas, std::string> sprite_atlas::load_wgt_spr(
        sdlpp::renderer& r,
        const std::filesystem::path& path,
        bool generate_masks) {

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return sdlpp::make_unexpected("Could not open WGT sprite file: " + path.string());
        }

        uint16_t version = 0;
        if (!file.read(reinterpret_cast<char*>(&version), 2)) {
            return sdlpp::make_unexpected("Failed to read WGT version");
        }

        char sig[13];
        if (!file.read(sig, 13)) {
            return sdlpp::make_unexpected("Failed to read WGT header signature");
        }

        if (std::string_view(sig, 13) != " Sprite File ") {
            return sdlpp::make_unexpected("Invalid WGT sprite file signature");
        }

        uint8_t pal_raw[768];
        if (!file.read(reinterpret_cast<char*>(pal_raw), 768)) {
            return sdlpp::make_unexpected("Failed to read WGT palette data");
        }

        uint16_t maxsprite = 0;
        if (!file.read(reinterpret_cast<char*>(&maxsprite), 2)) {
            return sdlpp::make_unexpected("Failed to read WGT maxsprite");
        }

        int startingsprite = (version <= 3) ? 1 : 0;
        int total_sprites_bound = maxsprite + 1;

        struct wgt_block {
            uint16_t w, h;
            std::vector<uint8_t> pixels;
        };
        std::vector<wgt_block> blocks;
        blocks.reserve(static_cast<std::size_t>(total_sprites_bound));

        int max_w = 0;
        int max_h = 0;

        for (int i = startingsprite; i <= maxsprite; ++i) {
            uint16_t spritemade = 0;
            if (!file.read(reinterpret_cast<char*>(&spritemade), 2)) {
                return sdlpp::make_unexpected("Failed to read spritemade flag at sprite " + std::to_string(i));
            }

            if (spritemade == 1) {
                uint16_t w, h;
                if (!file.read(reinterpret_cast<char*>(&w), 2) || !file.read(reinterpret_cast<char*>(&h), 2)) {
                    return sdlpp::make_unexpected("Failed to read dimensions at sprite " + std::to_string(i));
                }

                std::vector<uint8_t> pixels(static_cast<std::size_t>(w * h));
                if (!file.read(reinterpret_cast<char*>(pixels.data()), w * h)) {
                    return sdlpp::make_unexpected("Failed to read pixel data at sprite " + std::to_string(i));
                }

                blocks.push_back({w, h, std::move(pixels)});
                max_w = std::max(max_w, static_cast<int>(w));
                max_h = std::max(max_h, static_cast<int>(h));
            } else {
                // Keep the sprite index aligned by placing an empty placeholder
                blocks.push_back({0, 0, {}});
            }
        }

        int total_loaded_sprites = static_cast<int>(blocks.size());
        if (total_loaded_sprites == 0 || max_w <= 0 || max_h <= 0) {
            return sdlpp::make_unexpected("No valid sprites loaded from WGT sprite file");
        }

        int cols = 5;
        int rows = (total_loaded_sprites + cols - 1) / cols;

        int atlas_w = cols * max_w;
        int atlas_h = rows * max_h;

        auto surf_res = sdlpp::surface::create_rgb(atlas_w, atlas_h, sdlpp::pixel_format_enum::RGBA8888);
        if (!surf_res) {
            return sdlpp::make_unexpected("Failed to create atlas surface: " + surf_res.error());
        }
        auto& surf = *surf_res;

        surf.fill(sdlpp::color{0, 0, 0, 0});

        std::vector<sdlpp::rect<int>> frames;
        frames.reserve(static_cast<std::size_t>(total_loaded_sprites));

        {
            sdlpp::surface::lock_guard lock(surf);

            for (std::size_t i = 0; i < static_cast<std::size_t>(total_loaded_sprites); ++i) {
                const auto& b = blocks[i];
                int col = static_cast<int>(i) % cols;
                int row = static_cast<int>(i) / cols;

                int frame_x = col * max_w;
                int frame_y = row * max_h;

                frames.emplace_back(frame_x, frame_y, b.w, b.h);

                // Skip processing pixels for empty placeholder frames
                if (b.w == 0 || b.h == 0) {
                    continue;
                }

                for (int y = 0; y < b.h; ++y) {
                    for (int x = 0; x < b.w; ++x) {
                        uint8_t idx = b.pixels[static_cast<std::size_t>(y * b.w + x)];
                        if (idx == 0) {
                            surf.put_pixel(frame_x + x, frame_y + y, sdlpp::color{0, 0, 0, 0});
                        } else {
                            uint8_t r_val = pal_raw[idx * 3] * 4;
                            uint8_t g_val = pal_raw[idx * 3 + 1] * 4;
                            uint8_t b_val = pal_raw[idx * 3 + 2] * 4;
                            surf.put_pixel(frame_x + x, frame_y + y, sdlpp::color{r_val, g_val, b_val, 255});
                        }
                    }
                }
            }
        }

        return sprite_atlas(r, surf, std::move(frames), generate_masks);
    }
} // namespace simplex
