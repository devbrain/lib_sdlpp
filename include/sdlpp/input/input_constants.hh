#pragma once

/**
 * @file input_constants.hh
 * @brief Centralized input system constants
 * 
 * This header centralizes special SDL input constants that are shared
 * across multiple input subsystems (pen, mouse, touch).
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/compiler.hh>
#include <sdlpp/input/input_id_types.hh>

namespace sdlpp {
    /**
     * @brief Special input device ID constants
     * 
     * These constants represent special device IDs used by SDL for
     * cross-device input simulation (e.g., touch simulating mouse).
     */
    namespace input_constants {
// Disable old-style-cast warnings for SDL constants
#if defined(SDLPP_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4996) // For C-style casts
#elif defined(SDLPP_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(SDLPP_COMPILER_CLANG) || defined(SDLPP_COMPILER_WASM)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

        /**
         * @brief Mouse ID used when touch events simulate mouse input
         */
        inline constexpr SDL_MouseID touch_as_mouse = SDL_TOUCH_MOUSEID;
        
        /**
         * @brief Mouse ID used when pen events simulate mouse input
         */
        inline constexpr SDL_MouseID pen_as_mouse = SDL_PEN_MOUSEID;
        
        /**
         * @brief Pen ID used when mouse events simulate pen input
         */
        inline constexpr pen_id mouse_as_pen = SDL_PEN_MOUSEID;
        
        /**
         * @brief Pen ID used when touch events simulate pen input
         */
        inline constexpr pen_id touch_as_pen = static_cast<pen_id>(SDL_PEN_TOUCHID);
        
        /**
         * @brief Touch ID used when pen events simulate touch input
         */
        inline constexpr touch_id pen_as_touch = SDL_PEN_TOUCHID;

// Re-enable warnings
#if defined(SDLPP_COMPILER_MSVC)
#pragma warning(pop)
#elif defined(SDLPP_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(SDLPP_COMPILER_CLANG) || defined(SDLPP_COMPILER_WASM)
#pragma clang diagnostic pop
#endif
    } // namespace input_constants
} // namespace sdlpp