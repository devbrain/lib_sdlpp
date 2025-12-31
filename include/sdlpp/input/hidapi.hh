#pragma once

/**
 * @file hidapi.hh
 * @brief HID API functionality
 * 
 * This header provides C++ wrappers around SDL3's HID API, offering
 * low-level access to USB Human Interface Devices.
 * 
 * Note: The HID API can be disabled at SDL compile time with SDL_HIDAPI_DISABLED.
 * Use sdlpp::version_info::features::has_hidapi to check availability.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/core/version.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/type_utils.hh>
#include <sdlpp/detail/pointer.hh>



#include <string>
#include <vector>
#include <span>
#include <cstdint>
#include <memory>
#include <chrono>
#include <locale>
#include <codecvt>

namespace sdlpp {
#ifndef SDL_HIDAPI_DISABLED

// Suppress deprecation warnings for std::codecvt_utf8 and std::wstring_convert
// These are deprecated in C++17 but no standard replacement exists yet
#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable: 4996)
#endif

    namespace detail {
        /**
         * @brief Convert wide string to UTF-8 string
         * @param wide_str Wide string to convert
         * @return UTF-8 encoded string
         */
        inline std::string wstring_to_utf8(const std::wstring& wide_str) {
            if (wide_str.empty()) {
                return {};
            }

            // Use std::codecvt_utf8 for the conversion
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.to_bytes(wide_str);
        }

        /**
         * @brief Convert wide string to UTF-8 string (from C-style wide string)
         * @param wide_str Wide string pointer
         * @return UTF-8 encoded string
         */
        inline std::string wstring_to_utf8(const wchar_t* wide_str) {
            if (!wide_str) {
                return {};
            }
            return wstring_to_utf8(std::wstring(wide_str));
        }
    }

    /**
     * @brief HID bus types
     */
    enum class hid_bus_type : int {
        unknown = SDL_HID_API_BUS_UNKNOWN, // Unknown bus type
        usb = SDL_HID_API_BUS_USB, // USB bus
        bluetooth = SDL_HID_API_BUS_BLUETOOTH, // Bluetooth or Bluetooth LE bus
        i2c = SDL_HID_API_BUS_I2C, // I2C bus
        spi = SDL_HID_API_BUS_SPI // SPI bus
    };

    /**
     * @brief HID device information
     */
    struct hid_device_info {
        std::string path; // Platform-specific device path
        uint16_t vendor_id; // Device Vendor ID
        uint16_t product_id; // Device Product ID
        std::string serial_number; // Serial Number (UTF-8)
        uint16_t release_number; // Device Release Number (BCD)
        std::string manufacturer_string; // Manufacturer String (UTF-8)
        std::string product_string; // Product string (UTF-8)
        uint16_t usage_page; // Usage Page (Windows/Mac/hidraw only)
        uint16_t usage; // Usage (Windows/Mac/hidraw only)
        int interface_number; // USB interface number (-1 if not USB)
        int interface_class; // USB interface class
        int interface_subclass; // USB interface subclass
        int interface_protocol; // USB interface protocol
        hid_bus_type bus_type; // Underlying bus type
    };

    /**
     * @brief Smart pointer type for SDL_hid_device with automatic cleanup
     */
    namespace detail {
        struct hid_device_deleter {
            void operator()(SDL_hid_device* device) const noexcept {
                if (device) {
                    SDL_hid_close(device);
                }
            }
        };
    }

    using hid_device_ptr = std::unique_ptr <SDL_hid_device, detail::hid_device_deleter>;

    /**
     * @brief Initialize the HIDAPI library
     *
     * This function initializes the HIDAPI library. Calling it is not strictly
     * necessary, as it will be called automatically by enumerate() and open()
     * if needed. This function should be called at the beginning of execution
     * however, if there is a chance of HIDAPI handles being opened by different
     * threads simultaneously.
     *
     * Each call to this function should have a matching call to exit()
     *
     * @return Expected<void> - empty on success, error message on failure
     */
    [[nodiscard]] inline expected <void, std::string> hid_init() {
        if (SDL_hid_init() < 0) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Finalize the HIDAPI library
     *
     * This function frees all of the static data associated with HIDAPI.
     * It should be called at the end of execution to avoid memory leaks.
     *
     * @return Expected<void> - empty on success, error message on failure
     */
    [[nodiscard]] inline expected <void, std::string> hid_exit() {
        if (SDL_hid_exit() < 0) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Check to see if devices may have been added or removed
     *
     * Enumerating the HID devices is an expensive operation, so you can call
     * this to see if there have been any system device changes since the last
     * call to this function. A change in the counter returned doesn't necessarily
     * mean that anything has changed, but you can call enumerate() to get an
     * updated device list.
     *
     * Calling this function for the first time may cause a thread or other
     * system resource to be allocated to track device change notifications.
     *
     * @return Change counter that is incremented with each potential device change,
     *         or 0 if device change detection isn't available
     */
    [[nodiscard]] inline uint32_t hid_device_change_count() noexcept {
        return SDL_hid_device_change_count();
    }

    namespace detail {
        /**
         * @brief Convert SDL device info to our struct
         */
        inline hid_device_info convert_device_info(const SDL_hid_device_info* info) {
            hid_device_info result{};

            if (info->path) {
                result.path = info->path;
            }

            result.vendor_id = info->vendor_id;
            result.product_id = info->product_id;

            // Convert wide strings to UTF-8
            if (info->serial_number) {
                result.serial_number = wstring_to_utf8(info->serial_number);
            }

            result.release_number = info->release_number;

            if (info->manufacturer_string) {
                result.manufacturer_string = wstring_to_utf8(info->manufacturer_string);
            }

            if (info->product_string) {
                result.product_string = wstring_to_utf8(info->product_string);
            }

            result.usage_page = info->usage_page;
            result.usage = info->usage;
            result.interface_number = info->interface_number;
            result.interface_class = info->interface_class;
            result.interface_subclass = info->interface_subclass;
            result.interface_protocol = info->interface_protocol;
            result.bus_type = static_cast <hid_bus_type>(info->bus_type);

            return result;
        }
    }

    /**
     * @brief Enumerate HID devices
     *
     * This function returns a list of all the HID devices attached to the
     * system which match vendor_id and product_id. If vendor_id is set to 0
     * then any vendor matches. If product_id is set to 0 then any product
     * matches. If vendor_id and product_id are both set to 0, then all HID
     * devices will be returned.
     *
     * By default SDL will only enumerate controllers, to reduce risk of hanging
     * or crashing on bad drivers, but SDL_HINT_HIDAPI_ENUMERATE_ONLY_CONTROLLERS
     * can be set to "0" to enumerate all HID devices.
     *
     * @param vendor_id The Vendor ID (VID) to match, or 0 for any
     * @param product_id The Product ID (PID) to match, or 0 for any
     * @return Vector of device information structures
     */
    [[nodiscard]] inline std::vector <hid_device_info> hid_enumerate(uint16_t vendor_id = 0,
                                                                     uint16_t product_id = 0) {
        std::vector <hid_device_info> devices_list;

        SDL_hid_device_info* devs = SDL_hid_enumerate(vendor_id, product_id);
        if (!devs) {
            return devices_list;
        }

        // Convert linked list to vector
        for (SDL_hid_device_info* cur = devs; cur; cur = cur->next) {
            devices_list.push_back(detail::convert_device_info(cur));
        }

        SDL_hid_free_enumeration(devs);
        return devices_list;
    }

    /**
     * @brief RAII wrapper for SDL_hid_device
     *
     * This class provides a safe, RAII-managed interface to SDL's HID functionality.
     * The device is automatically closed when the object goes out of scope.
     */
    class hid_device {
        private:
            hid_device_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty device
             */
            hid_device() = default;

            /**
             * @brief Construct from existing SDL_hid_device pointer
             * @param device Existing SDL_hid_device pointer (takes ownership)
             */
            explicit hid_device(SDL_hid_device* device)
                : ptr(device) {
            }

            /**
             * @brief Move constructor
             */
            hid_device(hid_device&&) = default;

            /**
             * @brief Move assignment operator
             */
            hid_device& operator=(hid_device&&) = default;

            // Delete copy operations - devices are move-only
            hid_device(const hid_device&) = delete;
            hid_device& operator=(const hid_device&) = delete;

            /**
             * @brief Check if the device is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying SDL_hid_device pointer
             */
            [[nodiscard]] SDL_hid_device* get() const noexcept { return ptr.get(); }

            /**
             * @brief Open a HID device using VID, PID and optionally serial number
             *
             * If serial_number is empty, the first device with the specified VID and PID
             * is opened.
             *
             * @param vendor_id The Vendor ID (VID) of the device to open
             * @param product_id The Product ID (PID) of the device to open
             * @param serial_number The Serial Number of the device to open (UTF-8, optional)
             * @return Expected containing device, or error message
             */
            static expected <hid_device, std::string> open(uint16_t vendor_id,
                                                           uint16_t product_id,
                                                           const std::string& serial_number = {}) {
                // Convert UTF-8 string to wide string for SDL
                std::wstring wide_serial;
                const wchar_t* serial = nullptr;
                
                if (!serial_number.empty()) {
                    // Convert UTF-8 to wide string
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    try {
                        wide_serial = converter.from_bytes(serial_number);
                        serial = wide_serial.c_str();
                    } catch (const std::range_error&) {
                        return make_unexpectedf("Invalid UTF-8 in serial number");
                    }
                }
                
                auto* dev = SDL_hid_open(vendor_id, product_id, serial);
                if (!dev) {
                    return make_unexpectedf(get_error());
                }
                return hid_device(dev);
            }

            /**
             * @brief Open a HID device by its path name
             *
             * The path name can be determined by calling enumerate(), or a
             * platform-specific path name can be used (eg: /dev/hidraw0 on Linux).
             *
             * @param path The path name of the device to open
             * @return Expected containing device, or error message
             */
            static expected <hid_device, std::string> open_path(const std::string& path) {
                auto* dev = SDL_hid_open_path(path.c_str());
                if (!dev) {
                    return make_unexpectedf(get_error());
                }
                return hid_device(dev);
            }

            /**
             * @brief Write an Output report to the device
             *
             * The first byte of data must contain the Report ID. For devices which only
             * support a single report, this must be set to 0x0.
             *
             * @param data The data to send, including the report number as the first byte
             * @return Expected containing number of bytes written, or error message
             */
            [[nodiscard]] expected <size_t, std::string> write(std::span <const uint8_t> data) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }
                
                auto int_size = detail::size_to_int(data.size());
                if (!int_size) {
                    return make_unexpectedf("Data size too large:", int_size.error());
                }
                
                int bytes_written = SDL_hid_write(ptr.get(), data.data(), static_cast<size_t>(*int_size));
                if (bytes_written < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(bytes_written);
            }

            /**
             * @brief Read an Input report from the device with timeout
             *
             * Input reports are returned to the host through the INTERRUPT IN endpoint.
             * The first byte will contain the Report number if the device uses numbered
             * reports.
             *
             * @param buffer Buffer to store the read data
             * @param timeout Timeout duration, or a negative duration for blocking wait
             * @return Expected containing number of bytes read, or error message
             */
            template<typename Rep, typename Period>
            expected <size_t, std::string> read_timeout(std::span <uint8_t> buffer,
                                                        std::chrono::duration <Rep, Period> timeout) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }

                auto int_size = detail::size_to_int(buffer.size());
                if (!int_size) {
                    return make_unexpectedf("Buffer size too large:", int_size.error());
                }

                // Convert to milliseconds
                auto ms = std::chrono::duration_cast <std::chrono::milliseconds>(timeout).count();
                int timeout_ms = timeout.count() < 0 ? -1 : static_cast <int>(ms);

                int bytes_read = SDL_hid_read_timeout(ptr.get(), buffer.data(), static_cast<size_t>(*int_size), timeout_ms);
                if (bytes_read < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(bytes_read);
            }

            /**
             * @brief Read an Input report from the device
             *
             * Input reports are returned to the host through the INTERRUPT IN endpoint.
             * The first byte will contain the Report number if the device uses numbered
             * reports.
             *
             * @param buffer Buffer to store the read data
             * @return Expected containing number of bytes read, or error message
             */
            [[nodiscard]] expected <size_t, std::string> read(std::span <uint8_t> buffer) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }
                
                auto int_size = detail::size_to_int(buffer.size());
                if (!int_size) {
                    return make_unexpectedf("Buffer size too large:", int_size.error());
                }
                
                int bytes_read = SDL_hid_read(ptr.get(), buffer.data(), static_cast<size_t>(*int_size));
                if (bytes_read < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(bytes_read);
            }

            /**
             * @brief Set the device handle to be non-blocking
             *
             * In non-blocking mode calls to read() will return immediately with a
             * value of 0 if there is no data to be read. In blocking mode, read()
             * will wait (block) until there is data to read before returning.
             *
             * Nonblocking can be turned on and off at any time.
             *
             * @param nonblock true to enable non-blocking, false to disable
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_nonblocking(bool nonblock) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }
                if (SDL_hid_set_nonblocking(ptr.get(), nonblock ? 1 : 0) < 0) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Send a Feature report to the device
             *
             * Feature reports are sent over the Control endpoint as a Set_Report
             * transfer. The first byte of data must contain the Report ID.
             *
             * @param data The data to send, including the report number as the first byte
             * @return Expected containing number of bytes written, or error message
             */
            [[nodiscard]] expected <size_t, std::string> send_feature_report(std::span <const uint8_t> data) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }
                
                auto int_size = detail::size_to_int(data.size());
                if (!int_size) {
                    return make_unexpectedf("Data size too large:", int_size.error());
                }
                
                int bytes_sent = SDL_hid_send_feature_report(ptr.get(), data.data(), static_cast<size_t>(*int_size));
                if (bytes_sent < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(bytes_sent);
            }

            /**
             * @brief Get a feature report from the device
             *
             * Set the first byte of buffer to the Report ID of the report to be read.
             * Upon return, the first byte will still contain the Report ID, and the
             * report data will start in buffer[1].
             *
             * @param buffer Buffer to store the report data, with Report ID in first byte
             * @return Expected containing number of bytes read, or error message
             */
            [[nodiscard]] expected <size_t, std::string> get_feature_report(std::span <uint8_t> buffer) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }
                
                auto int_size = detail::size_to_int(buffer.size());
                if (!int_size) {
                    return make_unexpectedf("Buffer size too large:", int_size.error());
                }
                
                int bytes_received = SDL_hid_get_feature_report(ptr.get(), buffer.data(), static_cast<size_t>(*int_size));
                if (bytes_received < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(bytes_received);
            }

            /**
             * @brief Get an input report from the device
             *
             * Set the first byte of buffer to the Report ID of the report to be read.
             * Upon return, the first byte will still contain the Report ID, and the
             * report data will start in buffer[1].
             *
             * @param buffer Buffer to store the report data, with Report ID in first byte
             * @return Expected containing number of bytes read, or error message
             */
            [[nodiscard]] expected <size_t, std::string> get_input_report(std::span <uint8_t> buffer) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }
                
                auto int_size = detail::size_to_int(buffer.size());
                if (!int_size) {
                    return make_unexpectedf("Buffer size too large:", int_size.error());
                }
                
                int bytes_received = SDL_hid_get_input_report(ptr.get(), buffer.data(), static_cast<size_t>(*int_size));
                if (bytes_received < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(bytes_received);
            }

            /**
             * @brief Get the manufacturer string from the device
             *
             * @return Expected containing manufacturer string (UTF-8), or error message
             */
            [[nodiscard]] expected <std::string, std::string> get_manufacturer_string() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }

                std::wstring str_buffer(256, L'\0');
                if (SDL_hid_get_manufacturer_string(ptr.get(), str_buffer.data(), str_buffer.size()) < 0) {
                    return make_unexpectedf(get_error());
                }

                // Trim to actual length
                size_t len = std::wcslen(str_buffer.c_str());
                str_buffer.resize(len);
                return detail::wstring_to_utf8(str_buffer);
            }

            /**
             * @brief Get the product string from the device
             *
             * @return Expected containing product string (UTF-8), or error message
             */
            [[nodiscard]] expected <std::string, std::string> get_product_string() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }

                std::wstring str_buffer(256, L'\0');
                if (SDL_hid_get_product_string(ptr.get(), str_buffer.data(), str_buffer.size()) < 0) {
                    return make_unexpectedf(get_error());
                }

                // Trim to actual length
                size_t len = std::wcslen(str_buffer.c_str());
                str_buffer.resize(len);
                return detail::wstring_to_utf8(str_buffer);
            }

            /**
             * @brief Get the serial number string from the device
             *
             * @return Expected containing serial number string (UTF-8), or error message
             */
            [[nodiscard]] expected <std::string, std::string> get_serial_number_string() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }

                std::wstring str_buffer(256, L'\0');
                if (SDL_hid_get_serial_number_string(ptr.get(), str_buffer.data(), str_buffer.size()) < 0) {
                    return make_unexpectedf(get_error());
                }

                // Trim to actual length
                size_t len = std::wcslen(str_buffer.c_str());
                str_buffer.resize(len);
                return detail::wstring_to_utf8(str_buffer);
            }

            /**
             * @brief Get a string from the device, based on its string index
             *
             * @param string_index The index of the string to get
             * @return Expected containing indexed string (UTF-8), or error message
             */
            [[nodiscard]] expected <std::string, std::string> get_indexed_string(int string_index) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }

                std::wstring str_buffer(256, L'\0');
                if (SDL_hid_get_indexed_string(ptr.get(), string_index, str_buffer.data(), str_buffer.size()) < 0) {
                    return make_unexpectedf(get_error());
                }

                // Trim to actual length
                size_t len = std::wcslen(str_buffer.c_str());
                str_buffer.resize(len);
                return detail::wstring_to_utf8(str_buffer);
            }

            /**
             * @brief Get device information
             *
             * @return Expected containing device info, or error message
             */
            [[nodiscard]] expected <hid_device_info, std::string> get_device_info() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }

                SDL_hid_device_info* info = SDL_hid_get_device_info(ptr.get());
                if (!info) {
                    return make_unexpectedf(get_error());
                }

                return detail::convert_device_info(info);
            }

            /**
             * @brief Get a report descriptor from the device
             *
             * @param buffer Buffer to store the descriptor (recommended size: 4096 bytes)
             * @return Expected containing number of bytes copied, or error message
             */
            [[nodiscard]] expected <size_t, std::string> get_report_descriptor(std::span <uint8_t> buffer) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid device");
                }
                
                auto int_size = detail::size_to_int(buffer.size());
                if (!int_size) {
                    return make_unexpectedf("Buffer size too large:", int_size.error());
                }

                int bytes_copied = SDL_hid_get_report_descriptor(ptr.get(), buffer.data(), static_cast<size_t>(*int_size));
                if (bytes_copied < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(bytes_copied);
            }
    };

    /**
     * @brief Start or stop a BLE scan on iOS and tvOS to pair Steam Controllers
     *
     * @param active true to start the scan, false to stop the scan
     */
    inline void hid_ble_scan(bool active) noexcept {
        SDL_hid_ble_scan(active);
    }

    /**
     * @brief RAII helper for HID API initialization
     *
     * This class ensures hid_init() is called on construction and
     * hid_exit() is called on destruction.
     */
    class hid_context {
        public:
            /**
             * @brief Initialize HID API
             * @throws std::runtime_error if initialization fails
             */
            hid_context() {
                auto init_result = hid_init();
                if (!init_result) {
                    throw std::runtime_error("Failed to initialize HID API: " + init_result.error());
                }
            }

            /**
             * @brief Cleanup HID API
             */
            ~hid_context() noexcept {
                auto exit_result = hid_exit();
                (void)exit_result; // Ignore errors in destructor
            }

            // Non-copyable, non-movable
            hid_context(const hid_context&) = delete;
            hid_context& operator=(const hid_context&) = delete;
            hid_context(hid_context&&) = delete;
            hid_context& operator=(hid_context&&) = delete;
    };

