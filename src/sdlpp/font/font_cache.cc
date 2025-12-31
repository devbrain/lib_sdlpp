//
// Font glyph cache for SDL++ - implementation
//

#include <sdlpp/font/font_cache.hh>
#include <sdlpp/font/font.hh>
#include <sdlpp/font/sdl_raster_target.hh>

#include <onyx_font/text/utf8.hh>
#include <cmath>
#include <algorithm>

namespace sdlpp::font {

// ============================================================================
// Constructor / Destructor
// ============================================================================

font_cache::font_cache(renderer& renderer, font& fnt)
    : m_renderer(&renderer)
    , m_font(&fnt) {}

font_cache::~font_cache() = default;

font_cache::font_cache(font_cache&&) noexcept = default;
font_cache& font_cache::operator=(font_cache&&) noexcept = default;

// ============================================================================
// Glyph Caching
// ============================================================================

bool font_cache::store_glyph(char32_t codepoint) {
    if (m_glyphs.contains(codepoint)) {
        return true;  // Already cached
    }

    auto* rast = m_font->rasterizer();
    if (!rast) return false;

    // Measure the glyph
    auto metrics = rast->measure_glyph(codepoint);
    if (metrics.advance_x <= 0) {
        return false;  // Invalid glyph
    }

    // Calculate surface size based on actual glyph bounds (add padding for antialiasing)
    // Width: must fit both the glyph and the advance
    float glyph_right = metrics.bearing_x + metrics.width;
    int width = static_cast<int>(std::ceil(std::max(metrics.advance_x, glyph_right))) + 2;

    // Height: must fit both the font metrics and the actual glyph
    auto font_metrics = rast->get_metrics();
    float glyph_descent = metrics.height - metrics.bearing_y;  // Part below baseline
    float required_descent = std::max(font_metrics.descent, glyph_descent);
    int height = static_cast<int>(std::ceil(font_metrics.ascent + required_descent)) + 2;

    if (width <= 0 || height <= 0) {
        return false;
    }

    // Create surface for glyph - use ABGR8888 which stores bytes as R,G,B,A on little-endian
    auto surf_result = surface::create_rgb(width, height, pixel_format_enum::ABGR8888);
    if (!surf_result) {
        return false;
    }

    auto& surf = *surf_result;

    // Clear to transparent
    surf.fill_rect(rect<int>(0, 0, width, height), color{0, 0, 0, 0});

    // Render glyph in white (will be color modulated when drawing)
    surface_raster_target target(surf, {255, 255, 255, 255});

    int baseline = static_cast<int>(std::ceil(font_metrics.ascent)) + 1;

    rast->rasterize_glyph(codepoint, target, 1, baseline);

    // Create texture
    auto tex_result = texture::create(*m_renderer, surf);
    if (!tex_result) {
        return false;
    }

    // Enable blending on texture
    tex_result->set_blend_mode(blend_mode::blend);

    // Store in cache
    glyph_data data;
    data.tex = std::move(*tex_result);
    // Use bearing_x for proper horizontal positioning
    // bearing_x is the distance from pen position to left edge of glyph
    data.offset_x = static_cast<int>(std::floor(metrics.bearing_x));
    data.offset_y = 0;
    data.advance = static_cast<int>(std::ceil(metrics.advance_x));
    data.width = width;
    data.height = height;

    m_glyphs[codepoint] = std::move(data);
    return true;
}

void font_cache::store_glyphs(char32_t begin, char32_t end) {
    for (char32_t cp = begin; cp < end; ++cp) {
        store_glyph(cp);
    }
}

const glyph_data* font_cache::find_glyph(char32_t codepoint) const {
    auto it = m_glyphs.find(codepoint);
    return it != m_glyphs.end() ? &it->second : nullptr;
}

// ============================================================================
// String Caching
// ============================================================================

font_cache::string_id font_cache::store_string(std::string_view text, const color& fg) {
    auto surf_result = m_font->render_text(text, fg);
    if (!surf_result) {
        return 0;  // Failed
    }

    auto tex_result = texture::create(*m_renderer, *surf_result);
    if (!tex_result) {
        return 0;
    }

    tex_result->set_blend_mode(blend_mode::blend);

    string_id id = m_next_string_id++;
    m_strings[id] = std::move(*tex_result);
    return id;
}

const texture* font_cache::find_string(string_id id) const {
    auto it = m_strings.find(id);
    return it != m_strings.end() ? &it->second : nullptr;
}

void font_cache::remove_string(string_id id) {
    m_strings.erase(id);
}

// ============================================================================
// Rendering
// ============================================================================

int font_cache::render_glyph(char32_t codepoint, int x, int y, const color& fg) {
    // Try to find cached glyph
    const glyph_data* glyph = find_glyph(codepoint);

    // Cache on-demand if not found
    if (!glyph) {
        if (!store_glyph(codepoint)) {
            return x;  // Failed, return same position
        }
        glyph = find_glyph(codepoint);
        if (!glyph) return x;
    }

    // Set color modulation
    const_cast<texture&>(glyph->tex).set_color_mod(fg);
    const_cast<texture&>(glyph->tex).set_alpha_mod(fg.a);

    // Draw glyph
    rect<int> dst(x + glyph->offset_x, y + glyph->offset_y, glyph->width, glyph->height);
    m_renderer->copy(glyph->tex, std::optional<rect<int>>{}, std::optional{dst});

    return x + glyph->advance;
}

int font_cache::render_text(std::string_view text, int x, int y, const color& fg) {
    int pen_x = x;

    for (char32_t codepoint : onyx_font::utf8_view(text)) {
        pen_x = render_glyph(codepoint, pen_x, y, fg);
    }

    return pen_x - x;  // Return width
}

void font_cache::render_string(string_id id, int x, int y) {
    const texture* tex = find_string(id);
    if (!tex) return;

    auto size_result = tex->get_size();
    if (!size_result) return;

    auto sz = *size_result;
    rect<int> dst(x, y, sz.width, sz.height);
    m_renderer->copy(*tex, std::optional<rect<int>>{}, std::optional{dst});
}

// ============================================================================
// Cache Management
// ============================================================================

void font_cache::clear_glyphs() {
    m_glyphs.clear();
}

void font_cache::clear_strings() {
    m_strings.clear();
    m_next_string_id = 1;
}

} // namespace sdlpp::font
