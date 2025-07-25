#pragma once

/**
 * @file gamepad_types.hh
 * @brief Gamepad-specific type definitions
 * 
 * This header defines types specific to gamepad controllers.
 */

#include <sdlpp/core/sdl.hh>
#include <cstdint>

namespace sdlpp {
    /**
     * @brief Standard gamepad axis types
     */
    enum class gamepad_axis : Uint8 {
        leftx = SDL_GAMEPAD_AXIS_LEFTX,           ///< Left stick X axis
        lefty = SDL_GAMEPAD_AXIS_LEFTY,           ///< Left stick Y axis
        rightx = SDL_GAMEPAD_AXIS_RIGHTX,         ///< Right stick X axis
        righty = SDL_GAMEPAD_AXIS_RIGHTY,         ///< Right stick Y axis
        left_trigger = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,   ///< Left trigger
        right_trigger = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER  ///< Right trigger
    };
    
    /**
     * @brief Standard gamepad button types
     */
    enum class gamepad_button : Uint8 {
        south = SDL_GAMEPAD_BUTTON_SOUTH,         ///< Bottom face button (A/Cross)
        east = SDL_GAMEPAD_BUTTON_EAST,           ///< Right face button (B/Circle)
        west = SDL_GAMEPAD_BUTTON_WEST,           ///< Left face button (X/Square)
        north = SDL_GAMEPAD_BUTTON_NORTH,         ///< Top face button (Y/Triangle)
        back = SDL_GAMEPAD_BUTTON_BACK,           ///< Back/Select button
        guide = SDL_GAMEPAD_BUTTON_GUIDE,         ///< Guide/Home button
        start = SDL_GAMEPAD_BUTTON_START,         ///< Start button
        left_stick = SDL_GAMEPAD_BUTTON_LEFT_STICK,     ///< Left stick click
        right_stick = SDL_GAMEPAD_BUTTON_RIGHT_STICK,   ///< Right stick click
        left_shoulder = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,   ///< Left shoulder button
        right_shoulder = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, ///< Right shoulder button
        dpad_up = SDL_GAMEPAD_BUTTON_DPAD_UP,     ///< D-pad up
        dpad_down = SDL_GAMEPAD_BUTTON_DPAD_DOWN, ///< D-pad down
        dpad_left = SDL_GAMEPAD_BUTTON_DPAD_LEFT, ///< D-pad left
        dpad_right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT, ///< D-pad right
        misc1 = SDL_GAMEPAD_BUTTON_MISC1,         ///< Additional button 1
        right_paddle1 = SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1, ///< Right paddle 1
        left_paddle1 = SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,   ///< Left paddle 1
        right_paddle2 = SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2, ///< Right paddle 2
        left_paddle2 = SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,   ///< Left paddle 2
        touchpad = SDL_GAMEPAD_BUTTON_TOUCHPAD,   ///< Touchpad click
        misc2 = SDL_GAMEPAD_BUTTON_MISC2,         ///< Additional button 2
        misc3 = SDL_GAMEPAD_BUTTON_MISC3,         ///< Additional button 3
        misc4 = SDL_GAMEPAD_BUTTON_MISC4,         ///< Additional button 4
        misc5 = SDL_GAMEPAD_BUTTON_MISC5,         ///< Additional button 5
        misc6 = SDL_GAMEPAD_BUTTON_MISC6          ///< Additional button 6
    };
    
    /**
     * @brief Gamepad types/vendors
     */
    enum class gamepad_type {
        unknown = SDL_GAMEPAD_TYPE_UNKNOWN,
        standard = SDL_GAMEPAD_TYPE_STANDARD,
        xbox360 = SDL_GAMEPAD_TYPE_XBOX360,
        xboxone = SDL_GAMEPAD_TYPE_XBOXONE,
        ps3 = SDL_GAMEPAD_TYPE_PS3,
        ps4 = SDL_GAMEPAD_TYPE_PS4,
        ps5 = SDL_GAMEPAD_TYPE_PS5,
        nintendo_switch_pro = SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO,
        nintendo_switch_joycon_left = SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT,
        nintendo_switch_joycon_right = SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT,
        nintendo_switch_joycon_pair = SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR
    };
} // namespace sdlpp

// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for gamepad_axis
 */
std::ostream& operator<<(std::ostream& os, gamepad_axis value);

/**
 * @brief Stream input operator for gamepad_axis
 */
std::istream& operator>>(std::istream& is, gamepad_axis& value);

/**
 * @brief Stream output operator for gamepad_button
 */
std::ostream& operator<<(std::ostream& os, gamepad_button value);

/**
 * @brief Stream input operator for gamepad_button
 */
std::istream& operator>>(std::istream& is, gamepad_button& value);

/**
 * @brief Stream output operator for gamepad_type
 */
std::ostream& operator<<(std::ostream& os, gamepad_type value);

/**
 * @brief Stream input operator for gamepad_type
 */
std::istream& operator>>(std::istream& is, gamepad_type& value);

}