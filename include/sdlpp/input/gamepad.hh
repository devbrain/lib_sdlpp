#pragma once

/**
 * @file gamepad.hh
 * @brief Gamepad input functionality
 * 
 * This header provides C++ wrappers around SDL3's gamepad API, offering
 * standardized gamepad access with well-defined button and axis mappings.
 * Unlike the joystick API, gamepad API provides semantic meaning to controls.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/utility/guid.hh>
#include <sdlpp/input/joystick.hh>

#include <string>
#include <vector>
#include <optional>
#include <span>
#include <chrono>
#include <cstdint>

namespace sdlpp {
    /**
     * @brief Gamepad types
     */
    enum class gamepad_type : int {
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

    /**
     * @brief Gamepad axis indices
     */
    enum class gamepad_axis : int {
        invalid = SDL_GAMEPAD_AXIS_INVALID,
        leftx = SDL_GAMEPAD_AXIS_LEFTX,
        lefty = SDL_GAMEPAD_AXIS_LEFTY,
        rightx = SDL_GAMEPAD_AXIS_RIGHTX,
        righty = SDL_GAMEPAD_AXIS_RIGHTY,
        left_trigger = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
        right_trigger = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
        max = SDL_GAMEPAD_AXIS_COUNT
    };

    /**
     * @brief Gamepad button indices
     */
    enum class gamepad_button : int {
        invalid = SDL_GAMEPAD_BUTTON_INVALID,
        south = SDL_GAMEPAD_BUTTON_SOUTH, // A/Cross
        east = SDL_GAMEPAD_BUTTON_EAST, // B/Circle
        west = SDL_GAMEPAD_BUTTON_WEST, // X/Square
        north = SDL_GAMEPAD_BUTTON_NORTH, // Y/Triangle
        back = SDL_GAMEPAD_BUTTON_BACK,
        guide = SDL_GAMEPAD_BUTTON_GUIDE,
        start = SDL_GAMEPAD_BUTTON_START,
        left_stick = SDL_GAMEPAD_BUTTON_LEFT_STICK,
        right_stick = SDL_GAMEPAD_BUTTON_RIGHT_STICK,
        left_shoulder = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
        right_shoulder = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
        dpad_up = SDL_GAMEPAD_BUTTON_DPAD_UP,
        dpad_down = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
        dpad_left = SDL_GAMEPAD_BUTTON_DPAD_LEFT,
        dpad_right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        misc1 = SDL_GAMEPAD_BUTTON_MISC1,
        misc2 = SDL_GAMEPAD_BUTTON_MISC2,
        misc3 = SDL_GAMEPAD_BUTTON_MISC3,
        misc4 = SDL_GAMEPAD_BUTTON_MISC4,
        misc5 = SDL_GAMEPAD_BUTTON_MISC5,
        misc6 = SDL_GAMEPAD_BUTTON_MISC6,
        left_paddle1 = SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,
        left_paddle2 = SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,
        right_paddle1 = SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,
        right_paddle2 = SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,
        touchpad = SDL_GAMEPAD_BUTTON_TOUCHPAD,
        max = SDL_GAMEPAD_BUTTON_COUNT
    };

    /**
     * @brief Gamepad button labels (for PS4/PS5 controllers)
     */
    enum class gamepad_button_label : int {
        unknown = SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN,
        a = SDL_GAMEPAD_BUTTON_LABEL_A,
        b = SDL_GAMEPAD_BUTTON_LABEL_B,
        x = SDL_GAMEPAD_BUTTON_LABEL_X,
        y = SDL_GAMEPAD_BUTTON_LABEL_Y,
        cross = SDL_GAMEPAD_BUTTON_LABEL_CROSS,
        circle = SDL_GAMEPAD_BUTTON_LABEL_CIRCLE,
        square = SDL_GAMEPAD_BUTTON_LABEL_SQUARE,
        triangle = SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE
    };

    /**
     * @brief Sensor types for gamepads
     */
    enum class gamepad_sensor_type : int {
        invalid = SDL_SENSOR_INVALID,
        unknown = SDL_SENSOR_UNKNOWN,
        accel = SDL_SENSOR_ACCEL,
        gyro = SDL_SENSOR_GYRO,
        accel_l = SDL_SENSOR_ACCEL_L,
        gyro_l = SDL_SENSOR_GYRO_L,
        accel_r = SDL_SENSOR_ACCEL_R,
        gyro_r = SDL_SENSOR_GYRO_R
    };

    /**
     * @brief Smart pointer type for SDL_Gamepad with automatic cleanup
     */
    using gamepad_ptr = pointer <SDL_Gamepad, SDL_CloseGamepad>;

    /**
     * @brief Check if the gamepad subsystem is initialized
     *
     * @return true if there are gamepads available
     */
    [[nodiscard]] inline bool has_gamepad() noexcept {
        return SDL_HasGamepad();
    }

    /**
     * @brief Get list of available gamepads
     *
     * @return Vector of joystick instance IDs that are gamepads
     */
    [[nodiscard]] inline std::vector <joystick_id> get_gamepads() {
        int count = 0;
        const SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
        if (!gamepads || count <= 0) {
            return {};
        }
        return std::vector <joystick_id>(gamepads, gamepads + count);
    }

    /**
     * @brief Check if a joystick is a gamepad
     *
     * @param instance_id The joystick instance ID
     * @return true if the joystick is a gamepad
     */
    [[nodiscard]] inline bool is_gamepad(joystick_id instance_id) noexcept {
        return SDL_IsGamepad(instance_id);
    }

    /**
     * @brief Get the implementation-dependent name of a gamepad
     *
     * @param instance_id The joystick instance ID
     * @return Gamepad name, or empty string if not found
     */
    [[nodiscard]] inline std::string get_gamepad_name_for_id(joystick_id instance_id) {
        const char* name = SDL_GetGamepadNameForID(instance_id);
        return name ? name : "";
    }

    /**
     * @brief Get the type of a gamepad
     *
     * @param instance_id The joystick instance ID
     * @return Gamepad type
     */
    [[nodiscard]] inline gamepad_type get_gamepad_type_for_id(joystick_id instance_id) noexcept {
        return static_cast <gamepad_type>(SDL_GetGamepadTypeForID(instance_id));
    }

    /**
     * @brief Get the mapping string for a gamepad
     *
     * @param instance_id The joystick instance ID
     * @return Mapping string, or empty if not found
     */
    [[nodiscard]] inline std::string get_gamepad_mapping_for_id(joystick_id instance_id) {
        char* mapping = SDL_GetGamepadMappingForID(instance_id);
        if (!mapping) return "";
        std::string mapping_str(mapping);
        SDL_free(mapping);
        return mapping_str;
    }

    /**
     * @brief RAII wrapper for SDL_Gamepad
     *
     * This class provides a safe, RAII-managed interface to SDL's gamepad functionality.
     * The gamepad is automatically closed when the object goes out of scope.
     */
    class gamepad {
        private:
            gamepad_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty gamepad
             */
            gamepad() = default;

            /**
             * @brief Construct from existing SDL_Gamepad pointer
             * @param g Existing SDL_Gamepad pointer (takes ownership)
             */
            explicit gamepad(SDL_Gamepad* g)
                : ptr(g) {
            }

            /**
             * @brief Move constructor
             */
            gamepad(gamepad&&) = default;

            /**
             * @brief Move assignment operator
             */
            gamepad& operator=(gamepad&&) = default;

            // Delete copy operations - gamepads are move-only
            gamepad(const gamepad&) = delete;
            gamepad& operator=(const gamepad&) = delete;

            /**
             * @brief Check if the gamepad is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Gamepad pointer
             */
            [[nodiscard]] SDL_Gamepad* get() const noexcept { return ptr.get(); }

            /**
             * @brief Open a gamepad for use
             *
             * @param instance_id The joystick instance ID
             * @return Expected containing gamepad, or error message
             */
            static expected <gamepad, std::string> open(joystick_id instance_id) {
                auto* g = SDL_OpenGamepad(instance_id);
                if (!g) {
                    return make_unexpected(get_error());
                }
                return gamepad(g);
            }

            /**
             * @brief Get the instance ID of this gamepad
             *
             * @return Instance ID, or 0 if invalid
             */
            [[nodiscard]] joystick_id get_id() const noexcept {
                return ptr ? SDL_GetGamepadID(ptr.get()) : 0;
            }

            /**
             * @brief Get the name of this gamepad
             *
             * @return Gamepad name, or empty string if invalid
             */
            [[nodiscard]] std::string get_name() const {
                if (!ptr) return "";
                const char* name = SDL_GetGamepadName(ptr.get());
                return name ? name : "";
            }

            /**
             * @brief Get the type of this gamepad
             *
             * @return Gamepad type
             */
            [[nodiscard]] gamepad_type get_type() const noexcept {
                return ptr
                           ? static_cast <gamepad_type>(SDL_GetGamepadType(ptr.get()))
                           : gamepad_type::unknown;
            }

            /**
             * @brief Get the player index of this gamepad
             *
             * @return Player index, or -1 if not set
             */
            [[nodiscard]] int get_player_index() const noexcept {
                return ptr ? SDL_GetGamepadPlayerIndex(ptr.get()) : -1;
            }

            /**
             * @brief Set the player index of this gamepad
             *
             * @param player_index The player index to set
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_player_index(int player_index) {
                if (!ptr) {
                    return make_unexpected("Invalid gamepad");
                }
                if (!SDL_SetGamepadPlayerIndex(ptr.get(), player_index)) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Get the USB vendor ID of this gamepad
             *
             * @return USB vendor ID, or 0 if not available
             */
            [[nodiscard]] uint16_t get_vendor() const noexcept {
                return ptr ? SDL_GetGamepadVendor(ptr.get()) : 0;
            }

            /**
             * @brief Get the USB product ID of this gamepad
             *
             * @return USB product ID, or 0 if not available
             */
            [[nodiscard]] uint16_t get_product() const noexcept {
                return ptr ? SDL_GetGamepadProduct(ptr.get()) : 0;
            }

            /**
             * @brief Get the product version of this gamepad
             *
             * @return Product version, or 0 if not available
             */
            [[nodiscard]] uint16_t get_product_version() const noexcept {
                return ptr ? SDL_GetGamepadProductVersion(ptr.get()) : 0;
            }

            /**
             * @brief Get the firmware version of this gamepad
             *
             * @return Firmware version, or 0 if not available
             */
            [[nodiscard]] uint16_t get_firmware_version() const noexcept {
                return ptr ? SDL_GetGamepadFirmwareVersion(ptr.get()) : 0;
            }

            /**
             * @brief Get the serial number of this gamepad
             *
             * @return Serial number, or empty string if not available
             */
            [[nodiscard]] std::string get_serial() const {
                if (!ptr) return "";
                const char* serial = SDL_GetGamepadSerial(ptr.get());
                return serial ? serial : "";
            }

            /**
             * @brief Get the Steam handle of this gamepad
             *
             * @return Steam Input API handle, or 0 if not available
             */
            [[nodiscard]] uint64_t get_steam_handle() const noexcept {
                return ptr ? SDL_GetGamepadSteamHandle(ptr.get()) : 0;
            }

            /**
             * @brief Get the connection state of this gamepad
             *
             * @return Connection state
             */
            [[nodiscard]] joystick_connection_state get_connection_state() const noexcept {
                return ptr
                           ? static_cast <joystick_connection_state>(SDL_GetGamepadConnectionState(ptr.get()))
                           : joystick_connection_state::invalid;
            }

            /**
             * @brief Get the power info of this gamepad
             *
             * @param[out] percent Battery percentage (0-100), or -1 if unknown
             * @return Power state
             */
            [[nodiscard]] power_state get_power_info(int* percent = nullptr) const noexcept {
                if (!ptr) {
                    if (percent) *percent = -1;
                    return power_state::error;
                }
                return static_cast <power_state>(SDL_GetGamepadPowerInfo(ptr.get(), percent));
            }

            /**
             * @brief Check if this gamepad has a specific axis
             *
             * @param axis The axis to check
             * @return true if the gamepad has this axis
             */
            [[nodiscard]] bool has_axis(gamepad_axis axis) const noexcept {
                return ptr && SDL_GamepadHasAxis(ptr.get(), static_cast <SDL_GamepadAxis>(axis));
            }

            /**
             * @brief Get the current value of an axis
             *
             * @param axis The axis to query
             * @return Axis value (-32768 to 32767), or 0 if invalid
             */
            [[nodiscard]] int16_t get_axis(gamepad_axis axis) const noexcept {
                return ptr ? SDL_GetGamepadAxis(ptr.get(), static_cast <SDL_GamepadAxis>(axis)) : static_cast<int16_t>(0);
            }

            /**
             * @brief Check if this gamepad has a specific button
             *
             * @param button The button to check
             * @return true if the gamepad has this button
             */
            [[nodiscard]] bool has_button(gamepad_button button) const noexcept {
                return ptr && SDL_GamepadHasButton(ptr.get(), static_cast <SDL_GamepadButton>(button));
            }

            /**
             * @brief Get the current state of a button
             *
             * @param button The button to query
             * @return true if pressed, false if released
             */
            [[nodiscard]] bool get_button(gamepad_button button) const noexcept {
                return ptr && SDL_GetGamepadButton(ptr.get(), static_cast <SDL_GamepadButton>(button));
            }

            /**
             * @brief Get the label for a button
             *
             * @param button The button to query
             * @return Button label
             */
            [[nodiscard]] gamepad_button_label get_button_label(gamepad_button button) const noexcept {
                return ptr
                           ? static_cast <gamepad_button_label>(
                               SDL_GetGamepadButtonLabel(ptr.get(), static_cast <SDL_GamepadButton>(button)))
                           : gamepad_button_label::unknown;
            }

            /**
             * @brief Get the number of touchpads on this gamepad
             *
             * @return Number of touchpads
             */
            [[nodiscard]] size_t get_num_touchpads() const noexcept {
                if (!ptr) return 0;
                int count = SDL_GetNumGamepadTouchpads(ptr.get());
                return count > 0 ? static_cast<size_t>(count) : 0;
            }

            /**
             * @brief Get the number of fingers a touchpad supports
             *
             * @param touchpad The touchpad index
             * @return Number of simultaneous fingers supported
             */
            [[nodiscard]] size_t get_num_touchpad_fingers(int touchpad) const noexcept {
                if (!ptr) return 0;
                int count = SDL_GetNumGamepadTouchpadFingers(ptr.get(), touchpad);
                return count > 0 ? static_cast<size_t>(count) : 0;
            }

            /**
             * @brief Get touchpad finger state
             *
             * @param touchpad The touchpad index
             * @param finger The finger index
             * @param[out] down Set to true if finger is down
             * @param[out] x X position (0.0 to 1.0)
             * @param[out] y Y position (0.0 to 1.0)
             * @param[out] pressure Pressure (0.0 to 1.0)
             * @return true on success
             */
            [[nodiscard]] bool get_touchpad_finger(int touchpad, int finger,
                                                   bool* down, float* x, float* y, float* pressure) const noexcept {
                if (!ptr) return false;
                bool finger_down = false;
                bool success = SDL_GetGamepadTouchpadFinger(ptr.get(), touchpad, finger,
                                                           &finger_down, x, y, pressure);
                if (down) *down = finger_down;
                return success;
            }

            /**
             * @brief Check if this gamepad has a specific sensor
             *
             * @param type The sensor type
             * @return true if the gamepad has this sensor
             */
            [[nodiscard]] bool has_sensor(gamepad_sensor_type type) const noexcept {
                return ptr && SDL_GamepadHasSensor(ptr.get(), static_cast <SDL_SensorType>(type));
            }

            /**
             * @brief Enable or disable a sensor
             *
             * @param type The sensor type
             * @param enabled true to enable, false to disable
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_sensor_enabled(gamepad_sensor_type type, bool enabled) {
                if (!ptr) {
                    return make_unexpected("Invalid gamepad");
                }
                if (!SDL_SetGamepadSensorEnabled(ptr.get(), static_cast <SDL_SensorType>(type), enabled)) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Check if a sensor is enabled
             *
             * @param type The sensor type
             * @return true if enabled
             */
            [[nodiscard]] bool is_sensor_enabled(gamepad_sensor_type type) const noexcept {
                return ptr && SDL_GamepadSensorEnabled(ptr.get(), static_cast <SDL_SensorType>(type));
            }

            /**
             * @brief Get the data rate of a sensor
             *
             * @param type The sensor type
             * @return Data rate in Hz, or 0.0f if not available
             */
            [[nodiscard]] float get_sensor_data_rate(gamepad_sensor_type type) const noexcept {
                return ptr ? SDL_GetGamepadSensorDataRate(ptr.get(), static_cast <SDL_SensorType>(type)) : 0.0f;
            }

            /**
             * @brief Get sensor data
             *
             * @param type The sensor type
             * @param data Output buffer for sensor data
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> get_sensor_data(gamepad_sensor_type type, std::span <float> data) {
                if (!ptr) {
                    return make_unexpected("Invalid gamepad");
                }
                if (!SDL_GetGamepadSensorData(ptr.get(), static_cast <SDL_SensorType>(type),
                                              data.data(), static_cast <int>(data.size()))) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Start a rumble effect
             *
             * @param low_frequency_rumble Low frequency motor intensity (0-65535)
             * @param high_frequency_rumble High frequency motor intensity (0-65535)
             * @param duration_ms Duration in milliseconds
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> rumble(uint16_t low_frequency_rumble,
                                                uint16_t high_frequency_rumble,
                                                uint32_t duration_ms) {
                if (!ptr) {
                    return make_unexpected("Invalid gamepad");
                }
                if (!SDL_RumbleGamepad(ptr.get(), low_frequency_rumble, high_frequency_rumble, duration_ms)) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Start a rumble effect on triggers
             *
             * @param left_rumble Left trigger motor intensity (0-65535)
             * @param right_rumble Right trigger motor intensity (0-65535)
             * @param duration_ms Duration in milliseconds
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> rumble_triggers(uint16_t left_rumble,
                                                         uint16_t right_rumble,
                                                         uint32_t duration_ms) {
                if (!ptr) {
                    return make_unexpected("Invalid gamepad");
                }
                if (!SDL_RumbleGamepadTriggers(ptr.get(), left_rumble, right_rumble, duration_ms)) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Set the LED color
             *
             * @param red Red intensity (0-255)
             * @param green Green intensity (0-255)
             * @param blue Blue intensity (0-255)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_led(uint8_t red, uint8_t green, uint8_t blue) {
                if (!ptr) {
                    return make_unexpected("Invalid gamepad");
                }
                if (!SDL_SetGamepadLED(ptr.get(), red, green, blue)) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Send an effect packet
             *
             * @param data The data to send
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> send_effect(std::span <const uint8_t> data) {
                if (!ptr) {
                    return make_unexpected("Invalid gamepad");
                }
                if (!SDL_SendGamepadEffect(ptr.get(), data.data(), static_cast <int>(data.size()))) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Get the underlying joystick
             *
             * @return Joystick pointer (non-owning), or nullptr if invalid
             */
            [[nodiscard]] SDL_Joystick* get_joystick() const noexcept {
                return ptr ? SDL_GetGamepadJoystick(ptr.get()) : nullptr;
            }

            /**
             * @brief Get the mapping string for this gamepad
             *
             * @return Mapping string, or empty if not found
             */
            [[nodiscard]] std::string get_mapping() const {
                if (!ptr) return "";
                char* mapping = SDL_GetGamepadMapping(ptr.get());
                if (!mapping) return "";
                std::string mapping_str(mapping);
                SDL_free(mapping);
                return mapping_str;
            }

            /**
             * @brief Apply a mapping string to this gamepad
             *
             * @param mapping The mapping string
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_mapping(const std::string& mapping) {
                if (!ptr) {
                    return make_unexpected("Invalid gamepad");
                }
                if (!SDL_SetGamepadMapping(get_id(), mapping.c_str())) {
                    return make_unexpected(get_error());
                }
                return {};
            }
    };

    /**
     * @brief Update the gamepad database
     *
     * This is called automatically by the event loop if you use it.
     * You only need to call this if you are not using the event loop.
     */
    inline void update_gamepads() noexcept {
        SDL_UpdateGamepads();
    }

    /**
     * @brief Get the name for a gamepad axis
     *
     * @param axis The axis
     * @return Human-readable name
     */
    [[nodiscard]] inline std::string get_gamepad_axis_name(gamepad_axis axis) {
        const char* name = SDL_GetGamepadStringForAxis(static_cast <SDL_GamepadAxis>(axis));
        return name ? name : "Unknown";
    }

    /**
     * @brief Get the name for a gamepad button
     *
     * @param button The button
     * @return Human-readable name
     */
    [[nodiscard]] inline std::string get_gamepad_button_name(gamepad_button button) {
        const char* name = SDL_GetGamepadStringForButton(static_cast <SDL_GamepadButton>(button));
        return name ? name : "Unknown";
    }

    /**
     * @brief Parse a gamepad axis from string
     *
     * @param str The string to parse
     * @return The axis, or gamepad_axis::invalid if not recognized
     */
    [[nodiscard]] inline gamepad_axis get_gamepad_axis_from_string(const std::string& str) {
        return static_cast <gamepad_axis>(SDL_GetGamepadAxisFromString(str.c_str()));
    }

    /**
     * @brief Parse a gamepad button from string
     *
     * @param str The string to parse
     * @return The button, or gamepad_button::invalid if not recognized
     */
    [[nodiscard]] inline gamepad_button get_gamepad_button_from_string(const std::string& str) {
        return static_cast <gamepad_button>(SDL_GetGamepadButtonFromString(str.c_str()));
    }

    /**
     * @brief Add a gamepad mapping
     *
     * @param mapping The mapping string
     * @return Expected<int> - 1 if added, 0 if updated, or error message
     */
    inline expected <int, std::string> add_gamepad_mapping(const std::string& mapping) {
        int mapping_count = SDL_AddGamepadMapping(mapping.c_str());
        if (mapping_count < 0) {
            return make_unexpected(get_error());
        }
        return mapping_count;
    }

    /**
     * @brief Add gamepad mappings from file
     *
     * @param file Path to the mapping file
     * @return Expected<int> - number of mappings added, or error message
     */
    inline expected <int, std::string> add_gamepad_mappings_from_file(const std::string& file) {
        int mapping_count = SDL_AddGamepadMappingsFromFile(file.c_str());
        if (mapping_count < 0) {
            return make_unexpected(get_error());
        }
        return mapping_count;
    }

    /**
     * @brief Add gamepad mappings from IO stream
     *
     * @param stream The IO stream containing mappings
     * @param close_stream Whether to close the stream after reading
     * @return Expected<int> - number of mappings added, or error message
     */
    inline expected <int, std::string> add_gamepad_mappings_from_io(SDL_IOStream* stream, bool close_stream) {
        int mapping_count = SDL_AddGamepadMappingsFromIO(stream, close_stream);
        if (mapping_count < 0) {
            return make_unexpected(get_error());
        }
        return mapping_count;
    }

    /**
     * @brief Get the mapping for a GUID
     *
     * @param guid The GUID to look up
     * @return Mapping string, or empty if not found
     */
    [[nodiscard]] inline std::string get_gamepad_mapping_for_guid(const guid& guid) {
        SDL_GUID sdl_guid = guid.to_sdl();
        char* mapping = SDL_GetGamepadMappingForGUID(sdl_guid);
        if (!mapping) return "";
        std::string mapping_str(mapping);
        SDL_free(mapping);
        return mapping_str;
    }

    /**
     * @brief Get the gamepad type from name
     *
     * @param name The gamepad name
     * @return Gamepad type
     */
    [[nodiscard]] inline gamepad_type get_gamepad_type_from_string(const std::string& name) {
        return static_cast <gamepad_type>(SDL_GetGamepadTypeFromString(name.c_str()));
    }

    /**
     * @brief Get string representation of gamepad type
     *
     * @param type The gamepad type
     * @return String representation
     */
    [[nodiscard]] inline std::string get_gamepad_type_string(gamepad_type type) {
        const char* str = SDL_GetGamepadStringForType(static_cast <SDL_GamepadType>(type));
        return str ? str : "unknown";
    }

    /**
     * @brief Helper class for gamepad state
     *
     * This class provides convenient access to gamepad state with
     * semantic button and axis names.
     */
    class gamepad_state {
        private:
            gamepad* pad = nullptr;

        public:
            /**
             * @brief Construct gamepad state helper
             *
             * @param g The gamepad to track
             */
            explicit gamepad_state(gamepad& g) noexcept
                : pad(&g) {
            }

            // Axes
            [[nodiscard]] int16_t left_x() const noexcept { return pad->get_axis(gamepad_axis::leftx); }
            [[nodiscard]] int16_t left_y() const noexcept { return pad->get_axis(gamepad_axis::lefty); }
            [[nodiscard]] int16_t right_x() const noexcept { return pad->get_axis(gamepad_axis::rightx); }
            [[nodiscard]] int16_t right_y() const noexcept { return pad->get_axis(gamepad_axis::righty); }
            [[nodiscard]] int16_t left_trigger() const noexcept { return pad->get_axis(gamepad_axis::left_trigger); }
            [[nodiscard]] int16_t right_trigger() const noexcept { return pad->get_axis(gamepad_axis::right_trigger); }

            // Face buttons
            [[nodiscard]] bool a() const noexcept { return pad->get_button(gamepad_button::south); }
            [[nodiscard]] bool b() const noexcept { return pad->get_button(gamepad_button::east); }
            [[nodiscard]] bool x() const noexcept { return pad->get_button(gamepad_button::west); }
            [[nodiscard]] bool y() const noexcept { return pad->get_button(gamepad_button::north); }

            // D-pad
            [[nodiscard]] bool dpad_up() const noexcept { return pad->get_button(gamepad_button::dpad_up); }
            [[nodiscard]] bool dpad_down() const noexcept { return pad->get_button(gamepad_button::dpad_down); }
            [[nodiscard]] bool dpad_left() const noexcept { return pad->get_button(gamepad_button::dpad_left); }
            [[nodiscard]] bool dpad_right() const noexcept { return pad->get_button(gamepad_button::dpad_right); }

            // Shoulders
            [[nodiscard]] bool left_shoulder() const noexcept { return pad->get_button(gamepad_button::left_shoulder); }

            [[nodiscard]] bool right_shoulder() const noexcept {
                return pad->get_button(gamepad_button::right_shoulder);
            }

            // Sticks
            [[nodiscard]] bool left_stick() const noexcept { return pad->get_button(gamepad_button::left_stick); }
            [[nodiscard]] bool right_stick() const noexcept { return pad->get_button(gamepad_button::right_stick); }

            // Menu buttons
            [[nodiscard]] bool start() const noexcept { return pad->get_button(gamepad_button::start); }
            [[nodiscard]] bool back() const noexcept { return pad->get_button(gamepad_button::back); }
            [[nodiscard]] bool guide() const noexcept { return pad->get_button(gamepad_button::guide); }

            // Touchpad
            [[nodiscard]] bool touchpad() const noexcept { return pad->get_button(gamepad_button::touchpad); }

            // Check if any button is pressed
            [[nodiscard]] bool any_button_pressed() const noexcept {
                for (int i = 0; i < static_cast <int>(gamepad_button::max); ++i) {
                    if (pad->get_button(static_cast <gamepad_button>(i))) {
                        return true;
                    }
                }
                return false;
            }
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for gamepad_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gamepad_type value);

/**
 * @brief Stream input operator for gamepad_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gamepad_type& value);

/**
 * @brief Stream output operator for gamepad_axis
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gamepad_axis value);

/**
 * @brief Stream input operator for gamepad_axis
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gamepad_axis& value);

/**
 * @brief Stream output operator for gamepad_button
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gamepad_button value);

/**
 * @brief Stream input operator for gamepad_button
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gamepad_button& value);

/**
 * @brief Stream output operator for gamepad_button_label
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gamepad_button_label value);

/**
 * @brief Stream input operator for gamepad_button_label
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gamepad_button_label& value);

/**
 * @brief Stream output operator for gamepad_sensor_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, gamepad_sensor_type value);

/**
 * @brief Stream input operator for gamepad_sensor_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, gamepad_sensor_type& value);

}