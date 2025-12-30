//
// SDL surface adapter for onyx_image
//

#include <sdlpp/image/sdl_surface_adapter.hh>
#include <cstring>

namespace sdlpp::image {

sdl_surface_adapter::sdl_surface_adapter(sdlpp::surface& target)
    : m_target(target) {}

pixel_format_enum to_sdl_format(onyx_image::pixel_format fmt) {
    switch (fmt) {
        case onyx_image::pixel_format::indexed8:
            return pixel_format_enum::INDEX8;
        case onyx_image::pixel_format::rgb888:
            return pixel_format_enum::RGB24;
        case onyx_image::pixel_format::rgba8888:
            return pixel_format_enum::RGBA8888;
    }
    return pixel_format_enum::RGBA8888;
}

pixel_format_enum sdl_surface_adapter::to_sdl_format(onyx_image::pixel_format fmt) {
    return sdlpp::image::to_sdl_format(fmt);
}

bool sdl_surface_adapter::set_size(int width, int height, onyx_image::pixel_format format) {
    auto sdl_fmt = to_sdl_format(format);
    auto result = sdlpp::surface::create_rgb(width, height, sdl_fmt);
    if (!result) {
        return false;
    }
    m_target = std::move(*result);
    m_subrects.clear();
    return true;
}

void sdl_surface_adapter::write_pixels(int x, int y, int count, const std::uint8_t* pixels) {
    if (!m_target) return;

    auto* dst = static_cast<std::uint8_t*>(m_target.get_pixels());
    if (!dst) return;

    int pitch = m_target.get_pitch();
    int h = static_cast<int>(m_target.height());

    // Bounds check
    if (y < 0 || y >= h) return;
    if (x < 0) {
        count += x;
        pixels -= x;
        x = 0;
    }
    if (count <= 0) return;
    if (x + count > pitch) {
        count = pitch - x;
    }
    if (count <= 0) return;

    std::memcpy(dst + y * pitch + x, pixels, static_cast<std::size_t>(count));
}

void sdl_surface_adapter::write_pixel(int x, int y, std::uint8_t pixel) {
    if (!m_target) return;

    auto* dst = static_cast<std::uint8_t*>(m_target.get_pixels());
    if (!dst) return;

    int w = static_cast<int>(m_target.width());
    int h = static_cast<int>(m_target.height());
    int pitch = m_target.get_pitch();

    if (x < 0 || x >= w || y < 0 || y >= h) return;

    dst[y * pitch + x] = pixel;
}

void sdl_surface_adapter::set_palette_size(int count) {
    if (!m_target || m_target.format() != pixel_format_enum::INDEX8) return;

    auto pal_result = palette::create(count);
    if (pal_result) {
        m_palette = std::move(*pal_result);
        (void)m_target.set_palette(m_palette.cref());
    }
}

void sdl_surface_adapter::write_palette(int start, std::span<const std::uint8_t> colors) {
    if (!m_target || !m_palette.is_valid()) return;

    // Convert RGB triplets to SDL_Color array
    int num_colors = static_cast<int>(colors.size() / 3);
    std::vector<color> sdl_colors;
    sdl_colors.reserve(static_cast<std::size_t>(num_colors));

    for (int i = 0; i < num_colors; ++i) {
        sdl_colors.push_back({
            colors[static_cast<std::size_t>(i * 3 + 0)],  // R
            colors[static_cast<std::size_t>(i * 3 + 1)],  // G
            colors[static_cast<std::size_t>(i * 3 + 2)],  // B
            255                                            // A
        });
    }

    m_palette.set_colors(sdl_colors, start);
}

void sdl_surface_adapter::set_subrect(int index, const onyx_image::subrect& sr) {
    if (static_cast<std::size_t>(index) >= m_subrects.size()) {
        m_subrects.resize(static_cast<std::size_t>(index) + 1);
    }
    m_subrects[static_cast<std::size_t>(index)] = sr;
}

} // namespace sdlpp::image
