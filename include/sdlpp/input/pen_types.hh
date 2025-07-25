#pragma once

/**
 * @file pen_types.hh
 * @brief Pen/stylus type definitions
 * 
 * This header defines pen-related types and flags.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/input/input_id_types.hh>
#include <sdlpp/input/input_constants.hh>
#include <cstdint>

namespace sdlpp {
    /**
     * @brief Pen input state flags
     * 
     * These flags indicate which pen inputs are currently active.
     */
    enum class pen_input_flags : Uint32 {
        none = 0,
        down = SDL_PEN_INPUT_DOWN,           ///< Pen is touching the surface
        button_1 = SDL_PEN_INPUT_BUTTON_1,   ///< Primary button pressed
        button_2 = SDL_PEN_INPUT_BUTTON_2,   ///< Secondary button pressed
        button_3 = SDL_PEN_INPUT_BUTTON_3,   ///< Third button pressed
        button_4 = SDL_PEN_INPUT_BUTTON_4,   ///< Fourth button pressed
        button_5 = SDL_PEN_INPUT_BUTTON_5,   ///< Fifth button pressed
        eraser_tip = SDL_PEN_INPUT_ERASER_TIP ///< Eraser tip is being used
    };
    
    /**
     * @brief Bitwise OR operator for pen input flags
     */
    [[nodiscard]] inline constexpr pen_input_flags operator|(pen_input_flags a, pen_input_flags b) noexcept {
        return static_cast<pen_input_flags>(
            static_cast<Uint32>(a) | static_cast<Uint32>(b)
        );
    }
    
    /**
     * @brief Bitwise AND operator for pen input flags
     */
    [[nodiscard]] inline constexpr pen_input_flags operator&(pen_input_flags a, pen_input_flags b) noexcept {
        return static_cast<pen_input_flags>(
            static_cast<Uint32>(a) & static_cast<Uint32>(b)
        );
    }
    
    /**
     * @brief Bitwise XOR operator for pen input flags
     */
    [[nodiscard]] inline constexpr pen_input_flags operator^(pen_input_flags a, pen_input_flags b) noexcept {
        return static_cast<pen_input_flags>(
            static_cast<Uint32>(a) ^ static_cast<Uint32>(b)
        );
    }
    
    /**
     * @brief Bitwise NOT operator for pen input flags
     */
    [[nodiscard]] inline constexpr pen_input_flags operator~(pen_input_flags a) noexcept {
        return static_cast<pen_input_flags>(
            ~static_cast<Uint32>(a)
        );
    }
    
    /**
     * @brief Compound OR assignment operator
     */
    inline constexpr pen_input_flags& operator|=(pen_input_flags& a, pen_input_flags b) noexcept {
        return a = a | b;
    }
    
    /**
     * @brief Compound AND assignment operator
     */
    inline constexpr pen_input_flags& operator&=(pen_input_flags& a, pen_input_flags b) noexcept {
        return a = a & b;
    }
    
    /**
     * @brief Compound XOR assignment operator
     */
    inline constexpr pen_input_flags& operator^=(pen_input_flags& a, pen_input_flags b) noexcept {
        return a = a ^ b;
    }
    
    /**
     * @brief Check if a flag is set
     */
    [[nodiscard]] inline constexpr bool has_flag(pen_input_flags flags, pen_input_flags flag) noexcept {
        return (flags & flag) == flag;
    }
    
    /**
     * @brief Pen axis types
     */
    enum class pen_axis : int {
        pressure = SDL_PEN_AXIS_PRESSURE,         ///< Pressure (0.0 to 1.0)
        xtilt = SDL_PEN_AXIS_XTILT,              ///< X-axis tilt angle
        ytilt = SDL_PEN_AXIS_YTILT,              ///< Y-axis tilt angle
        distance = SDL_PEN_AXIS_DISTANCE,         ///< Distance from surface
        rotation = SDL_PEN_AXIS_ROTATION,         ///< Rotation angle
        slider = SDL_PEN_AXIS_SLIDER,             ///< Slider position
        tangential_pressure = SDL_PEN_AXIS_TANGENTIAL_PRESSURE, ///< Tangential pressure
        
        count = SDL_PEN_AXIS_COUNT                ///< Number of pen axes
    };
    
    /**
     * @brief Special pen ID constants
     */
    namespace pen_constants {
        /**
         * @brief Pen ID for mouse events simulating pen input
         */
        inline constexpr pen_id mouse_id = input_constants::mouse_as_pen;
        
        /**
         * @brief Pen ID for touch events simulating pen input
         */
        inline constexpr pen_id touch_id = input_constants::touch_as_pen;
    }
} // namespace sdlpp

// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for pen_input_flags
 */
std::ostream& operator<<(std::ostream& os, pen_input_flags value);

/**
 * @brief Stream input operator for pen_input_flags
 */
std::istream& operator>>(std::istream& is, pen_input_flags& value);

/**
 * @brief Stream output operator for pen_axis
 */
std::ostream& operator<<(std::ostream& os, pen_axis value);

/**
 * @brief Stream input operator for pen_axis
 */
std::istream& operator>>(std::istream& is, pen_axis& value);

}