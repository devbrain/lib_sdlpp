//
// High-level image loading API for SDL++
//

#pragma once

#include <sdlpp/video/surface.hh>
#include <sdlpp/video/texture.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/image/sdl_surface_adapter.hh>

#include <onyx_image/onyx_image.hpp>
#include <span>
#include <string>
#include <filesystem>

namespace sdlpp::image {

/**
 * @brief Load an image from file into an SDL surface.
 *
 * Automatically detects format from file contents. Supports 30+ formats
 * including PNG, JPEG, GIF, BMP, PCX, TGA, and retro formats (C64, Atari ST, etc.)
 *
 * @param path Path to image file
 * @return Surface containing decoded image, or error message
 *
 * @code
 * auto result = sdlpp::image::load("sprite.png");
 * if (result) {
 *     auto& surface = *result;
 *     // Use surface...
 * }
 * @endcode
 */
[[nodiscard]] SDLPP_EXPORT expected<surface, std::string> load(const std::filesystem::path& path);

/**
 * @brief Load an image from memory into an SDL surface.
 *
 * @param data Raw image data
 * @return Surface containing decoded image, or error message
 */
[[nodiscard]] SDLPP_EXPORT expected<surface, std::string> load(std::span<const std::uint8_t> data);

/**
 * @brief Load an image from file directly into an SDL texture.
 *
 * More efficient than load() + create_texture_from_surface() as it
 * can skip intermediate conversions.
 *
 * @param renderer Renderer to create texture with
 * @param path Path to image file
 * @return Texture containing decoded image, or error message
 */
[[nodiscard]] SDLPP_EXPORT expected<texture, std::string> load_texture(
    renderer& renderer,
    const std::filesystem::path& path);

/**
 * @brief Load an image from memory directly into an SDL texture.
 *
 * @param renderer Renderer to create texture with
 * @param data Raw image data
 * @return Texture containing decoded image, or error message
 */
[[nodiscard]] SDLPP_EXPORT expected<texture, std::string> load_texture(
    renderer& renderer,
    std::span<const std::uint8_t> data);

/**
 * @brief Decode image using a specific codec.
 *
 * @param data Raw image data
 * @param codec_name Name of codec to use (e.g., "png", "koala", "neochrome")
 * @return Surface containing decoded image, or error message
 */
[[nodiscard]] SDLPP_EXPORT expected<surface, std::string> decode(
    std::span<const std::uint8_t> data,
    const std::string& codec_name);

/**
 * @brief Get list of supported image format names.
 * @return Vector of codec names
 */
[[nodiscard]] SDLPP_EXPORT std::vector<std::string> supported_formats();

/**
 * @brief Check if a specific format is supported.
 * @param codec_name Format name to check
 * @return true if format is supported
 */
[[nodiscard]] SDLPP_EXPORT bool is_format_supported(const std::string& codec_name);

// ============================================================================
// Multi-Image Container Support (ICO, DCX, EXE icons, etc.)
// ============================================================================

/**
 * @brief Metadata for a sub-image within a container.
 */
struct image_region {
    rect<int> bounds;           ///< Position and size within the atlas
    enum class kind { sprite, tile, frame } type = kind::sprite;
    std::uint32_t index = 0;    ///< Original index in container
};

/**
 * @brief Result of loading a multi-image container.
 *
 * Contains the atlas surface and metadata for each sub-image.
 */
struct image_atlas {
    surface atlas;                          ///< Combined surface with all images
    std::vector<image_region> regions;      ///< Regions for each sub-image

    /**
     * @brief Get number of images in the container.
     */
    [[nodiscard]] std::size_t count() const noexcept { return regions.size(); }

    /**
     * @brief Check if this is a multi-image container.
     */
    [[nodiscard]] bool is_multi_image() const noexcept { return regions.size() > 1; }

    /**
     * @brief Extract a single image from the atlas.
     * @param index Image index (0-based)
     * @return Extracted surface, or error if index out of range
     */
    [[nodiscard]] SDLPP_EXPORT expected<surface, std::string> extract(std::size_t index) const;

    /**
     * @brief Find the best icon size for a target dimension.
     *
     * Useful for ICO files - finds the icon closest to the requested size.
     *
     * @param target_size Desired icon size (width or height)
     * @return Index of best matching image, or nullopt if empty
     */
    [[nodiscard]] std::optional<std::size_t> find_best_size(int target_size) const;
};

/**
 * @brief Load a multi-image container (ICO, DCX, EXE).
 *
 * Returns an atlas with all images and their regions.
 *
 * @param path Path to image file
 * @return Atlas with all images, or error message
 *
 * @code
 * auto result = sdlpp::image::load_atlas("app.ico");
 * if (result) {
 *     // Find 32x32 icon
 *     if (auto idx = result->find_best_size(32)) {
 *         auto icon = result->extract(*idx);
 *         // Use 32x32 icon...
 *     }
 * }
 * @endcode
 */
[[nodiscard]] SDLPP_EXPORT expected<image_atlas, std::string> load_atlas(const std::filesystem::path& path);

/**
 * @brief Load a multi-image container from memory.
 * @param data Raw image data
 * @return Atlas with all images, or error message
 */
[[nodiscard]] SDLPP_EXPORT expected<image_atlas, std::string> load_atlas(std::span<const std::uint8_t> data);

} // namespace sdlpp::image
