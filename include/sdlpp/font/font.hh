//
// High-level font API for SDL++
//

#pragma once

#include <sdlpp/video/surface.hh>
#include <sdlpp/video/texture.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/detail/expected.hh>
#include <failsafe/detail/string_utils.hh>

#include <onyx_font/font_factory.hh>
#include <onyx_font/text/font_source.hh>
#include <onyx_font/text/text_rasterizer.hh>
#include <onyx_font/text/text_style.hh>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <span>
#include <vector>

namespace sdlpp::font {

// Re-export commonly used types from onyx_font
using text_style = onyx_font::text_style;
using render_style = onyx_font::render_style;
using font_type = onyx_font::font_type;

/**
 * @brief Text metrics for measurement.
 */
struct text_metrics {
    float width = 0.0f;      ///< Total width
    float height = 0.0f;     ///< Total height (ascent + descent)
    float ascent = 0.0f;     ///< Distance from baseline to top
    float descent = 0.0f;    ///< Distance from baseline to bottom (positive)
    float line_height = 0.0f;///< Recommended line spacing
};

/**
 * @brief High-level font wrapper for SDL++.
 *
 * Wraps onyx_font functionality with SDL-friendly interface.
 * Supports TTF, Windows FON, BGI vector fonts, and raw BIOS fonts.
 *
 * @code
 * // Load TTF font
 * auto result = sdlpp::font::font::load("arial.ttf");
 * if (result) {
 *     auto& font = *result;
 *     font.set_size(24.0f);
 *
 *     // Render to surface
 *     auto surface = font.render_text("Hello!", colors::white);
 *
 *     // Or render to texture
 *     auto texture = font.render_texture(renderer, "World!", colors::yellow);
 * }
 * @endcode
 */
class font {
public:
    /**
     * @brief Load font from file.
     *
     * Auto-detects format (TTF, FON, BGI, etc.)
     *
     * @param path Path to font file
     * @param index Font index for multi-font containers (default 0)
     * @return Loaded font or error message
     */
    [[nodiscard]] SDLPP_EXPORT static expected<font, std::string> load(
        const std::filesystem::path& path,
        std::size_t index = 0);

    /**
     * @brief Load font from memory.
     *
     * @param data Font data (must remain valid for font lifetime)
     * @param index Font index for multi-font containers
     * @return Loaded font or error message
     */
    [[nodiscard]] SDLPP_EXPORT static expected<font, std::string> load(
        std::span<const std::uint8_t> data,
        std::size_t index = 0);

    /**
     * @brief Load raw BIOS font dump.
     *
     * @param data Raw font data
     * @param options Font format options
     * @return Loaded font or error message
     */
    [[nodiscard]] SDLPP_EXPORT static expected<font, std::string> load_raw(
        std::span<const std::uint8_t> data,
        const onyx_font::raw_font_options& options);

    /**
     * @brief Destructor.
     */
    SDLPP_EXPORT ~font();

    // Move-only
    font(const font&) = delete;
    font& operator=(const font&) = delete;
    SDLPP_EXPORT font(font&&) noexcept;
    SDLPP_EXPORT font& operator=(font&&) noexcept;

    // ========================================================================
    // Font Information
    // ========================================================================

    /**
     * @brief Check if font is valid.
     */
    [[nodiscard]] bool is_valid() const noexcept { return m_valid; }
    explicit operator bool() const noexcept { return is_valid(); }

    /**
     * @brief Get font type (bitmap, vector, or outline).
     */
    [[nodiscard]] SDLPP_EXPORT font_type type() const;

    /**
     * @brief Check if this is a scalable font.
     */
    [[nodiscard]] SDLPP_EXPORT bool is_scalable() const;

    /**
     * @brief Get native size for bitmap fonts.
     * @return Native pixel height, or 0 for scalable fonts
     */
    [[nodiscard]] SDLPP_EXPORT float native_size() const;

    /**
     * @brief Get number of fonts in the loaded container.
     */
    [[nodiscard]] std::size_t font_count() const noexcept { return m_container_info.fonts.size(); }

    // ========================================================================
    // Size and Style
    // ========================================================================

    /**
     * @brief Set rendering size in pixels.
     * @param pixels Font height in pixels
     */
    SDLPP_EXPORT void set_size(float pixels);

    /**
     * @brief Get current rendering size.
     */
    [[nodiscard]] float size() const noexcept { return m_size; }

    /**
     * @brief Set text style (bold, italic, underline, strikethrough).
     * @param style Style flags (can be combined with |)
     */
    SDLPP_EXPORT void set_style(text_style style);

    /**
     * @brief Set detailed render style.
     */
    SDLPP_EXPORT void set_style(const render_style& style);

    /**
     * @brief Get current style flags.
     */
    [[nodiscard]] SDLPP_EXPORT text_style style() const;

    /**
     * @brief Reset style to normal.
     */
    SDLPP_EXPORT void reset_style();

    // ========================================================================
    // Measurement
    // ========================================================================

