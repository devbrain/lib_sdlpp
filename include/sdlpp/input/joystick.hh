#pragma once

/**
 * @file joystick.hh
 * @brief Joystick input functionality
 * 
 * This header provides C++ wrappers around SDL3's joystick API, offering
 * low-level joystick access. For most games, consider using the gamepad API
 * instead, which provides standardized button mappings.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/system/power_state.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/utility/guid.hh>

#include <string>
#include <vector>
#include <optional>
#include <span>
#include <cstdint>

namespace sdlpp {
    /**
     * @brief Joystick instance ID type
     */
    using joystick_id = SDL_JoystickID;

    /**
     * @brief Power state (from SDL_power.h)
     */

    /**
     * @brief Joystick type
     */
    enum class joystick_type : int {
        unknown = SDL_JOYSTICK_TYPE_UNKNOWN,
        gamepad = SDL_JOYSTICK_TYPE_GAMEPAD,
        wheel = SDL_JOYSTICK_TYPE_WHEEL,
        arcade_stick = SDL_JOYSTICK_TYPE_ARCADE_STICK,
        flight_stick = SDL_JOYSTICK_TYPE_FLIGHT_STICK,
        dance_pad = SDL_JOYSTICK_TYPE_DANCE_PAD,
        guitar = SDL_JOYSTICK_TYPE_GUITAR,
        drum_kit = SDL_JOYSTICK_TYPE_DRUM_KIT,
        arcade_pad = SDL_JOYSTICK_TYPE_ARCADE_PAD,
        throttle = SDL_JOYSTICK_TYPE_THROTTLE
    };

    /**
     * @brief Joystick connection state
     */
    enum class joystick_connection_state : int {
        invalid = SDL_JOYSTICK_CONNECTION_INVALID,
        unknown = SDL_JOYSTICK_CONNECTION_UNKNOWN,
        wired = SDL_JOYSTICK_CONNECTION_WIRED,
        wireless = SDL_JOYSTICK_CONNECTION_WIRELESS
    };

    /**
     * @brief Hat position values
     */
    enum class hat_position : uint8_t {
        centered = SDL_HAT_CENTERED,
        up = SDL_HAT_UP,
        right = SDL_HAT_RIGHT,
        down = SDL_HAT_DOWN,
        left = SDL_HAT_LEFT,
        rightup = SDL_HAT_RIGHTUP,
        rightdown = SDL_HAT_RIGHTDOWN,
        leftup = SDL_HAT_LEFTUP,
        leftdown = SDL_HAT_LEFTDOWN
    };

    /**
     * @brief Smart pointer type for SDL_Joystick with automatic cleanup
     */
    using joystick_ptr = pointer <SDL_Joystick, SDL_CloseJoystick>;

    /**
     * @brief Check if the joystick subsystem is initialized
     *
     * @return true if joystick subsystem is initialized
     */
    [[nodiscard]] inline bool has_joystick() noexcept {
        return SDL_HasJoystick();
    }

    /**
     * @brief Get list of available joysticks
     *
     * @return Vector of joystick instance IDs
     */
    [[nodiscard]] inline std::vector <joystick_id> get_joysticks() {
        int count = 0;
        const SDL_JoystickID* joysticks = SDL_GetJoysticks(&count);
        if (!joysticks || count <= 0) {
            return {};
        }
        return std::vector <joystick_id>(joysticks, joysticks + count);
    }

    /**
     * @brief Update the joystick subsystem
     *
     * This is called automatically by the event loop if you are using it.
     * You only need to call this if you are not using the event loop.
     */
    inline void update_joysticks() noexcept {
        SDL_UpdateJoysticks();
    }

    /**
     * @brief Get the implementation-dependent name of a joystick
     *
     * @param instance_id The joystick instance ID
     * @return Joystick name, or empty string if not found
     */
    [[nodiscard]] inline std::string get_joystick_name_for_id(joystick_id instance_id) {
        const char* name = SDL_GetJoystickNameForID(instance_id);
        return name ? name : "";
    }

    /**
     * @brief Get the implementation-dependent path of a joystick
     *
     * @param instance_id The joystick instance ID
     * @return Joystick path, or empty string if not found
     */
    [[nodiscard]] inline std::string get_joystick_path_for_id(joystick_id instance_id) {
        const char* path = SDL_GetJoystickPathForID(instance_id);
        return path ? path : "";
    }

    /**
     * @brief Get the player index of a joystick
     *
     * @param instance_id The joystick instance ID
     * @return Player index, or -1 if not set
     */
    [[nodiscard]] inline int get_joystick_player_index_for_id(joystick_id instance_id) noexcept {
        return SDL_GetJoystickPlayerIndexForID(instance_id);
    }

    /**
     * @brief Get the GUID of a joystick
     *
     * @param instance_id The joystick instance ID
     * @return GUID of the joystick
     */
    [[nodiscard]] inline guid get_joystick_guid_for_id(joystick_id instance_id) noexcept {
        return guid(SDL_GetJoystickGUIDForID(instance_id));
    }

    /**
     * @brief Get the USB vendor ID of a joystick
     *
     * @param instance_id The joystick instance ID
     * @return USB vendor ID, or 0 if not available
     */
    [[nodiscard]] inline uint16_t get_joystick_vendor_for_id(joystick_id instance_id) noexcept {
        return SDL_GetJoystickVendorForID(instance_id);
    }

    /**
     * @brief Get the USB product ID of a joystick
     *
     * @param instance_id The joystick instance ID
     * @return USB product ID, or 0 if not available
     */
    [[nodiscard]] inline uint16_t get_joystick_product_for_id(joystick_id instance_id) noexcept {
        return SDL_GetJoystickProductForID(instance_id);
    }

    /**
     * @brief Get the product version of a joystick
     *
     * @param instance_id The joystick instance ID
     * @return Product version, or 0 if not available
     */
    [[nodiscard]] inline uint16_t get_joystick_product_version_for_id(joystick_id instance_id) noexcept {
        return SDL_GetJoystickProductVersionForID(instance_id);
    }

    /**
     * @brief Get the type of a joystick
     *
     * @param instance_id The joystick instance ID
     * @return Joystick type
     */
    [[nodiscard]] inline joystick_type get_joystick_type_for_id(joystick_id instance_id) noexcept {
        return static_cast <joystick_type>(SDL_GetJoystickTypeForID(instance_id));
    }

    /**
     * @brief RAII wrapper for SDL_Joystick
     *
     * This class provides a safe, RAII-managed interface to SDL's joystick functionality.
     * The joystick is automatically closed when the object goes out of scope.
     */
    class joystick {
        private:
            joystick_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty joystick
             */
            joystick() = default;

            /**
             * @brief Construct from existing SDL_Joystick pointer
             * @param j Existing SDL_Joystick pointer (takes ownership)
             */
            explicit joystick(SDL_Joystick* j)
                : ptr(j) {
            }

            /**
             * @brief Move constructor
             */
            joystick(joystick&&) = default;

            /**
             * @brief Move assignment operator
             */
            joystick& operator=(joystick&&) = default;

            // Delete copy operations - joysticks are move-only
            joystick(const joystick&) = delete;
            joystick& operator=(const joystick&) = delete;

            /**
             * @brief Check if the joystick is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Joystick pointer
             */
            [[nodiscard]] SDL_Joystick* get() const noexcept { return ptr.get(); }

            /**
             * @brief Open a joystick for use
             *
             * @param instance_id The joystick instance ID
             * @return Expected containing joystick, or error message
             */
            static expected <joystick, std::string> open(joystick_id instance_id) {
                auto* j = SDL_OpenJoystick(instance_id);
                if (!j) {
                    return make_unexpectedf(get_error());
                }
                return joystick(j);
            }

            /**
             * @brief Get the instance ID of this joystick
             *
             * @return Instance ID, or 0 if invalid
             */
            [[nodiscard]] joystick_id get_id() const noexcept {
                return ptr ? SDL_GetJoystickID(ptr.get()) : 0;
            }

            /**
             * @brief Get the name of this joystick
             *
             * @return Joystick name, or empty string if invalid
             */
            [[nodiscard]] std::string get_name() const {
                if (!ptr) return "";
                const char* name = SDL_GetJoystickName(ptr.get());
                return name ? name : "";
            }

            /**
             * @brief Get the path of this joystick
             *
             * @return Joystick path, or empty string if invalid
             */
            [[nodiscard]] std::string get_path() const {
                if (!ptr) return "";
                const char* path = SDL_GetJoystickPath(ptr.get());
                return path ? path : "";
            }

            /**
             * @brief Get the player index of this joystick
             *
             * @return Player index, or -1 if not set
             */
            [[nodiscard]] int get_player_index() const noexcept {
                return ptr ? SDL_GetJoystickPlayerIndex(ptr.get()) : -1;
            }

            /**
             * @brief Set the player index of this joystick
             *
             * @param player_index The player index to set
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_player_index(int player_index) {
                if (!ptr) {
                    return make_unexpectedf("Invalid joystick");
                }
                if (!SDL_SetJoystickPlayerIndex(ptr.get(), player_index)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Get the GUID of this joystick
             *
             * @return GUID of the joystick
             */
            [[nodiscard]] guid get_guid() const noexcept {
                return ptr ? guid(SDL_GetJoystickGUID(ptr.get())) : guid();
            }

            /**
             * @brief Get the USB vendor ID of this joystick
             *
             * @return USB vendor ID, or 0 if not available
             */
            [[nodiscard]] uint16_t get_vendor() const noexcept {
                return ptr ? SDL_GetJoystickVendor(ptr.get()) : 0;
            }

            /**
             * @brief Get the USB product ID of this joystick
             *
             * @return USB product ID, or 0 if not available
             */
            [[nodiscard]] uint16_t get_product() const noexcept {
                return ptr ? SDL_GetJoystickProduct(ptr.get()) : 0;
            }

            /**
             * @brief Get the product version of this joystick
             *
             * @return Product version, or 0 if not available
             */
            [[nodiscard]] uint16_t get_product_version() const noexcept {
                return ptr ? SDL_GetJoystickProductVersion(ptr.get()) : 0;
            }

            /**
             * @brief Get the firmware version of this joystick
             *
             * @return Firmware version, or 0 if not available
             */
            [[nodiscard]] uint16_t get_firmware_version() const noexcept {
                return ptr ? SDL_GetJoystickFirmwareVersion(ptr.get()) : 0;
            }

            /**
             * @brief Get the serial number of this joystick
             *
             * @return Serial number, or empty string if not available
             */
            [[nodiscard]] std::string get_serial() const {
                if (!ptr) return "";
                const char* serial = SDL_GetJoystickSerial(ptr.get());
                return serial ? serial : "";
            }

            /**
             * @brief Get the type of this joystick
             *
             * @return Joystick type
             */
            [[nodiscard]] joystick_type get_type() const noexcept {
                return ptr
                           ? static_cast <joystick_type>(SDL_GetJoystickType(ptr.get()))
                           : joystick_type::unknown;
            }

            /**
             * @brief Check if this joystick has been opened as a gamepad
             *
             * @return true if attached as gamepad
             */
            [[nodiscard]] bool is_gamepad() const noexcept {
                return ptr && SDL_IsGamepad(get_id());
            }

            /**
             * @brief Get the connection state of this joystick
             *
             * @return Connection state
             */
            [[nodiscard]] joystick_connection_state get_connection_state() const noexcept {
                return ptr
                           ? static_cast <joystick_connection_state>(SDL_GetJoystickConnectionState(ptr.get()))
                           : joystick_connection_state::invalid;
            }

            /**
             * @brief Get the power info of this joystick
             *
             * @param[out] percent Battery percentage (0-100), or -1 if unknown
             * @return Power state
             */
            [[nodiscard]] power_state get_power_info(int* percent = nullptr) const noexcept {
                if (!ptr) {
                    if (percent) *percent = -1;
                    return power_state::error;
                }
                return static_cast <power_state>(SDL_GetJoystickPowerInfo(ptr.get(), percent));
            }

            /**
             * @brief Get the number of axes on this joystick
             *
             * @return Number of axes, or 0 if invalid
             */
            [[nodiscard]] size_t get_num_axes() const noexcept {
                if (!ptr) return 0;
                int count = SDL_GetNumJoystickAxes(ptr.get());
                return count > 0 ? static_cast<size_t>(count) : 0;
            }

            /**
             * @brief Get the number of trackballs on this joystick
             *
             * @return Number of trackballs, or 0 if invalid
             */
            [[nodiscard]] size_t get_num_balls() const noexcept {
                if (!ptr) return 0;
                int count = SDL_GetNumJoystickBalls(ptr.get());
                return count > 0 ? static_cast<size_t>(count) : 0;
            }

            /**
             * @brief Get the number of hats on this joystick
             *
             * @return Number of hats, or 0 if invalid
             */
            [[nodiscard]] size_t get_num_hats() const noexcept {
                if (!ptr) return 0;
                int count = SDL_GetNumJoystickHats(ptr.get());
                return count > 0 ? static_cast<size_t>(count) : 0;
            }

            /**
             * @brief Get the number of buttons on this joystick
             *
             * @return Number of buttons, or 0 if invalid
             */
            [[nodiscard]] size_t get_num_buttons() const noexcept {
                if (!ptr) return 0;
                int count = SDL_GetNumJoystickButtons(ptr.get());
                return count > 0 ? static_cast<size_t>(count) : 0;
            }

            /**
             * @brief Get the current state of an axis
             *
             * @param axis The axis index (0 to get_num_axes()-1)
             * @return Axis value (-32768 to 32767), or 0 if invalid
             */
            [[nodiscard]] int16_t get_axis(size_t axis) const noexcept {
                return ptr ? SDL_GetJoystickAxis(ptr.get(), static_cast<int>(axis)) : static_cast<int16_t>(0);
            }

            /**
             * @brief Get whether an axis has an initial value
             *
             * @param axis The axis index
             * @param[out] state Set to the initial state if available
             * @return true if axis has been moved, false otherwise
             */
            [[nodiscard]] bool get_axis_initial_state(size_t axis, int16_t& state) const noexcept {
                return ptr && SDL_GetJoystickAxisInitialState(ptr.get(), static_cast<int>(axis), &state);
            }

            /**
             * @brief Get the ball axis change since the last poll
             *
             * @param ball The ball index (0 to get_num_balls()-1)
             * @param[out] dx The X axis change
             * @param[out] dy The Y axis change
             * @return true on success, false if invalid
             */
            [[nodiscard]] bool get_ball(size_t ball, int& dx, int& dy) const noexcept {
                return ptr && SDL_GetJoystickBall(ptr.get(), static_cast<int>(ball), &dx, &dy);
            }

            /**
             * @brief Get the current state of a hat
             *
             * @param hat The hat index (0 to get_num_hats()-1)
             * @return Hat position
             */
            [[nodiscard]] hat_position get_hat(size_t hat) const noexcept {
                return ptr
                           ? static_cast <hat_position>(SDL_GetJoystickHat(ptr.get(), static_cast<int>(hat)))
                           : hat_position::centered;
            }

            /**
             * @brief Get the current state of a button
             *
             * @param button The button index (0 to get_num_buttons()-1)
             * @return true if pressed, false if released
             */
            [[nodiscard]] bool get_button(size_t button) const noexcept {
                return ptr && SDL_GetJoystickButton(ptr.get(), static_cast<int>(button));
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
                    return make_unexpectedf("Invalid joystick");
                }
                if (!SDL_RumbleJoystick(ptr.get(), low_frequency_rumble, high_frequency_rumble, duration_ms)) {
                    return make_unexpectedf(get_error());
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
                    return make_unexpectedf("Invalid joystick");
                }
                if (!SDL_RumbleJoystickTriggers(ptr.get(), left_rumble, right_rumble, duration_ms)) {
                    return make_unexpectedf(get_error());
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
                    return make_unexpectedf("Invalid joystick");
                }
                if (!SDL_SetJoystickLED(ptr.get(), red, green, blue)) {
                    return make_unexpectedf(get_error());
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
                    return make_unexpectedf("Invalid joystick");
                }
                if (!SDL_SendJoystickEffect(ptr.get(), data.data(), static_cast <int>(data.size()))) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }
    };

    /**
     * @brief Check if a joystick is virtual
     *
     * @param instance_id The joystick instance ID
     * @return true if virtual, false otherwise
     */
    [[nodiscard]] inline bool is_joystick_virtual(joystick_id instance_id) noexcept {
        return SDL_IsJoystickVirtual(instance_id);
    }

    /**
     * @brief Virtual joystick descriptor
     */
    struct virtual_joystick_desc {
        uint16_t vendor_id = 0;
        uint16_t product_id = 0;
        uint16_t naxes = 0;
        uint16_t nballs = 0;
        uint16_t nbuttons = 0;
        uint16_t nhats = 0;
        uint16_t padding = 0;
        uint32_t button_mask = 0;
        uint32_t axis_mask = 0;
        const char* name = nullptr;
        void* userdata = nullptr;

        // Callbacks would be added here as function pointers
        // For now, leaving them out for simplicity
    };

    /**
     * @brief Attach a virtual joystick
     *
     * @param desc Virtual joystick descriptor
     * @return Expected containing joystick ID, or error message
     */
    inline expected <joystick_id, std::string> attach_virtual_joystick(const virtual_joystick_desc& desc) {
        SDL_VirtualJoystickDesc sdl_desc{};
        sdl_desc.vendor_id = desc.vendor_id;
        sdl_desc.product_id = desc.product_id;
        sdl_desc.naxes = desc.naxes;
        sdl_desc.nballs = desc.nballs;
        sdl_desc.nbuttons = desc.nbuttons;
        sdl_desc.nhats = desc.nhats;
        sdl_desc.padding = desc.padding;
        sdl_desc.button_mask = desc.button_mask;
        sdl_desc.axis_mask = desc.axis_mask;
        sdl_desc.name = desc.name;
        sdl_desc.userdata = desc.userdata;

        SDL_JoystickID id = SDL_AttachVirtualJoystick(&sdl_desc);
        if (id == 0) {
            return make_unexpectedf(get_error());
        }
        return id;
    }

    /**
     * @brief Detach a virtual joystick
     *
     * @param instance_id The joystick instance ID
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> detach_virtual_joystick(joystick_id instance_id) {
        if (!SDL_DetachVirtualJoystick(instance_id)) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Set virtual joystick axis value
     *
     * @param joystick The joystick
     * @param axis The axis index
     * @param value The axis value (-32768 to 32767)
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> set_virtual_joystick_axis(SDL_Joystick* joystick, int axis, int16_t value) {
        if (!SDL_SetJoystickVirtualAxis(joystick, axis, value)) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Set virtual joystick ball value
     *
     * @param joystick The joystick
     * @param ball The ball index
     * @param xrel The X relative motion
     * @param yrel The Y relative motion
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> set_virtual_joystick_ball(SDL_Joystick* joystick, int ball, int16_t xrel,
                                                                  int16_t yrel) {
        if (!SDL_SetJoystickVirtualBall(joystick, ball, xrel, yrel)) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Set virtual joystick button value
     *
     * @param joystick The joystick
     * @param button The button index
     * @param down true for pressed, false for released
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> set_virtual_joystick_button(SDL_Joystick* joystick, int button, bool down) {
        if (!SDL_SetJoystickVirtualButton(joystick, button, down)) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Set virtual joystick hat value
     *
     * @param joystick The joystick
     * @param hat The hat index
     * @param position The hat position
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string>
    set_virtual_joystick_hat(SDL_Joystick* joystick, int hat, hat_position position) {
        if (!SDL_SetJoystickVirtualHat(joystick, hat, static_cast <uint8_t>(position))) {
            return make_unexpectedf(get_error());
        }
        return {};
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for power_state
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, power_state value);

/**
 * @brief Stream input operator for power_state
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, power_state& value);

/**
 * @brief Stream output operator for joystick_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, joystick_type value);

/**
 * @brief Stream input operator for joystick_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, joystick_type& value);

/**
 * @brief Stream output operator for joystick_connection_state
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, joystick_connection_state value);

/**
 * @brief Stream input operator for joystick_connection_state
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, joystick_connection_state& value);

/**
 * @brief Stream output operator for hat_position
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, hat_position value);

/**
 * @brief Stream input operator for hat_position
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, hat_position& value);

}