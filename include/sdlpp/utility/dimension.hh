//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file dimension.hh
 * @brief Type-safe dimensions that enforce non-negative values
 */

#include <limits>
#include <cstdint>
#include <type_traits>
#include <algorithm>
#include <concepts>

namespace sdlpp {
    /**
     * @brief Concept for types that can be used as dimension values
     */
    template<typename T>
    concept dimensional = std::is_arithmetic_v <T> && !std::is_same_v <T, bool>;

    /**
     * @brief Concept for types that can represent non-negative dimensions
     */
    template<typename T>
    concept non_negative_dimension = requires(T t)
    {
        requires dimensional <decltype(t.value())>;
        { t.is_zero() } -> std::same_as <bool>;
        { t.is_positive() } -> std::same_as <bool>;
    };

    /**
     * @brief Concept for types that can represent coordinates (can be negative)
     */
    template<typename T>
    concept coordinate_like = requires(T t)
    {
        t.value;
        requires dimensional <decltype(t.value)>;
    };

    /**
     * @brief Concept for 2D dimension types
     */
    template<typename T>
    concept dimensions_like = requires(T t)
    {
        requires non_negative_dimension <decltype(t.width)>;
        requires non_negative_dimension <decltype(t.height)>;
        { t.area() }; // Can return any numeric type
        { t.is_empty() } -> std::same_as <bool>;
        { t.is_valid() } -> std::same_as <bool>;
    };

    /**
     * @brief Concept for 2D position types
     */
    template<typename T>
    concept position_like = requires(T t)
    {
        requires coordinate_like <decltype(t.x)>;
        requires coordinate_like <decltype(t.y)>;
    };

    /**
     * @brief A dimension value that is guaranteed to be non-negative
     *
     * This type enforces at compile-time and construction-time that
     * dimensions cannot be negative, making invalid states unrepresentable.
     */
    template<dimensional T = int>
    class dimension {
        private:
            T value_;

        public:
            /**
             * @brief Construct from a value, clamping negative values to 0
             */
            constexpr explicit dimension(T val) noexcept
                : value_(std::max(T{0}, val)) {
            }

            /**
             * @brief Default construct to 0
             */
            constexpr dimension() noexcept
                : value_(0) {
            }

            /**
             * @brief Get the underlying value
             */
            [[nodiscard]] constexpr T value() const noexcept { return value_; }

            /**
             * @brief Implicit conversion to underlying type for SDL compatibility
             */
            [[nodiscard]] constexpr operator T() const noexcept { return value_; }

            /**
             * @brief Check if dimension is zero
             */
            [[nodiscard]] constexpr bool is_zero() const noexcept { return value_ == 0; }

            /**
             * @brief Check if dimension is positive (non-zero)
             */
            [[nodiscard]] constexpr bool is_positive() const noexcept { return value_ > 0; }

            // Arithmetic operations that maintain non-negative invariant

            constexpr dimension& operator+=(const dimension& other) noexcept {
                // Check for overflow before addition
                if (value_ > std::numeric_limits <T>::max() - other.value_) {
                    value_ = std::numeric_limits <T>::max();
                } else {
                    value_ += other.value_;
                }
                return *this;
            }

            constexpr dimension& operator-=(const dimension& other) noexcept {
                value_ = (value_ > other.value_) ? value_ - other.value_ : T{0};
                return *this;
            }

            constexpr dimension& operator*=(T scalar) noexcept {
                if (scalar <= 0) {
                    value_ = 0;
                } else {
                    // Prevent overflow
                    if (value_ > 0 && scalar > std::numeric_limits <T>::max() / value_) {
                        value_ = std::numeric_limits <T>::max();
                    } else {
                        value_ *= scalar;
                    }
                }
                return *this;
            }

            constexpr dimension& operator/=(T scalar) noexcept {
                if (scalar > 0) {
                    value_ /= scalar;
                }
                // Division by zero or negative leaves value unchanged
                return *this;
            }

            // Comparison operators
            auto operator<=>(const dimension&) const = default;
    };

    // Arithmetic operators
    template<typename T>
    [[nodiscard]] constexpr dimension <T> operator+(dimension <T> a, dimension <T> b) noexcept {
        a += b;
        return a;
    }

    template<typename T>
    [[nodiscard]] constexpr dimension <T> operator-(dimension <T> a, dimension <T> b) noexcept {
        a -= b;
        return a;
    }

    template<typename T>
    [[nodiscard]] constexpr dimension <T> operator*(dimension <T> d, T scalar) noexcept {
        d *= scalar;
        return d;
    }

    template<typename T>
    [[nodiscard]] constexpr dimension <T> operator*(T scalar, dimension <T> d) noexcept {
        return d * scalar;
    }

    template<typename T>
    [[nodiscard]] constexpr dimension <T> operator/(dimension <T> d, T scalar) noexcept {
        d /= scalar;
        return d;
    }

    /**
     * @brief Width and height dimensions that must be non-negative
     */
    template<dimensional T = int>
    struct dimensions {
        dimension <T> width;
        dimension <T> height;

        constexpr dimensions() noexcept = default;

