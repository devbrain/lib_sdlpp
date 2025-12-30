//
// SDL surface adapter for onyx_image
//

#pragma once

#include <sdlpp/video/surface.hh>
#include <onyx_image/surface.hpp>
#include <onyx_image/types.hpp>

namespace sdlpp::image {

/**
 * @brief Adapter that allows onyx_image decoders to write into sdlpp::surface.
 *
 * Usage:
 * @code
 * sdlpp::surface surf;
 * sdlpp::image::sdl_surface_adapter adapter(surf);
 * onyx_image::decode(data, adapter);
 * // surf now contains the decoded image
 * @endcode
 */
class sdl_surface_adapter : public onyx_image::surface {
public:
    /**
     * @brief Construct adapter for an existing surface reference.
     * @param target Reference to sdlpp::surface that will receive decoded pixels
     * @note The surface will be reallocated by set_size() to match image dimensions
     */
    explicit sdl_surface_adapter(sdlpp::surface& target);

    // onyx_image::surface interface
    bool set_size(int width, int height, onyx_image::pixel_format format) override;
    void write_pixels(int x, int y, int count, const std::uint8_t* pixels) override;
    void write_pixel(int x, int y, std::uint8_t pixel) override;
    void set_palette_size(int count) override;
    void write_palette(int start, std::span<const std::uint8_t> colors) override;
    void set_subrect(int index, const onyx_image::subrect& sr) override;

    /**
     * @brief Get collected subrects (for multi-image containers).
     */
    [[nodiscard]] const std::vector<onyx_image::subrect>& subrects() const noexcept {
        return m_subrects;
    }

private:
    sdlpp::surface& m_target;
    sdlpp::palette m_palette;
    std::vector<onyx_image::subrect> m_subrects;

    // Convert onyx_image pixel_format to SDL pixel format
    static pixel_format_enum to_sdl_format(onyx_image::pixel_format fmt);
};

/**
 * @brief Convert onyx_image pixel format to SDL pixel format.
 */
[[nodiscard]] pixel_format_enum to_sdl_format(onyx_image::pixel_format fmt);

} // namespace sdlpp::image
