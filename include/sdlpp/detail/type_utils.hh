#pragma once

/**
 * @file type_utils.hh
 * @brief Type conversion utilities for SDL++ library
 * 
 * This header provides safe type conversion utilities, especially for
 * converting between size_t and int when interfacing with SDL's C API.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/expected.hh>
#include <limits>
#include <type_traits>
#include <string>

namespace sdlpp::detail {
    /**
     * @brief Safely convert between numeric types with bounds checking
     * 
     * @tparam To Target type
     * @tparam From Source type
     * @param value Value to convert
     * @return Expected containing converted value or error message
     */
    template<typename To, typename From>
    [[nodiscard]] constexpr expected<To, std::string> safe_numeric_cast(From value) {
        // Handle sign differences
        if constexpr (std::is_signed_v<From> && !std::is_signed_v<To>) {
            if (value < 0) {
                return make_unexpectedf("Cannot convert negative value to unsigned type");
            }
        }
        
        // Check upper bounds
        if constexpr (std::is_integral_v<From> && std::is_integral_v<To>) {
            if constexpr (sizeof(From) > sizeof(To) || 
                         (sizeof(From) == sizeof(To) && std::is_signed_v<To> && !std::is_signed_v<From>)) {
                if (value > static_cast<From>(std::numeric_limits<To>::max())) {
                    return make_unexpectedf("Value", std::to_string(value), "too large for target type");
                }
            }
        }
        
        return static_cast<To>(value);
    }
    
    /**
     * @brief Convert size_t to int (common pattern for SDL APIs)
     * 
     * @param value size_t value to convert
     * @return Expected containing int value or error message
     */
    [[nodiscard]] inline expected<int, std::string> size_to_int(size_t value) {
        return safe_numeric_cast<int>(value);
    }
    
    /**
     * @brief Convert size_t to Sint32 for SDL APIs
     * 
     * @param value size_t value to convert
     * @return Expected containing Sint32 value or error message
     */
    [[nodiscard]] inline expected<Sint32, std::string> size_to_sint32(size_t value) {
        return safe_numeric_cast<Sint32>(value);
    }
    
    /**
     * @brief Convert int to size_t with negative check
     * 
     * @param value int value to convert
     * @return Expected containing size_t value or error message
     */
    [[nodiscard]] inline expected<size_t, std::string> int_to_size(int value) {
        if (value < 0) {
            return make_unexpectedf("Cannot convert negative value", std::to_string(value), "to size_t");
        }
        return static_cast<size_t>(value);
    }
    
    /**
     * @brief Clamp size_t to int max for SDL APIs that require int
     * 
     * This is useful when you want to process as much as possible
     * even if the full size can't be handled in one call.
     * 
     * @param value size_t value to clamp
     * @return int value clamped to INT_MAX
     */
    [[nodiscard]] inline constexpr int clamp_size_to_int(size_t value) noexcept {
        return value > static_cast<size_t>(std::numeric_limits<int>::max()) 
            ? std::numeric_limits<int>::max() 
            : static_cast<int>(value);
    }
} // namespace sdlpp::detail