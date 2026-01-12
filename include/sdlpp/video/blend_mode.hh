//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file blend_mode.hh
 * @brief Blend mode definitions shared across SDL3 rendering components
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>

namespace sdlpp {
    /**
     * @brief Blend modes for rendering operations
     * 
     * These blend modes control how colors are combined during rendering
     * operations. They are used by surfaces, textures, and renderers.
     */
    enum class blend_mode : int {
        none = SDL_BLENDMODE_NONE, ///< No blending: dstRGBA = srcRGBA
        blend = SDL_BLENDMODE_BLEND,
        ///< Alpha blending: dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA)), dstA = srcA + (dstA * (1-srcA))
        blend_premultiplied = SDL_BLENDMODE_BLEND_PREMULTIPLIED, ///< Pre-multiplied alpha blending
        add = SDL_BLENDMODE_ADD, ///< Additive blending: dstRGB = (srcRGB * srcA) + dstRGB, dstA = dstA
        add_premultiplied = SDL_BLENDMODE_ADD_PREMULTIPLIED, ///< Pre-multiplied additive
        mod = SDL_BLENDMODE_MOD, ///< Color modulation: dstRGB = srcRGB * dstRGB, dstA = dstA
        mul = SDL_BLENDMODE_MUL,
        ///< Color multiplication: dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA)), dstA = dstA
        invalid = SDL_BLENDMODE_INVALID ///< Invalid blend mode
    };

    /**
     * @brief Scale modes for rendering operations
     */
    enum class scale_mode : int {
        nearest = SDL_SCALEMODE_NEAREST, ///< Nearest neighbor scaling (pixelated)
        linear = SDL_SCALEMODE_LINEAR, ///< Linear filtering (smooth)
        pixelart = SDL_SCALEMODE_PIXELART ///< Pixel art optimized scaling (SDL 3.4.0+)
    };

    /**
     * @brief Flip modes for rendering operations
     */
    enum class flip_mode : uint32_t {
        none = SDL_FLIP_NONE, ///< No flipping
        horizontal = SDL_FLIP_HORIZONTAL, ///< Flip horizontally
        vertical = SDL_FLIP_VERTICAL ///< Flip vertically
    };

    /**
     * @brief Enable bitwise operations for flip_mode
     */
    constexpr flip_mode operator|(flip_mode a, flip_mode b) {
        return static_cast <flip_mode>(
            static_cast <uint32_t>(a) | static_cast <uint32_t>(b)
        );
    }

    constexpr flip_mode operator&(flip_mode a, flip_mode b) {
        return static_cast <flip_mode>(
            static_cast <uint32_t>(a) & static_cast <uint32_t>(b)
        );
    }

    constexpr flip_mode& operator|=(flip_mode& a, flip_mode b) {
        a = a | b;
        return a;
    }

    constexpr flip_mode& operator&=(flip_mode& a, flip_mode b) {
        a = a & b;
        return a;
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for blend_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, blend_mode value);

/**
 * @brief Stream input operator for blend_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, blend_mode& value);

/**
 * @brief Stream output operator for scale_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, scale_mode value);

/**
 * @brief Stream input operator for scale_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, scale_mode& value);

/**
 * @brief Stream output operator for flip_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, flip_mode value);

/**
 * @brief Stream input operator for flip_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, flip_mode& value);

}