    /**
     * @brief Measure text dimensions.
     * @param text UTF-8 encoded text
     * @return Text metrics (width, height, ascent, descent)
     */
    [[nodiscard]] SDLPP_EXPORT text_metrics measure(std::string_view text) const;

    /**
     * @brief Measure text with word wrapping.
     * @param text UTF-8 encoded text
     * @param max_width Maximum line width
     * @return Text metrics for wrapped text
     */
    [[nodiscard]] SDLPP_EXPORT text_metrics measure_wrapped(
        std::string_view text, float max_width) const;

    /**
     * @brief Get font metrics at current size.
     */
    [[nodiscard]] SDLPP_EXPORT text_metrics get_metrics() const;

    /**
     * @brief Get line height at current size.
     */
    [[nodiscard]] SDLPP_EXPORT float line_height() const;

    // ========================================================================
    // Rendering
    // ========================================================================

    /**
     * @brief Render text to a new surface.
     *
     * Creates a surface exactly sized for the text.
     *
     * @param text UTF-8 encoded text
     * @param fg Foreground (text) color
     * @param bg Background color (use {0,0,0,0} for transparent)
     * @return Surface containing rendered text
     */
    [[nodiscard]] SDLPP_EXPORT expected<surface, std::string> render_text(
        std::string_view text,
        const color& fg,
        const color& bg = {0, 0, 0, 0}) const;

    template<typename... Args>
    requires (sizeof...(Args) > 0)
             && !(sizeof...(Args) == 1
                  && std::is_convertible_v<std::remove_cvref_t<
                         std::tuple_element_t<0, std::tuple<Args...>>>,
                     std::string_view>)
    [[nodiscard]] expected<surface, std::string> render_text(
        const color& fg,
        const color& bg,
        Args&&... args) const {
        return render_text(
            failsafe::detail::build_message(std::forward<Args>(args)...),
            fg,
            bg);
    }

    template<typename... Args>
    requires (sizeof...(Args) > 0)
             && !(sizeof...(Args) == 1
                  && std::is_convertible_v<std::remove_cvref_t<
                         std::tuple_element_t<0, std::tuple<Args...>>>,
                     std::string_view>)
    [[nodiscard]] expected<surface, std::string> render_text(
        const color& fg,
        Args&&... args) const {
        return render_text(fg, {0, 0, 0, 0}, std::forward<Args>(args)...);
    }

    /**
     * @brief Render text directly to a texture.
     *
     * @param renderer Renderer for texture creation
     * @param text UTF-8 encoded text
     * @param fg Text color
     * @return Texture containing rendered text
     */
    [[nodiscard]] SDLPP_EXPORT expected<texture, std::string> render_texture(
        renderer& renderer,
        std::string_view text,
        const color& fg) const;

    template<typename... Args>
    requires (sizeof...(Args) > 0)
             && !(sizeof...(Args) == 1
                  && std::is_convertible_v<std::remove_cvref_t<
                         std::tuple_element_t<0, std::tuple<Args...>>>,
                     std::string_view>)
    [[nodiscard]] expected<texture, std::string> render_texture(
        renderer& renderer,
        const color& fg,
        Args&&... args) const {
        return render_texture(
            renderer,
            failsafe::detail::build_message(std::forward<Args>(args)...),
            fg);
    }

    /**
     * @brief Render text onto an existing surface.
     *
     * @param target Target surface
     * @param text UTF-8 encoded text
     * @param x X position
     * @param y Y position (top of text, not baseline)
     * @param fg Text color
     * @return Width of rendered text
     */
    SDLPP_EXPORT float render_to(
        surface& target,
        std::string_view text,
        int x, int y,
        const color& fg) const;

    // ========================================================================
    // Access to Underlying onyx_font Objects
    // ========================================================================

    /**
     * @brief Get the underlying text rasterizer.
     *
     * For advanced usage with onyx_font API.
     */
    [[nodiscard]] SDLPP_EXPORT onyx_font::text_rasterizer* rasterizer();
    [[nodiscard]] SDLPP_EXPORT const onyx_font::text_rasterizer* rasterizer() const;

private:
    font() = default;

    std::vector<std::uint8_t> m_data;  // Owned font data
    onyx_font::container_info m_container_info;
    float m_size = 12.0f;
    bool m_valid = false;

    // Union of possible font types
    struct impl;
    std::unique_ptr<impl> m_impl;
};

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Get list of supported font formats.
 */
[[nodiscard]] SDLPP_EXPORT std::vector<std::string> supported_formats();

/**
 * @brief Analyze a font file.
 * @param path Path to font file
 * @return Container info with format and font list
 */
[[nodiscard]] SDLPP_EXPORT expected<onyx_font::container_info, std::string> analyze(
    const std::filesystem::path& path);

/**
 * @brief Analyze font data in memory.
 * @param data Raw font data
 * @return Container info with format and font list
 */
[[nodiscard]] SDLPP_EXPORT onyx_font::container_info analyze(
    std::span<const std::uint8_t> data);

} // namespace sdlpp::font
