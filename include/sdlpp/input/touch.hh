#pragma once

/**
 * @file touch.hh
 * @brief Touch input functionality
 * 
 * This header provides C++ wrappers around SDL3's touch API, offering
 * multi-touch support for touchscreens and trackpads.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>

#include <string>
#include <vector>
#include <cstdint>

namespace sdlpp {
    /**
     * @brief Touch device ID type
     */
    using touch_id = SDL_TouchID;

    /**
     * @brief Touch finger ID type
     */
    using finger_id = SDL_FingerID;

    /**
     * @brief Touch device type
     */
    enum class touch_device_type : int {
        invalid = SDL_TOUCH_DEVICE_INVALID,
        direct = SDL_TOUCH_DEVICE_DIRECT, // Touch screen with window-relative coordinates
        indirect_absolute = SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE, // Trackpad with absolute device coordinates
        indirect_relative = SDL_TOUCH_DEVICE_INDIRECT_RELATIVE // Trackpad with screen cursor-relative coordinates
    };

    /**
     * @brief Finger information
     */
    struct finger {
        finger_id id = 0;
        float x = 0.0f; // Normalized (0...1)
        float y = 0.0f; // Normalized (0...1)
        float pressure = 0.0f; // Normalized (0...1)
    };

    /**
     * @brief Get all touch devices
     *
     * @return Vector of touch device IDs
     */
    [[nodiscard]] inline std::vector <touch_id> get_touch_devices() {
        int count = 0;
        const SDL_TouchID* devices = SDL_GetTouchDevices(&count);
        if (!devices || count <= 0) {
            return {};
        }
        return std::vector <touch_id>(devices, devices + count);
    }

    /**
     * @brief Get the name of a touch device
     *
     * @param touchID Touch device ID
     * @return Device name, or empty string if not found
     */
    [[nodiscard]] inline std::string get_touch_device_name(touch_id touchID) {
        const char* name = SDL_GetTouchDeviceName(touchID);
        return name ? name : "";
    }

    /**
     * @brief Get the type of a touch device
     *
     * @param touchID Touch device ID
     * @return Touch device type
     */
    [[nodiscard]] inline touch_device_type get_touch_device_type(touch_id touchID) {
        return static_cast <touch_device_type>(SDL_GetTouchDeviceType(touchID));
    }

    /**
     * @brief Get all active fingers on a touch device
     *
     * @param touchID Touch device ID
     * @return Vector of finger information
     */
    [[nodiscard]] inline std::vector <finger> get_touch_fingers(touch_id touchID) {
        int count = 0;
        SDL_Finger** fingers = SDL_GetTouchFingers(touchID, &count);
        if (!fingers || count <= 0) {
            return {};
        }

        std::vector <finger> fingers_list;
        fingers_list.reserve(static_cast <std::size_t>(count));

        for (int i = 0; i < count; ++i) {
            if (fingers[i]) {
                fingers_list.push_back({
                    .id = fingers[i]->id,
                    .x = fingers[i]->x,
                    .y = fingers[i]->y,
                    .pressure = fingers[i]->pressure
                });
            }
        }

        SDL_free(fingers);
        return fingers_list;
    }

    /**
     * @brief Touch input helper class
     *
     * This class provides convenient access to touch device state.
     */
    class touch_state {
        private:
            touch_id device_id;

        public:
            /**
             * @brief Construct touch state helper
             *
             * @param id Touch device ID
             */
            explicit touch_state(touch_id id) noexcept
                : device_id(id) {
            }

            /**
             * @brief Get the device ID
             *
             * @return Touch device ID
             */
            [[nodiscard]] touch_id get_device_id() const noexcept { return device_id; }

            /**
             * @brief Get the device name
             *
             * @return Device name
             */
            [[nodiscard]] std::string get_name() const {
                return get_touch_device_name(device_id);
            }

            /**
             * @brief Get the device type
             *
             * @return Device type
             */
            [[nodiscard]] touch_device_type get_type() const {
                return get_touch_device_type(device_id);
            }

            /**
             * @brief Check if this is a direct touch device (touchscreen)
             *
             * @return true if direct touch device
             */
            [[nodiscard]] bool is_direct() const noexcept {
                return get_type() == touch_device_type::direct;
            }

            /**
             * @brief Check if this is an indirect touch device (trackpad)
             *
             * @return true if indirect touch device
             */
            [[nodiscard]] bool is_indirect() const noexcept {
                auto type = get_type();
                return type == touch_device_type::indirect_absolute ||
                       type == touch_device_type::indirect_relative;
            }

            /**
             * @brief Get all active fingers
             *
             * @return Vector of finger information
             */
            [[nodiscard]] std::vector <finger> get_fingers() const {
                return get_touch_fingers(device_id);
            }

            /**
             * @brief Get the number of active fingers
             *
             * @return Number of fingers currently touching
             */
            [[nodiscard]] size_t get_num_fingers() const {
                return get_fingers().size();
            }

            /**
             * @brief Get finger by index
             *
             * @param index Finger index
             * @return Finger information, or nullopt if invalid
             */
            [[nodiscard]] std::optional <finger> get_finger(size_t index) const {
                auto fingers = get_fingers();
                if (index < fingers.size()) {
                    return fingers[index];
                }
                return std::nullopt;
            }

            /**
             * @brief Find finger by ID
             *
             * @param id Finger ID
             * @return Finger information, or nullopt if not found
             */
            [[nodiscard]] std::optional <finger> find_finger(finger_id id) const {
                auto fingers = get_fingers();
                for (const auto& f : fingers) {
                    if (f.id == id) {
                        return f;
                    }
                }
                return std::nullopt;
            }

            /**
             * @brief Check if any fingers are touching
             *
             * @return true if at least one finger is touching
             */
            [[nodiscard]] bool has_touch() const noexcept {
                return !get_fingers().empty();
            }

            /**
             * @brief Get the primary finger (first finger)
             *
             * @return Primary finger information, or nullopt if no touch
             */
            [[nodiscard]] std::optional <finger> get_primary_finger() const {
                auto fingers = get_fingers();
                if (!fingers.empty()) {
                    return fingers[0];
                }
                return std::nullopt;
            }
    };

    /**
     * @brief Get all active touch states
     *
     * @return Vector of touch state helpers for all devices
     */
    [[nodiscard]] inline std::vector <touch_state> get_all_touch_states() {
        auto devices = get_touch_devices();
        std::vector <touch_state> states;
        states.reserve(devices.size());

        for (auto id : devices) {
            states.emplace_back(id);
        }

        return states;
    }
} // namespace sdlpp

// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
    /**
     * @brief Stream output operator for touch_device_type
     */
    SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, touch_device_type value);

    /**
     * @brief Stream input operator for touch_device_type
     */
    SDLPP_EXPORT std::istream& operator>>(std::istream& is, touch_device_type& value);
}
