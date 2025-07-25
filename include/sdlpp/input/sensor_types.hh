#pragma once

/**
 * @file sensor_types.hh
 * @brief Sensor type definitions
 * 
 * This header defines sensor-related types used by sensor and gamepad subsystems.
 */

#include <sdlpp/core/sdl.hh>

namespace sdlpp {
    /**
     * @brief Types of sensors
     */
    enum class sensor_type {
        invalid = SDL_SENSOR_INVALID,        ///< Invalid sensor
        unknown = SDL_SENSOR_UNKNOWN,        ///< Unknown sensor type
        accel = SDL_SENSOR_ACCEL,           ///< Accelerometer
        gyro = SDL_SENSOR_GYRO,             ///< Gyroscope
        accel_l = SDL_SENSOR_ACCEL_L,       ///< Left accelerometer (for gamepads)
        gyro_l = SDL_SENSOR_GYRO_L,         ///< Left gyroscope (for gamepads)
        accel_r = SDL_SENSOR_ACCEL_R,       ///< Right accelerometer (for gamepads)
        gyro_r = SDL_SENSOR_GYRO_R          ///< Right gyroscope (for gamepads)
    };
} // namespace sdlpp

// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for sensor_type
 */
std::ostream& operator<<(std::ostream& os, sensor_type value);

/**
 * @brief Stream input operator for sensor_type
 */
std::istream& operator>>(std::istream& is, sensor_type& value);

}