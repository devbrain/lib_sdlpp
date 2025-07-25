#pragma once

/**
 * @file id_types.hh
 * @brief Common ID type definitions used across SDL++
 * 
 * This header defines ID types that are used by multiple subsystems
 * to avoid circular dependencies.
 */

#include <sdlpp/core/sdl.hh>
#include <cstdint>

namespace sdlpp {
    /**
     * @brief Window ID type
     * 
     * Identifies a specific window in the system.
     */
    using window_id = SDL_WindowID;
    
    /**
     * @brief Display ID type
     * 
     * Identifies a specific display/monitor in the system.
     */
    using display_id = SDL_DisplayID;
    
    /**
     * @brief Audio device ID type
     * 
     * Identifies a specific audio device.
     * @note This is a simple type alias. For the opaque audio_device_id class,
     *       see audio/audio.hh
     */
    using audio_device_id_raw = SDL_AudioDeviceID;
    
    /**
     * @brief Camera ID type
     * 
     * Identifies a specific camera device.
     */
    using camera_id = SDL_CameraID;
    
    /**
     * @brief Sensor ID type
     * 
     * Identifies a specific sensor device.
     */
    using sensor_id = SDL_SensorID;
} // namespace sdlpp