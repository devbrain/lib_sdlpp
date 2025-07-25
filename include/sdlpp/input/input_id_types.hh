#pragma once

/**
 * @file input_id_types.hh
 * @brief Input device ID type definitions
 * 
 * This header defines ID types for input devices (keyboard, mouse, joystick, etc.)
 * to avoid circular dependencies between input and event modules.
 */

#include <sdlpp/core/sdl.hh>
#include <cstdint>

namespace sdlpp {
    /**
     * @brief Keyboard ID type
     * 
     * Identifies a specific keyboard device.
     */
    using keyboard_id = SDL_KeyboardID;
    
    /**
     * @brief Mouse ID type
     * 
     * Identifies a specific mouse device.
     */
    using mouse_id = SDL_MouseID;
    
    /**
     * @brief Joystick ID type
     * 
     * Identifies a specific joystick device.
     * Also used for gamepads, which are specialized joysticks.
     */
    using joystick_id = SDL_JoystickID;
    
    /**
     * @brief Touch device ID type
     * 
     * Identifies a specific touch input device.
     */
    using touch_id = SDL_TouchID;
    
    /**
     * @brief Touch finger ID type
     * 
     * Identifies a specific finger on a touch device.
     */
    using finger_id = SDL_FingerID;
    
    /**
     * @brief Pen/stylus ID type
     * 
     * Identifies a specific pen or stylus device.
     */
    using pen_id = SDL_PenID;
} // namespace sdlpp