#pragma once

/**
 * @file guid.hh
 * @brief GUID (Globally Unique Identifier) support
 * 
 * This header provides a C++ wrapper for SDL's GUID functionality,
 * primarily used for identifying input devices like joysticks and gamepads.
 */

#include <sdlpp/core/sdl.hh>
#include <string>
#include <array>
#include <cstring>
#include <optional>
#include <compare>

namespace sdlpp {
    /**
     * @brief GUID (Globally Unique Identifier) wrapper
     *
     * A 128-bit identifier used to uniquely identify input devices.
     * GUIDs are platform-dependent - the same device may have different
     * GUIDs on different operating systems.
     */
    class guid {
        public:
            /**
             * @brief Default constructor - creates a zero GUID
             */
            constexpr guid() noexcept
                : data_{} {
            }

            /**
             * @brief Construct from SDL_GUID
             * @param sdl_guid SDL GUID structure
             */
            explicit guid(const SDL_GUID& sdl_guid) noexcept {
                std::memcpy(data_.data(), sdl_guid.data, 16);
            }

            /**
             * @brief Construct from raw data
             * @param data 16-byte array of GUID data
             */
            explicit guid(const std::array <Uint8, 16>& data) noexcept
                : data_(data) {
            }

            /**
             * @brief Construct from string representation
             * @param str String representation of GUID (32 hex characters)
             * @return Optional containing the GUID if parsing succeeded
             *
             * @note The string should be 32 hexadecimal characters without separators.
             *       Invalid strings will return nullopt.
             *
             * Example:
             * @code
             * auto g = sdlpp::guid::from_string("030000005e040000ea02000000000000");
             * if (g) {
             *     // Use the GUID
             * }
             * @endcode
             */
            [[nodiscard]] static std::optional <guid> from_string(const std::string& str) noexcept {
                if (str.length() != 32) {
                    return std::nullopt;
                }

                // Validate that all characters are hex
                for (char c : str) {
                    if (!((c >= '0' && c <= '9') ||
                          (c >= 'a' && c <= 'f') ||
                          (c >= 'A' && c <= 'F'))) {
                        return std::nullopt;
                    }
                }

                SDL_GUID sdl_guid = SDL_StringToGUID(str.c_str());
                return guid(sdl_guid);
            }

            /**
             * @brief Convert to SDL_GUID
             * @return SDL GUID structure
             */
            [[nodiscard]] SDL_GUID to_sdl() const noexcept {
                SDL_GUID sdl_guid;
                std::memcpy(sdl_guid.data, data_.data(), 16);
                return sdl_guid;
            }

            /**
             * @brief Convert to string representation
             * @return String representation (32 lowercase hex characters)
             *
             * Example:
             * @code
             * sdlpp::guid g(...);
             * std::string str = g.to_string();  // "030000005e040000ea02000000000000"
             * @endcode
             */
            [[nodiscard]] std::string to_string() const {
                char buffer[33]; // 32 hex chars + null terminator
                SDL_GUIDToString(to_sdl(), buffer, sizeof(buffer));
                return std::string(buffer);
            }

            /**
             * @brief Get raw data
             * @return Reference to the 16-byte data array
             */
            [[nodiscard]] const std::array <Uint8, 16>& data() const noexcept {
                return data_;
            }

            /**
             * @brief Get mutable raw data
             * @return Reference to the 16-byte data array
             */
            [[nodiscard]] std::array <Uint8, 16>& data() noexcept {
                return data_;
            }

            /**
             * @brief Check if this is a zero GUID
             * @return true if all bytes are zero
             */
            [[nodiscard]] bool is_zero() const noexcept {
                for (Uint8 byte : data_) {
                    if (byte != 0) {
                        return false;
                    }
                }
                return true;
            }

            /**
             * @brief Check if this is a valid (non-zero) GUID
             * @return true if at least one byte is non-zero
             */
            [[nodiscard]] bool is_valid() const noexcept {
                return !is_zero();
            }

            /**
             * @brief Three-way comparison operator
             * @param other Other GUID to compare
             * @return Ordering relationship between this and other
             */
            [[nodiscard]] auto operator<=>(const guid& other) const noexcept = default;

            /**
             * @brief Equality operator
             * @param other Other GUID to compare
             * @return true if GUIDs are equal
             */
            [[nodiscard]] bool operator==(const guid& other) const noexcept = default;

            /**
             * @brief Create a zero GUID
             * @return A GUID with all bytes set to zero
             */
            [[nodiscard]] static constexpr guid zero() noexcept {
                return guid{};
            }

        private:
            std::array <Uint8, 16> data_;
    };

    /**
     * @brief GUID information extracted from joystick GUID
     *
     * Contains vendor ID, product ID, version, and CRC16 checksum
     * extracted from a joystick GUID.
     */
    struct guid_info {
        Uint16 vendor{0}; ///< USB vendor ID
        Uint16 product{0}; ///< USB product ID
        Uint16 version{0}; ///< Product version
        Uint16 crc16{0}; ///< CRC16 checksum of device name

        /**
         * @brief Check if the info contains valid data
         * @return true if at least vendor or product ID is non-zero
         */
        [[nodiscard]] bool is_valid() const noexcept {
            return vendor != 0 || product != 0;
        }
    };

    /**
     * @brief Get joystick GUID information
     *
     * Extracts vendor ID, product ID, version, and CRC16 from a joystick GUID.
     * This information can be used to identify specific device models.
     *
     * @param g The GUID to extract information from
     * @return GUID information structure
     *
     * @note Not all GUIDs contain this information. Check is_valid() on the result.
     *
     * Example:
     * @code
     * auto info = sdlpp::get_joystick_guid_info(device_guid);
     * if (info.is_valid()) {
     *     std::cout << "Vendor: " << std::hex << info.vendor << "\n";
     *     std::cout << "Product: " << std::hex << info.product << "\n";
     * }
     * @endcode
     */
    [[nodiscard]] inline guid_info get_joystick_guid_info(const guid& g) noexcept {
        guid_info info;
        SDL_GetJoystickGUIDInfo(g.to_sdl(), &info.vendor, &info.product,
                                &info.version, &info.crc16);
        return info;
    }

    /**
     * @brief Stream output operator for GUID
     * @param os Output stream
     * @param g GUID to output
     * @return Reference to the output stream
     */
    inline std::ostream& operator<<(std::ostream& os, const guid& g) {
        return os << g.to_string();
    }
} // namespace sdlpp

// std::hash specialization for guid
namespace std {
    template<>
    struct hash <sdlpp::guid> {
        [[nodiscard]] size_t operator()(const sdlpp::guid& g) const noexcept {
            // Simple hash combining all bytes
            size_t hash_value = 0;
            const auto& data = g.data();
            for (unsigned char i : data) {
                hash_value = (hash_value << 8) | (hash_value >> (sizeof(size_t) * 8 - 8));
                hash_value ^= i;
            }
            return hash_value;
        }
    };
}
