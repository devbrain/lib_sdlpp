#pragma once

/**
 * @file pen.hh
 * @brief Pen/stylus input functionality
 * 
 * This header provides C++ wrappers around SDL3's pen API, offering
 * support for graphics tablets, styluses, and other pen input devices.
 * 
 * Note: SDL3's pen API is primarily event-driven. Most pen information
 * comes through pen events rather than query functions.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/input/input_constants.hh>
#include <sdlpp/input/pen_types.hh>

#include <SDL3/SDL_pen.h>

#include <cstdint>
#include <string>
#include <vector>

namespace sdlpp {
    /**
     * @brief Special mouse ID for pen events
     */
    constexpr SDL_MouseID pen_mouse_id = input_constants::pen_as_mouse;

    /**
     * @brief Special touch ID for pen events
     */
    constexpr SDL_TouchID pen_touch_id = input_constants::pen_as_touch;

    /**
     * @brief Pen device type (SDL 3.4.0+)
     *
     * Indicates whether a pen operates on a screen directly (like a Wacom Cintiq)
     * or on a separate touchpad (like a Wacom Intuos tablet).
     */
    enum class pen_device_type : int {
        invalid = SDL_PEN_DEVICE_TYPE_INVALID,   ///< Not a valid pen device
        unknown = SDL_PEN_DEVICE_TYPE_UNKNOWN,   ///< Unknown pen type
        direct = SDL_PEN_DEVICE_TYPE_DIRECT,     ///< Pen touches display directly
        indirect = SDL_PEN_DEVICE_TYPE_INDIRECT  ///< Pen touches separate surface
    };

    /**
     * @brief Get the type of a pen device (SDL 3.4.0+)
     *
     * @param instance_id The pen instance ID
     * @return The pen device type
     */
    [[nodiscard]] inline pen_device_type get_pen_device_type(SDL_PenID instance_id) {
        return static_cast<pen_device_type>(SDL_GetPenDeviceType(instance_id));
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for pen_input_flags
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, pen_input_flags value);

/**
 * @brief Stream input operator for pen_input_flags
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, pen_input_flags& value);

/**
 * @brief Stream output operator for pen_axis
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, pen_axis value);

/**
 * @brief Stream input operator for pen_axis
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, pen_axis& value);

}