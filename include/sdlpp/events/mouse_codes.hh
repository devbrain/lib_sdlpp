#pragma once

/**
 * @file mouse_codes.hh
 * @brief Mouse button and wheel definitions
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>

namespace sdlpp {
    /**
     * @brief Mouse button enumeration
     */
    enum class mouse_button : Uint8 {
        left = SDL_BUTTON_LEFT,
        middle = SDL_BUTTON_MIDDLE,
        right = SDL_BUTTON_RIGHT,
        x1 = SDL_BUTTON_X1,
        x2 = SDL_BUTTON_X2
    };

    /**
     * @brief Mouse button mask flags
     */
    enum class mouse_button_mask : Uint32 {
        none = 0,
        left = SDL_BUTTON_LMASK,
        middle = SDL_BUTTON_MMASK,
        right = SDL_BUTTON_RMASK,
        x1 = SDL_BUTTON_X1MASK,
        x2 = SDL_BUTTON_X2MASK
    };

    /**
     * @brief Mouse wheel direction
     */
    enum class mouse_wheel_direction : Uint32 {
        normal = SDL_MOUSEWHEEL_NORMAL,
        flipped = SDL_MOUSEWHEEL_FLIPPED
    };

    /**
     * @brief Bitwise OR operator for mouse button mask
     */
    [[nodiscard]] inline mouse_button_mask operator|(mouse_button_mask lhs, mouse_button_mask rhs) noexcept {
        return static_cast <mouse_button_mask>(static_cast <Uint32>(lhs) | static_cast <Uint32>(rhs));
    }

    /**
     * @brief Bitwise AND operator for mouse button mask
     */
    [[nodiscard]] inline mouse_button_mask operator&(mouse_button_mask lhs, mouse_button_mask rhs) noexcept {
        return static_cast <mouse_button_mask>(static_cast <Uint32>(lhs) & static_cast <Uint32>(rhs));
    }

    /**
     * @brief Bitwise XOR operator for mouse button mask
     */
    [[nodiscard]] inline mouse_button_mask operator^(mouse_button_mask lhs, mouse_button_mask rhs) noexcept {
        return static_cast <mouse_button_mask>(static_cast <Uint32>(lhs) ^ static_cast <Uint32>(rhs));
    }

    /**
     * @brief Bitwise NOT operator for mouse button mask
     */
    [[nodiscard]] inline mouse_button_mask operator~(mouse_button_mask mask) noexcept {
        return static_cast <mouse_button_mask>(~static_cast <Uint32>(mask));
    }

    /**
     * @brief Check if button is in mask
     */
    [[nodiscard]] inline bool has_button(mouse_button_mask mask, mouse_button_mask check) noexcept {
        return (mask & check) != mouse_button_mask::none;
    }

    /**
     * @brief Convert button to mask
     */
    [[nodiscard]] inline mouse_button_mask button_to_mask(mouse_button button) noexcept {
        return static_cast <mouse_button_mask>(1u << (static_cast <int>(button) - 1));
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for mouse_button
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, mouse_button value);

/**
 * @brief Stream input operator for mouse_button
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, mouse_button& value);

/**
 * @brief Stream output operator for mouse_button_mask
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, mouse_button_mask value);

/**
 * @brief Stream input operator for mouse_button_mask
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, mouse_button_mask& value);

/**
 * @brief Stream output operator for mouse_wheel_direction
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, mouse_wheel_direction value);

/**
 * @brief Stream input operator for mouse_wheel_direction
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, mouse_wheel_direction& value);

}