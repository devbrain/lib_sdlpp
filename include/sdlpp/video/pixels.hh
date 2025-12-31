//
// Created by igor on 7/13/25.
//

#pragma once

/**
 * @file pixels.hh
 * @brief C++ wrapper for SDL3 pixel format and color manipulation functions
 * 
 * This header provides modern C++ wrappers around SDL3's pixel format system,
 * including color representation, pixel format conversion, and palette management.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/core/error.hh>
#include <string>

namespace sdlpp {
    // Note: color and fcolor are now defined in color.hh
    // They provide the same interface plus many additional features

    // Legacy conversion functions for backward compatibility
    // These maintain the old API but are deprecated

    [[deprecated("Use fcolor(color_instance) constructor instead")]]
    inline fcolor from_color(const color& c) {
        return fcolor(c);
    }

    [[deprecated("Use color(fcolor_instance) constructor instead")]]
    inline color to_color(const fcolor& fc) {
        return color(fc);
    }

    /**
     * @brief Strongly-typed enumeration of SDL pixel formats
     * 
     * This enum provides type-safe access to SDL's pixel format constants.
     * Formats define how color data is stored in memory, including bit depth,
     * color component order, and encoding method.
     */
    enum class pixel_format_enum : Uint32 {
        unknown = SDL_PIXELFORMAT_UNKNOWN, ///< Unknown pixel format
        INDEX1LSB = SDL_PIXELFORMAT_INDEX1LSB, ///< 1-bit indexed, LSB first
        INDEX1MSB = SDL_PIXELFORMAT_INDEX1MSB, ///< 1-bit indexed, MSB first
        INDEX4LSB = SDL_PIXELFORMAT_INDEX4LSB, ///< 4-bit indexed, LSB first
        INDEX4MSB = SDL_PIXELFORMAT_INDEX4MSB, ///< 4-bit indexed, MSB first
        INDEX8 = SDL_PIXELFORMAT_INDEX8, ///< 8-bit indexed format
        RGB332 = SDL_PIXELFORMAT_RGB332, ///< 8-bit RGB (3-3-2)
        RGB444 = SDL_PIXELFORMAT_XRGB4444, ///< 16-bit RGB (4-4-4)
        RGB555 = SDL_PIXELFORMAT_XRGB1555, ///< 16-bit RGB (5-5-5)
        BGR555 = SDL_PIXELFORMAT_XBGR1555, ///< 16-bit BGR (5-5-5)
        ARGB4444 = SDL_PIXELFORMAT_ARGB4444, ///< 16-bit ARGB (4-4-4-4)
        RGBA4444 = SDL_PIXELFORMAT_RGBA4444, ///< 16-bit RGBA (4-4-4-4)
        ABGR4444 = SDL_PIXELFORMAT_ABGR4444, ///< 16-bit ABGR (4-4-4-4)
        BGRA4444 = SDL_PIXELFORMAT_BGRA4444, ///< 16-bit BGRA (4-4-4-4)
        ARGB1555 = SDL_PIXELFORMAT_ARGB1555, ///< 16-bit ARGB (1-5-5-5)
        RGBA5551 = SDL_PIXELFORMAT_RGBA5551, ///< 16-bit RGBA (5-5-5-1)
        ABGR1555 = SDL_PIXELFORMAT_ABGR1555, ///< 16-bit ABGR (1-5-5-5)
        BGRA5551 = SDL_PIXELFORMAT_BGRA5551, ///< 16-bit BGRA (5-5-5-1)
        RGB565 = SDL_PIXELFORMAT_RGB565, ///< 16-bit RGB (5-6-5)
        BGR565 = SDL_PIXELFORMAT_BGR565, ///< 16-bit BGR (5-6-5)
        RGB24 = SDL_PIXELFORMAT_RGB24, ///< 24-bit RGB (8-8-8)
        BGR24 = SDL_PIXELFORMAT_BGR24, ///< 24-bit BGR (8-8-8)
        RGB888 = SDL_PIXELFORMAT_XRGB8888, ///< 32-bit RGB (8-8-8)
        RGBX8888 = SDL_PIXELFORMAT_RGBX8888, ///< 32-bit RGBX (8-8-8-X)
        BGR888 = SDL_PIXELFORMAT_XBGR8888, ///< 32-bit BGR (8-8-8)
        BGRX8888 = SDL_PIXELFORMAT_BGRX8888, ///< 32-bit BGRX (8-8-8-X)
        ARGB8888 = SDL_PIXELFORMAT_ARGB8888, ///< 32-bit ARGB (8-8-8-8)
        RGBA8888 = SDL_PIXELFORMAT_RGBA8888, ///< 32-bit RGBA (8-8-8-8)
        ABGR8888 = SDL_PIXELFORMAT_ABGR8888, ///< 32-bit ABGR (8-8-8-8)
        BGRA8888 = SDL_PIXELFORMAT_BGRA8888, ///< 32-bit BGRA (8-8-8-8)
        ARGB2101010 = SDL_PIXELFORMAT_ARGB2101010, ///< 32-bit ARGB (2-10-10-10)
    };

    /**
     * @brief Wrapper for SDL pixel format operations
     * 
     * This struct provides RAII management and convenient methods for working
     * with SDL pixel formats. It handles format details retrieval and provides
     * methods for mapping colors to pixel values and vice versa.
     */
    struct pixel_format {
        SDL_PixelFormat format; ///< The SDL pixel format enum value
        const SDL_PixelFormatDetails* details; ///< Cached pixel format details

        /**
         * @brief Construct a pixel_format wrapper
         * @param format The pixel format to use
         * @throws std::runtime_error if the format is invalid or unsupported
         */
        explicit pixel_format(pixel_format_enum fmt)
            : format(static_cast <SDL_PixelFormat>(fmt)) {
            details = SDL_GetPixelFormatDetails(this->format);
            if (!details) {
                throw std::runtime_error(get_error());
            }
        }

        /**
         * @brief Map RGB color components to a pixel value
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @return Pixel value in the format's encoding
         */
        [[nodiscard]] Uint32 map_rgb(Uint8 r, Uint8 g, Uint8 b) const {
            return SDL_MapRGB(details, nullptr, r, g, b);
        }

        /**
         * @brief Map RGBA color components to a pixel value
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @param a Alpha component (0-255)
         * @return Pixel value in the format's encoding
         */
        [[nodiscard]] Uint32 map_rgba(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const {
            return SDL_MapRGBA(details, nullptr, r, g, b, a);
        }

        /**
         * @brief Map a color struct to a pixel value (RGB only)
         * @param c Color to map
         * @return Pixel value in the format's encoding
         */
        [[nodiscard]] Uint32 map_rgb(const color& c) const {
            return map_rgb(c.r, c.g, c.b);
        }

        /**
         * @brief Map a color struct to a pixel value (RGBA)
         * @param c Color to map
         * @return Pixel value in the format's encoding
         */
        [[nodiscard]] Uint32 map_rgba(const color& c) const {
            return map_rgba(c.r, c.g, c.b, c.a);
        }

        /**
         * @brief Get RGBA color components from a pixel value
         * @param pixel Pixel value in the format's encoding
         * @return color struct with extracted components
         */
        [[nodiscard]] color get_rgba(Uint32 pixel) const {
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, details, nullptr, &r, &g, &b, &a);
            return {r, g, b, a};
        }
    };

    /**
     * @brief Smart pointer type for SDL_Palette with automatic cleanup
     */
    using palette_ptr = pointer <SDL_Palette, SDL_DestroyPalette>;

    /**
     * @brief Create a new palette with the specified number of colors
     * 
     * Creates a palette structure that can hold color entries. The palette
     * entries are initialized to white.
     * 
     * @param ncolors Number of color entries in the palette
     * @return Expected containing the palette pointer on success, or error message on failure
     */
    inline expected <palette_ptr, std::string> make_palette(int ncolors) {
        SDL_Palette* raw = SDL_CreatePalette(ncolors);
        if (!raw) {
            return make_unexpectedf(get_error());
        }
        return palette_ptr(raw);
    }

    /**
     * @brief Convert pixels from one format to another
     * 
     * Converts a block of pixels from one pixel format to another. This function
     * handles format conversion, byte order swapping, and bit depth changes.
     * 
     * @param w Width of the pixel block in pixels
     * @param h Height of the pixel block in pixels
     * @param src_format Source pixel format
     * @param src Pointer to source pixel data
     * @param src_pitch Bytes per row in source data (including padding)
     * @param dst_format Destination pixel format
     * @param dst Pointer to destination buffer
     * @param dst_pitch Bytes per row in destination buffer
     * @return Expected<void> - empty on success, error message on failure
     * 
     * @note The destination buffer must be pre-allocated with sufficient space
     * @note This function is thread-safe for different src/dst buffers
     */
    inline expected <void, std::string>
    convert_pixels(int w, int h,
                   SDL_PixelFormat src_format, const void* src, int src_pitch,
                   SDL_PixelFormat dst_format, void* dst, int dst_pitch) {
        if (!SDL_ConvertPixels(w, h, src_format, src, src_pitch,
                               dst_format, dst, dst_pitch)) {
            return make_unexpectedf(get_error());
        }
        return {};
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for pixel_format_enum
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, pixel_format_enum value);

/**
 * @brief Stream input operator for pixel_format_enum
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, pixel_format_enum& value);

}