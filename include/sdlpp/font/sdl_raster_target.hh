//
// SDL raster target for onyx_font
//

#pragma once

#include <sdlpp/video/surface.hh>
#include <sdlpp/video/color.hh>
#include <onyx_font/text/raster_target.hh>
#include <cstdint>
#include <algorithm>

namespace sdlpp::font {

/**
 * @brief Raster target that renders text directly to an sdlpp::surface.
 *
 * Supports alpha blending with a configurable text color.
 * Works with RGBA32 surfaces for best results.
 *
 * @code
 * sdlpp::surface canvas = ...;
 * sdlpp::font::surface_raster_target target(canvas, {255, 255, 255, 255});
 * rasterizer.rasterize_text("Hello", target, 10, 50);
 * @endcode
 */
class surface_raster_target {
public:
    /**
     * @brief Construct target for SDL surface.
     * @param surface Target surface (should be RGBA32 format for best results)
     * @param text_color Color for rendered text
     */
    surface_raster_target(sdlpp::surface& surface, const color& text_color)
        : m_surface(surface)
        , m_color(text_color)
        , m_width(static_cast<int>(surface.width()))
        , m_height(static_cast<int>(surface.height()))
        , m_pitch(surface.get_pitch()) {}

    /**
     * @brief Blend a single pixel onto the surface.
     * @param x X coordinate
     * @param y Y coordinate
     * @param alpha Glyph coverage (0-255)
     */
    void put_pixel(int x, int y, std::uint8_t alpha) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height || alpha == 0) {
            return;
        }

        auto* pixels = static_cast<std::uint8_t*>(m_surface.get_pixels());
        if (!pixels) return;

        // Assume RGBA32 format (4 bytes per pixel)
        std::uint8_t* dst = pixels + y * m_pitch + x * 4;

        if (alpha == 255) {
            // Fully opaque - direct write
            dst[0] = m_color.r;
            dst[1] = m_color.g;
            dst[2] = m_color.b;
            dst[3] = 255;
        } else {
            // Alpha blend using Porter-Duff "over"
            std::uint32_t inv_alpha = 255 - alpha;
            dst[0] = static_cast<std::uint8_t>((m_color.r * alpha + dst[0] * inv_alpha + 127) / 255);
            dst[1] = static_cast<std::uint8_t>((m_color.g * alpha + dst[1] * inv_alpha + 127) / 255);
            dst[2] = static_cast<std::uint8_t>((m_color.b * alpha + dst[2] * inv_alpha + 127) / 255);
            dst[3] = static_cast<std::uint8_t>((255 * alpha + dst[3] * inv_alpha + 127) / 255);
        }
    }

    /**
     * @brief Write a horizontal span of pixels (optimized).
     * @param x Starting X coordinate
     * @param y Y coordinate
     * @param alphas Array of alpha values
     * @param count Number of pixels
     */
    void put_span(int x, int y, const std::uint8_t* alphas, int count) {
        if (y < 0 || y >= m_height) return;

        int x0 = std::max(0, x);
        int x1 = std::min(m_width, x + count);

        for (int px = x0; px < x1; ++px) {
            put_pixel(px, y, alphas[px - x]);
        }
    }

    [[nodiscard]] int width() const noexcept { return m_width; }
    [[nodiscard]] int height() const noexcept { return m_height; }

    /**
     * @brief Change text color for subsequent rendering.
     * @param c New text color
     */
    void set_color(const color& c) noexcept { m_color = c; }

private:
    sdlpp::surface& m_surface;
    color m_color;
    int m_width;
    int m_height;
    int m_pitch;
};

// Verify concept satisfaction
static_assert(onyx_font::raster_target<surface_raster_target>);
static_assert(onyx_font::raster_target_with_span<surface_raster_target>);

} // namespace sdlpp::font
