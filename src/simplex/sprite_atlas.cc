#include <simplex/sprite_atlas.hh>
#include <sdlpp/image/image.hh>

namespace simplex {
    sprite_atlas::sprite_atlas(sdlpp::texture tex, int frame_width, int frame_height)
        : m_texture(std::move(tex)) {
        slice_grid(frame_width, frame_height);
    }

    sprite_atlas::sprite_atlas(sdlpp::texture tex, std::vector<sdlpp::rect<int>> frames)
        : m_texture(std::move(tex)), m_frames(std::move(frames)) {
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

    sdlpp::expected<sprite_atlas, std::string> sprite_atlas::load(
        sdlpp::renderer& r,
        const std::filesystem::path& path,
        int frame_width,
        int frame_height) {
        auto tex_res = sdlpp::image::load_texture(r, path);
        if (!tex_res) {
            return sdlpp::make_unexpected(tex_res.error());
        }

        return sprite_atlas(std::move(*tex_res), frame_width, frame_height);
    }

    sdlpp::expected<sprite_atlas, std::string> sprite_atlas::load(
        sdlpp::renderer& r,
        const std::filesystem::path& path,
        std::vector<sdlpp::rect<int>> frames) {
        auto tex_res = sdlpp::image::load_texture(r, path);
        if (!tex_res) {
            return sdlpp::make_unexpected(tex_res.error());
        }

        return sprite_atlas(std::move(*tex_res), std::move(frames));
    }
} // namespace simplex
