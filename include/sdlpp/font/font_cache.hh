//
// Font glyph cache for SDL++
//

#pragma once

#include <sdlpp/video/texture.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/detail/expected.hh>

#include <unordered_map>
#include <memory>
#include <cstdint>
#include <string_view>

namespace sdlpp::font {

// Forward declaration
class font;

/**
 * @brief Cached glyph data (texture + metrics).
 */
struct glyph_data {
    texture tex;        ///< Glyph texture
    int offset_x = 0;   ///< X offset from pen position
    int offset_y = 0;   ///< Y offset from baseline
    int advance = 0;    ///< Horizontal advance to next glyph
    int width = 0;      ///< Glyph width
    int height = 0;     ///< Glyph height
};

/**
 * @brief Font cache for efficient repeated text rendering.
 *
 * Caches individual glyphs as textures and pre-rendered strings.
 *
 * @code
 * font_cache cache(renderer, my_font);
 *
 * // Pre-cache common characters
 * cache.store_basic_latin();
 *
 * // Render using cached glyphs
 * cache.render_text("Hello", 100, 100, colors::white);
 *
 * // Cache entire strings for static text
 * auto id = cache.store_string("Score: 0");
 * cache.render_string(id, 10, 10);
 * @endcode
 */
class font_cache {
public:
    using string_id = std::size_t;

    /**
     * @brief Create a font cache.
     * @param renderer Renderer for texture creation
     * @param fnt Font to cache glyphs from
     */
    SDLPP_EXPORT font_cache(renderer& renderer, font& fnt);

    /**
     * @brief Destructor.
     */
    SDLPP_EXPORT ~font_cache();

    // Move-only
    font_cache(const font_cache&) = delete;
    font_cache& operator=(const font_cache&) = delete;
    SDLPP_EXPORT font_cache(font_cache&&) noexcept;
    SDLPP_EXPORT font_cache& operator=(font_cache&&) noexcept;

    // ========================================================================
    // Glyph Caching
    // ========================================================================

    /**
     * @brief Pre-cache a single glyph.
     * @param codepoint Unicode codepoint
     * @return true if glyph was cached
     */
    SDLPP_EXPORT bool store_glyph(char32_t codepoint);

    /**
     * @brief Pre-cache a range of glyphs.
     * @param begin First codepoint (inclusive)
     * @param end Last codepoint (exclusive)
     */
    SDLPP_EXPORT void store_glyphs(char32_t begin, char32_t end);

    /**
     * @brief Pre-cache Basic Latin characters (U+0020 to U+007E).
     */
    void store_basic_latin() { store_glyphs(0x0020, 0x007F); }

    /**
     * @brief Pre-cache Latin-1 Supplement (U+00A0 to U+00FF).
     */
    void store_latin1_supplement() { store_glyphs(0x00A0, 0x0100); }

    /**
     * @brief Pre-cache all Latin-1 characters.
     */
    void store_latin1() {
        store_basic_latin();
        store_latin1_supplement();
    }

    /**
     * @brief Find a cached glyph.
     * @return Pointer to glyph data, or nullptr if not cached
     */
    [[nodiscard]] SDLPP_EXPORT const glyph_data* find_glyph(char32_t codepoint) const;

    /**
     * @brief Check if glyph is cached.
     */
    [[nodiscard]] bool has_glyph(char32_t codepoint) const {
        return find_glyph(codepoint) != nullptr;
    }

    // ========================================================================
    // String Caching
    // ========================================================================

    /**
     * @brief Cache a pre-rendered string.
     * @param text UTF-8 text to render and cache
     * @param fg Text color
     * @return ID for later retrieval
     */
    SDLPP_EXPORT string_id store_string(std::string_view text, const color& fg);

    /**
     * @brief Find a cached string texture.
     * @return Pointer to texture, or nullptr if not cached
     */
    [[nodiscard]] SDLPP_EXPORT const texture* find_string(string_id id) const;

    /**
     * @brief Check if string is cached.
     */
    [[nodiscard]] bool has_string(string_id id) const {
        return find_string(id) != nullptr;
    }

    /**
     * @brief Remove a cached string.
     */
    SDLPP_EXPORT void remove_string(string_id id);

    // ========================================================================
    // Rendering
    // ========================================================================

    /**
     * @brief Render a single cached glyph.
     * @param codepoint Unicode codepoint
     * @param x X position
     * @param y Y position (top of glyph, not baseline)
     * @param fg Color modulation
     * @return X position for next glyph, or x if glyph not found
     */
    SDLPP_EXPORT int render_glyph(char32_t codepoint, int x, int y, const color& fg);

    /**
     * @brief Render text using cached glyphs.
     *
     * Glyphs are cached on-demand if not already present.
     *
     * @param text UTF-8 text
     * @param x X position
     * @param y Y position (top of text, not baseline)
     * @param fg Text color
     * @return Width of rendered text
     */
    SDLPP_EXPORT int render_text(std::string_view text, int x, int y, const color& fg);

    /**
     * @brief Render a cached string.
     * @param id String ID from store_string()
     * @param x X position
     * @param y Y position
     */
    SDLPP_EXPORT void render_string(string_id id, int x, int y);

    // ========================================================================
    // Cache Management
    // ========================================================================

    /**
     * @brief Clear all cached glyphs.
     */
    SDLPP_EXPORT void clear_glyphs();

    /**
     * @brief Clear all cached strings.
     */
    SDLPP_EXPORT void clear_strings();

    /**
     * @brief Clear entire cache.
     */
    void clear() {
        clear_glyphs();
        clear_strings();
    }

    /**
     * @brief Get number of cached glyphs.
     */
    [[nodiscard]] std::size_t glyph_count() const noexcept { return m_glyphs.size(); }

    /**
     * @brief Get number of cached strings.
     */
    [[nodiscard]] std::size_t string_count() const noexcept { return m_strings.size(); }

private:
    renderer* m_renderer;
    font* m_font;

    std::unordered_map<char32_t, glyph_data> m_glyphs;
    std::unordered_map<string_id, texture> m_strings;
    string_id m_next_string_id = 1;
};

} // namespace sdlpp::font
