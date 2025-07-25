#pragma once

/**
 * @file sensor.hh
 * @brief Sensor input functionality
 * 
 * This header provides C++ wrappers around SDL3's sensor API, offering
 * access to accelerometers, gyroscopes, and other sensors on various platforms.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/type_utils.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/input/sensor_types.hh>

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <cmath>

namespace sdlpp {
    /**
     * @brief Sensor instance ID type
     */
    using sensor_id = SDL_SensorID;

    /**
     * @brief Standard gravity constant for accelerometer readings
     *
     * The accelerometer returns the current acceleration in SI meters per second
     * squared. This measurement includes the force of gravity, so a device at
     * rest will have a value of SDL_STANDARD_GRAVITY away from the center of the
     * earth, which is a positive Y value.
     */
    constexpr float standard_gravity = SDL_STANDARD_GRAVITY;


    /**
     * @brief Smart pointer type for SDL_Sensor with automatic cleanup
     */
    using sensor_ptr = pointer <SDL_Sensor, SDL_CloseSensor>;

    /**
     * @brief Get list of available sensors
     *
     * @return Vector of sensor instance IDs
     */
    [[nodiscard]] inline std::vector <sensor_id> get_sensors() {
        int count = 0;
        const SDL_SensorID* sensors = SDL_GetSensors(&count);
        if (!sensors || count <= 0) {
            return {};
        }
        std::vector <sensor_id> sensor_ids(sensors, sensors + count);
        SDL_free(const_cast <SDL_SensorID*>(sensors));
        return sensor_ids;
    }

    /**
     * @brief Get the name of a sensor
     *
     * This can be called before the sensor is opened.
     *
     * @param instance_id The sensor instance ID
     * @return Sensor name, or empty string if invalid
     */
    [[nodiscard]] inline std::string get_sensor_name_for_id(sensor_id instance_id) {
        const char* name = SDL_GetSensorNameForID(instance_id);
        return name ? name : "";
    }

    /**
     * @brief Get the type of a sensor
     *
     * This can be called before the sensor is opened.
     *
     * @param instance_id The sensor instance ID
     * @return Sensor type
     */
    [[nodiscard]] inline sensor_type get_sensor_type_for_id(sensor_id instance_id) noexcept {
        return static_cast <sensor_type>(SDL_GetSensorTypeForID(instance_id));
    }

    /**
     * @brief Get the platform-dependent type of a sensor
     *
     * This can be called before the sensor is opened.
     *
     * @param instance_id The sensor instance ID
     * @return Platform-dependent type, or -1 if invalid
     */
    [[nodiscard]] inline int get_sensor_non_portable_type_for_id(sensor_id instance_id) noexcept {
        return SDL_GetSensorNonPortableTypeForID(instance_id);
    }

    /**
     * @brief RAII wrapper for SDL_Sensor
     *
     * This class provides a safe, RAII-managed interface to SDL's sensor functionality.
     * The sensor is automatically closed when the object goes out of scope.
     */
    class sensor {
        private:
            sensor_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty sensor
             */
            sensor() = default;

            /**
             * @brief Construct from existing SDL_Sensor pointer
             * @param s Existing SDL_Sensor pointer (takes ownership)
             */
            explicit sensor(SDL_Sensor* s)
                : ptr(s) {
            }

            /**
             * @brief Move constructor
             */
            sensor(sensor&&) = default;

            /**
             * @brief Move assignment operator
             */
            sensor& operator=(sensor&&) = default;

            // Delete copy operations - sensors are move-only
            sensor(const sensor&) = delete;
            sensor& operator=(const sensor&) = delete;

            /**
             * @brief Check if the sensor is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Sensor pointer
             */
            [[nodiscard]] SDL_Sensor* get() const noexcept { return ptr.get(); }

            /**
             * @brief Open a sensor for use
             *
             * @param instance_id The sensor instance ID
             * @return Expected containing sensor, or error message
             */
            static expected <sensor, std::string> open(sensor_id instance_id) {
                auto* s = SDL_OpenSensor(instance_id);
                if (!s) {
                    return make_unexpected(get_error());
                }
                return sensor(s);
            }

            /**
             * @brief Get the instance ID of this sensor
             *
             * @return Instance ID, or 0 if invalid
             */
            [[nodiscard]] sensor_id get_id() const noexcept {
                return ptr ? SDL_GetSensorID(ptr.get()) : 0;
            }

            /**
             * @brief Get the name of this sensor
             *
             * @return Sensor name, or empty string if invalid
             */
            [[nodiscard]] std::string get_name() const {
                if (!ptr) return "";
                const char* name = SDL_GetSensorName(ptr.get());
                return name ? name : "";
            }

            /**
             * @brief Get the type of this sensor
             *
             * @return Sensor type
             */
            [[nodiscard]] sensor_type get_type() const noexcept {
                return ptr
                           ? static_cast <sensor_type>(SDL_GetSensorType(ptr.get()))
                           : sensor_type::invalid;
            }

            /**
             * @brief Get the platform-dependent type of this sensor
             *
             * @return Platform-dependent type, or -1 if invalid
             */
            [[nodiscard]] int get_non_portable_type() const noexcept {
                return ptr ? SDL_GetSensorNonPortableType(ptr.get()) : -1;
            }

            /**
             * @brief Get the properties ID for this sensor
             *
             * @return Properties ID, or 0 if invalid
             */
            [[nodiscard]] SDL_PropertiesID get_properties() const noexcept {
                return ptr ? SDL_GetSensorProperties(ptr.get()) : 0;
            }

            /**
             * @brief Get sensor data
             *
             * The number of values and interpretation of the data is sensor dependent.
             *
             * @param data Array to receive sensor data
             * @param num_values Number of values to read
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> get_data(float* data, size_t num_values) const {
                if (!ptr) {
                    return make_unexpected("Invalid sensor");
                }
                auto int_values = detail::size_to_int(num_values);
                if (!int_values) {
                    return make_unexpected("Number of values too large: " + int_values.error());
                }
                if (!SDL_GetSensorData(ptr.get(), data, *int_values)) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Get sensor data (3 values - common for accel/gyro)
             *
             * @return Expected containing array of 3 float values, or error message
             */
            [[nodiscard]] expected <std::array <float, 3>, std::string> get_data_3() const {
                std::array <float, 3> data{};
                auto get_result = get_data(data.data(), 3);
                if (!get_result) {
                    return make_unexpected(get_result.error());
                }
                return data;
            }

            /**
             * @brief Get sensor data (6 values - for some platform-specific sensors)
             *
             * @return Expected containing array of 6 float values, or error message
             */
            [[nodiscard]] expected <std::array <float, 6>, std::string> get_data_6() const {
                std::array <float, 6> data{};
                auto get_result = get_data(data.data(), 6);
                if (!get_result) {
                    return make_unexpected(get_result.error());
                }
                return data;
            }

            /**
             * @brief Check if this is an accelerometer
             *
             * @return true if this is an accelerometer
             */
            [[nodiscard]] bool is_accelerometer() const noexcept {
                auto type = get_type();
                return type == sensor_type::accel ||
                       type == sensor_type::accel_l ||
                       type == sensor_type::accel_r;
            }

            /**
             * @brief Check if this is a gyroscope
             *
             * @return true if this is a gyroscope
             */
            [[nodiscard]] bool is_gyroscope() const noexcept {
                auto type = get_type();
                return type == sensor_type::gyro ||
                       type == sensor_type::gyro_l ||
                       type == sensor_type::gyro_r;
            }
    };

    /**
     * @brief Get sensor from instance ID
     *
     * Returns the already opened sensor associated with an instance ID.
     *
     * @param instance_id The sensor instance ID
     * @return Non-owning pointer to sensor, or nullptr if not found
     */
    [[nodiscard]] inline SDL_Sensor* get_sensor_from_id(sensor_id instance_id) noexcept {
        return SDL_GetSensorFromID(instance_id);
    }

    /**
     * @brief Update the current state of open sensors
     *
     * This is called automatically by the event loop if sensor events are enabled.
     * This needs to be called from the thread that initialized the sensor subsystem.
     */
    inline void update_sensors() noexcept {
        SDL_UpdateSensors();
    }

    /**
     * @brief Helper class for accelerometer data
     *
     * Provides convenient access to accelerometer readings with proper axis interpretation.
     */
    class accelerometer_data {
        private:
            std::array <float, 3> values;

        public:
            /**
             * @brief Construct from raw sensor data
             *
             * @param data Array of 3 float values (x, y, z)
             */
            explicit accelerometer_data(const std::array <float, 3>& data)
                : values(data) {
            }

            /**
             * @brief Get X-axis acceleration (left/right)
             *
             * @return Acceleration in m/s² (-X = left, +X = right)
             */
            [[nodiscard]] float x() const noexcept { return values[0]; }

            /**
             * @brief Get Y-axis acceleration (bottom/top)
             *
             * @return Acceleration in m/s² (-Y = bottom, +Y = top)
             */
            [[nodiscard]] float y() const noexcept { return values[1]; }

            /**
             * @brief Get Z-axis acceleration (farther/closer)
             *
             * @return Acceleration in m/s² (-Z = farther, +Z = closer)
             */
            [[nodiscard]] float z() const noexcept { return values[2]; }

            /**
             * @brief Get the magnitude of acceleration
             *
             * @return Total acceleration magnitude in m/s²
             */
            [[nodiscard]] float magnitude() const noexcept {
                return std::sqrt(x() * x() + y() * y() + z() * z());
            }

            /**
             * @brief Check if device is approximately at rest
             *
             * @param tolerance Tolerance for considering device at rest (default 0.5 m/s²)
             * @return true if acceleration is close to standard gravity
             */
            [[nodiscard]] bool is_at_rest(float tolerance = 0.5f) const noexcept {
                float mag = magnitude();
                return std::abs(mag - standard_gravity) < tolerance;
            }

            /**
             * @brief Get the raw data array
             *
             * @return Reference to the internal array
             */
            [[nodiscard]] const std::array <float, 3>& data() const noexcept { return values; }
    };

    /**
     * @brief Helper class for gyroscope data
     *
     * Provides convenient access to gyroscope readings with proper axis interpretation.
     */
    class gyroscope_data {
        private:
            std::array <float, 3> values;

        public:
            /**
             * @brief Construct from raw sensor data
             *
             * @param data Array of 3 float values (pitch, yaw, roll)
             */
            explicit gyroscope_data(const std::array <float, 3>& data)
                : values(data) {
            }

            /**
             * @brief Get pitch (rotation around X-axis)
             *
             * @return Angular velocity in radians/second
             */
            [[nodiscard]] float pitch() const noexcept { return values[0]; }

            /**
             * @brief Get yaw (rotation around Y-axis)
             *
             * @return Angular velocity in radians/second
             */
            [[nodiscard]] float yaw() const noexcept { return values[1]; }

            /**
             * @brief Get roll (rotation around Z-axis)
             *
             * @return Angular velocity in radians/second
             */
            [[nodiscard]] float roll() const noexcept { return values[2]; }

            /**
             * @brief Get the magnitude of rotation
             *
             * @return Total angular velocity magnitude in radians/second
             */
            [[nodiscard]] float magnitude() const noexcept {
                return std::sqrt(pitch() * pitch() + yaw() * yaw() + roll() * roll());
            }

            /**
             * @brief Check if device is approximately stationary
             *
             * @param tolerance Tolerance for considering device stationary (default 0.01 rad/s)
             * @return true if angular velocity is below tolerance
             */
            [[nodiscard]] bool is_stationary(float tolerance = 0.01f) const noexcept {
                return magnitude() < tolerance;
            }

            /**
             * @brief Get the raw data array
             *
             * @return Reference to the internal array
             */
            [[nodiscard]] const std::array <float, 3>& data() const noexcept { return values; }
    };

    /**
     * @brief Sensor manager helper class
     *
     * Provides convenient methods for managing multiple sensors.
     */
    class sensor_manager {
        private:
            std::vector <sensor> sensors;

        public:
            /**
             * @brief Open all available sensors of a specific type
             *
             * @param type The sensor type to open
             * @return Number of sensors successfully opened
             */
            size_t open_all_of_type(sensor_type type) {
                size_t opened = 0;
                auto available = sdlpp::get_sensors();

                for (auto id : available) {
                    if (get_sensor_type_for_id(id) == type) {
                        auto open_result = sensor::open(id);
                        if (open_result) {
                            sensors.push_back(std::move(open_result.value()));
                            ++opened;
                        }
                    }
                }

                return opened;
            }

            /**
             * @brief Open all available sensors
             *
             * @return Number of sensors successfully opened
             */
            size_t open_all() {
                size_t opened = 0;
                auto available = sdlpp::get_sensors();

                for (auto id : available) {
                    auto open_result = sensor::open(id);
                    if (open_result) {
                        sensors.push_back(std::move(open_result.value()));
                        ++opened;
                    }
                }

                return opened;
            }

            /**
             * @brief Get all managed sensors
             *
             * @return Reference to vector of sensors
             */
            [[nodiscard]] std::vector <sensor>& get_sensors() noexcept {
                return sensors;
            }

            /**
             * @brief Get all managed sensors (const version)
             *
             * @return Const reference to vector of sensors
             */
            [[nodiscard]] const std::vector <sensor>& get_sensors() const noexcept {
                return sensors;
            }

            /**
             * @brief Find first sensor of specific type
             *
             * @param type The sensor type to find
             * @return Pointer to sensor, or nullptr if not found
             */
            [[nodiscard]] sensor* find_by_type(sensor_type type) noexcept {
                for (auto& s : sensors) {
                    if (s.get_type() == type) {
                        return &s;
                    }
                }
                return nullptr;
            }

            /**
             * @brief Close all managed sensors
             */
            void close_all() {
                sensors.clear();
            }
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for sensor_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, sensor_type value);

/**
 * @brief Stream input operator for sensor_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, sensor_type& value);

}