        constexpr dimensions(T w, T h) noexcept
            : width(w), height(h) {
        }

        constexpr dimensions(dimension <T> w, dimension <T> h) noexcept
            : width(w), height(h) {
        }

        /**
         * @brief Calculate area (guaranteed non-negative)
         */
        [[nodiscard]] constexpr auto area() const noexcept {
            if constexpr (std::is_integral_v <T>) {
                using larger_t = std::conditional_t <sizeof(T) <= 4, uint64_t, T>;
                return static_cast <larger_t>(width.value()) * static_cast <larger_t>(height.value());
            } else {
                return width.value() * height.value();
            }
        }

        /**
         * @brief Check if either dimension is zero
         */
        [[nodiscard]] constexpr bool is_empty() const noexcept {
            return width.is_zero() || height.is_zero();
        }

        /**
         * @brief Check if both dimensions are positive
         */
        [[nodiscard]] constexpr bool is_valid() const noexcept {
            return width.is_positive() && height.is_positive();
        }

        // Comparison operators
        auto operator<=>(const dimensions&) const = default;
    };

    // Type aliases for common use cases
    using dim = dimension <int>;
    using fdim = dimension <float>;
    using window_dimensions = dimensions <int>;
    using display_dimensions = dimensions <int>;

    /**
     * @brief Helper to create dimensions from potentially negative values
     *
     * This is useful when interfacing with SDL functions that return signed values
     * but semantically represent non-negative dimensions.
     */
    template<typename T>
    [[nodiscard]] constexpr dimensions <T> make_dimensions(T w, T h) noexcept {
        return dimensions <T>(w, h);
    }

    /**
     * @brief A coordinate that can be negative (for positions)
     *
     * This is a simple wrapper to provide semantic meaning and distinguish
     * coordinates from dimensions in APIs.
     */
    template<dimensional T = int>
    struct coordinate {
        T value;

        constexpr coordinate() noexcept = default;

        constexpr explicit coordinate(T val) noexcept
            : value(val) {
        }

        // Allow implicit conversion for ease of use
        constexpr operator T() const noexcept { return value; }

        // Arithmetic operations
        constexpr coordinate& operator+=(coordinate other) noexcept {
            value += other.value;
            return *this;
        }

        constexpr coordinate& operator-=(coordinate other) noexcept {
            value -= other.value;
            return *this;
        }

        // Comparison
        auto operator<=>(const coordinate&) const = default;
    };

    /**
     * @brief A position with x,y coordinates that can be negative
     */
    template<dimensional T = int>
    struct position {
        coordinate <T> x;
        coordinate <T> y;

        constexpr position() noexcept = default;

        constexpr position(T x_val, T y_val) noexcept
            : x(x_val), y(y_val) {
        }

        constexpr position(coordinate <T> x_coord, coordinate <T> y_coord) noexcept
            : x(x_coord), y(y_coord) {
        }

        // Comparison
        auto operator<=>(const position&) const = default;
    };

    // Type aliases
    using coord = coordinate <int>;
    using fcoord = coordinate <float>;
    using window_position = position <int>;

    /**
     * @brief Generic function to check if dimensions are valid for creation
     */
    template<dimensions_like D>
    [[nodiscard]] constexpr bool are_valid_dimensions(const D& dims) noexcept {
        return dims.is_valid();
    }

    /**
     * @brief Generic function to get area from any dimensions-like type
     */
    template<dimensions_like D>
    [[nodiscard]] constexpr auto get_area(const D& dims) noexcept {
        return dims.area();
    }

    /**
     * @brief Generic function to create SDL-compatible dimensions
     */
    template<dimensional T>
    [[nodiscard]] constexpr std::pair <int, int> to_sdl_dimensions(const dimensions <T>& dims) noexcept {
        return {static_cast <int>(dims.width), static_cast <int>(dims.height)};
    }

    /**
     * @brief Generic function to create SDL-compatible position
     */
    template<dimensional T>
    [[nodiscard]] constexpr std::pair <int, int> to_sdl_position(const position <T>& pos) noexcept {
        return {static_cast <int>(pos.x), static_cast <int>(pos.y)};
    }

    /**
     * @brief Concept-based dimension validation
     */
    template<non_negative_dimension D>
    [[nodiscard]] constexpr bool is_positive_dimension(const D& d) noexcept {
        return d.is_positive();
    }

    /**
     * @brief Create dimensions from any two non-negative dimension types
     */
    template<non_negative_dimension W, non_negative_dimension H>
    [[nodiscard]] constexpr auto make_dimensions_from(const W& width, const H& height) noexcept {
        using T = std::common_type_t <decltype(width.value()), decltype(height.value())>;
        return dimensions <T>(static_cast <T>(width.value()), static_cast <T>(height.value()));
    }

    /**
     * @brief Create position from any two coordinate types
     */
    template<coordinate_like X, coordinate_like Y>
    [[nodiscard]] constexpr auto make_position_from(const X& x, const Y& y) noexcept {
        using T = std::common_type_t <decltype(x.value), decltype(y.value)>;
        return position <T>(static_cast <T>(x.value), static_cast <T>(y.value));
    }
} // namespace sdlpp
