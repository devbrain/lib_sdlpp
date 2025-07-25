/**
 * @file io_common.hh
 * @brief Common I/O types and enumerations
 */
#pragma once

#include <string>

namespace sdlpp {
    /**
     * @brief File open modes for I/O operations
     *
     * These modes correspond to standard C fopen() modes but provide
     * type safety and better documentation.
     */
    enum class file_mode {
        read, // "r"  - Open for reading (file must exist)
        write, // "w"  - Open for writing (truncates existing file)
        append, // "a"  - Open for appending
        read_update, // "r+" - Open for reading and writing (file must exist)
        write_update, // "w+" - Open for reading and writing (truncates existing file)
        append_update, // "a+" - Open for reading and appending
        read_binary, // "rb" - Open for reading in binary mode
        write_binary, // "wb" - Open for writing in binary mode
        append_binary, // "ab" - Open for appending in binary mode
        read_update_binary, // "r+b" or "rb+" - Open for reading and writing in binary mode
        write_update_binary, // "w+b" or "wb+" - Open for reading and writing in binary mode (truncates)
        append_update_binary // "a+b" or "ab+" - Open for reading and appending in binary mode
    };

    /**
     * @brief Convert file_mode enum to C string for fopen/SDL functions
     * @param mode The file mode enum value
     * @return C string representation of the mode
     */
    [[nodiscard]] inline const char* to_string(file_mode mode) noexcept {
        switch (mode) {
            case file_mode::read: return "r";
            case file_mode::write: return "w";
            case file_mode::append: return "a";
            case file_mode::read_update: return "r+";
            case file_mode::write_update: return "w+";
            case file_mode::append_update: return "a+";
            case file_mode::read_binary: return "rb";
            case file_mode::write_binary: return "wb";
            case file_mode::append_binary: return "ab";
            case file_mode::read_update_binary: return "r+b";
            case file_mode::write_update_binary: return "w+b";
            case file_mode::append_update_binary: return "a+b";
        }
        return "r"; // Default to read mode
    }

    /**
     * @brief Check if a file mode is for reading
     * @param mode The file mode to check
     * @return true if the mode allows reading
     */
    [[nodiscard]] inline bool is_read_mode(file_mode mode) noexcept {
        switch (mode) {
            case file_mode::read:
            case file_mode::read_update:
            case file_mode::write_update:
            case file_mode::append_update:
            case file_mode::read_binary:
            case file_mode::read_update_binary:
            case file_mode::write_update_binary:
            case file_mode::append_update_binary:
                return true;
            default:
                return false;
        }
    }

    /**
     * @brief Check if a file mode is for writing
     * @param mode The file mode to check
     * @return true if the mode allows writing
     */
    [[nodiscard]] inline bool is_write_mode(file_mode mode) noexcept {
        switch (mode) {
            case file_mode::write:
            case file_mode::append:
            case file_mode::read_update:
            case file_mode::write_update:
            case file_mode::append_update:
            case file_mode::write_binary:
            case file_mode::append_binary:
            case file_mode::read_update_binary:
            case file_mode::write_update_binary:
            case file_mode::append_update_binary:
                return true;
            default:
                return false;
        }
    }

    /**
     * @brief Check if a file mode is binary
     * @param mode The file mode to check
     * @return true if the mode is binary
     */
    [[nodiscard]] inline bool is_binary_mode(file_mode mode) noexcept {
        switch (mode) {
            case file_mode::read_binary:
            case file_mode::write_binary:
            case file_mode::append_binary:
            case file_mode::read_update_binary:
            case file_mode::write_update_binary:
            case file_mode::append_update_binary:
                return true;
            default:
                return false;
        }
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>
#include <sdlpp/detail/export.hh>
namespace sdlpp {
/**
 * @brief Stream output operator for file_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, file_mode value);

/**
 * @brief Stream input operator for file_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, file_mode& value);

}