#else // SDL_HIDAPI_DISABLED

// Provide stub types when HID API is disabled
enum class hid_bus_type : int {
    unknown = 0,
    usb = 1,
    bluetooth = 2,
    i2c = 3,
    spi = 4
};

struct hid_device_info {
    std::string path;
    uint16_t vendor_id;
    uint16_t product_id;
    std::string serial_number;
    uint16_t release_number;
    std::string manufacturer_string;
    std::string product_string;
    uint16_t usage_page;
    uint16_t usage;
    int interface_number;
    int interface_class;
    int interface_subclass;
    int interface_protocol;
    hid_bus_type bus_type;
};

class hid_device {
public:
    [[nodiscard]] bool is_valid() const noexcept { return false; }
    [[nodiscard]] explicit operator bool() const noexcept { return false; }
};

// Stub functions that return errors
[[nodiscard]] inline expected<void, std::string> hid_init() {
    return make_unexpectedf("HID API is disabled in this SDL build");
}

[[nodiscard]] inline expected<void, std::string> hid_exit() {
    return make_unexpectedf("HID API is disabled in this SDL build");
}

[[nodiscard]] inline uint32_t hid_device_change_count() noexcept {
    return 0;
}

[[nodiscard]] inline std::vector<hid_device_info> hid_enumerate(uint16_t = 0, uint16_t = 0) {
    return {};
}

inline void hid_ble_scan(bool) noexcept {}


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for hid_bus_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, hid_bus_type value);

/**
 * @brief Stream input operator for hid_bus_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, hid_bus_type& value);

}

#if defined(__clang__)
# pragma clang diagnostic pop
#elif defined(__GNUC__)
# pragma GCC diagnostic pop
#elif defined(_MSC_VER)
# pragma warning(pop)
#endif

#endif // SDL_HIDAPI_DISABLED
} // namespace sdlpp
