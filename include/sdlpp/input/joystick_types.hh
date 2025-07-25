#pragma once

/**
 * @file joystick_types.hh
 * @brief Joystick-related type definitions
 * 
 * This header defines types specific to joystick devices.
 */

#include <sdlpp/core/sdl.hh>
#include <cstdint>

namespace sdlpp {
    /**
     * @brief Joystick hat positions
     * 
     * These are bitmask values that can be combined.
     */
    enum class joystick_hat : Uint8 {
        centered = SDL_HAT_CENTERED,     ///< Hat is centered
        up = SDL_HAT_UP,                 ///< Hat is pushed up
        right = SDL_HAT_RIGHT,           ///< Hat is pushed right
        down = SDL_HAT_DOWN,             ///< Hat is pushed down
        left = SDL_HAT_LEFT,             ///< Hat is pushed left
        rightup = SDL_HAT_RIGHTUP,       ///< Hat is pushed right and up
        rightdown = SDL_HAT_RIGHTDOWN,   ///< Hat is pushed right and down
        leftup = SDL_HAT_LEFTUP,         ///< Hat is pushed left and up
        leftdown = SDL_HAT_LEFTDOWN      ///< Hat is pushed left and down
    };
    
    /**
     * @brief Check if hat is centered
     */
    [[nodiscard]] inline constexpr bool is_hat_centered(joystick_hat hat) noexcept {
        return hat == joystick_hat::centered;
    }
    
    /**
     * @brief Check if hat has up direction
     */
    [[nodiscard]] inline constexpr bool has_hat_up(joystick_hat hat) noexcept {
        return (static_cast<Uint8>(hat) & SDL_HAT_UP) != 0;
    }
    
    /**
     * @brief Check if hat has down direction
     */
    [[nodiscard]] inline constexpr bool has_hat_down(joystick_hat hat) noexcept {
        return (static_cast<Uint8>(hat) & SDL_HAT_DOWN) != 0;
    }
    
    /**
     * @brief Check if hat has left direction
     */
    [[nodiscard]] inline constexpr bool has_hat_left(joystick_hat hat) noexcept {
        return (static_cast<Uint8>(hat) & SDL_HAT_LEFT) != 0;
    }
    
    /**
     * @brief Check if hat has right direction
     */
    [[nodiscard]] inline constexpr bool has_hat_right(joystick_hat hat) noexcept {
        return (static_cast<Uint8>(hat) & SDL_HAT_RIGHT) != 0;
    }
} // namespace sdlpp

// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for joystick_hat
 */
std::ostream& operator<<(std::ostream& os, joystick_hat value);

/**
 * @brief Stream input operator for joystick_hat
 */
std::istream& operator>>(std::istream& is, joystick_hat& value);

}