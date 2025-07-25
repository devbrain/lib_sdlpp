//
// Created by igor on 7/13/25.
//

#pragma once

/**
 * @file geometry.hh
 * @brief Template-based geometric primitives for SDL3
 * 
 * This header provides modern C++ geometric types (points, rectangles, sizes)
 * with both integer and floating-point versions, designed to work seamlessly
 * with SDL3's geometry types while providing rich functionality.
 */

#include <sdlpp/core/sdl.hh>
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <concepts>
#include <limits>

namespace sdlpp {
    /**
     * @brief Type traits for mapping C++ types to SDL types
     */
    template<typename T>
    struct sdl_types;

    template<>
    struct sdl_types <int> {
        using point_type = SDL_Point;
        using rect_type = SDL_Rect;
    };

    template<>
    struct sdl_types <float> {
        using point_type = SDL_FPoint;
        using rect_type = SDL_FRect;
    };

    /**
     * @brief Concept for numeric types suitable for geometry
     */
    template<typename T>
    concept arithmetic = std::is_arithmetic_v <T>;

    // Forward declarations for concepts
    template<arithmetic T>
    struct basic_point;
    template<arithmetic T>
    struct basic_size;
    template<arithmetic T>
    struct basic_rect;

    /**
     * @brief Concept for point-like types
     */
    template<typename T>
    concept point_like = requires(T t)
    {
        { t.x } -> std::convertible_to <typename T::value_type>;
        { t.y } -> std::convertible_to <typename T::value_type>;
        typename T::value_type;
        requires arithmetic <typename T::value_type>;
    };

    /**
     * @brief Concept for size-like types
     */
    template<typename T>
    concept size_like = requires(T t)
    {
        { t.width } -> std::convertible_to <typename T::value_type>;
        { t.height } -> std::convertible_to <typename T::value_type>;
        { t.area() } -> std::convertible_to <typename T::value_type>;
        { t.empty() } -> std::convertible_to <bool>;
        typename T::value_type;
        requires arithmetic <typename T::value_type>;
    };

    /**
     * @brief Concept for rect-like types
     */
    template<typename T>
    concept rect_like = requires(T t)
    {
        { t.x } -> std::convertible_to <typename T::value_type>;
        { t.y } -> std::convertible_to <typename T::value_type>;
        { t.w } -> std::convertible_to <typename T::value_type>;
        { t.h } -> std::convertible_to <typename T::value_type>;
        { t.empty() } -> std::convertible_to <bool>;
        { t.area() } -> std::convertible_to <typename T::value_type>;
        typename T::value_type;
        requires arithmetic <typename T::value_type>;
    };

    /**
     * @brief Generic 2D point with x,y coordinates
     * @tparam T Numeric type (typically int or float)
     */
    template<arithmetic T>
    struct basic_point {
        using value_type = T;

        T x = 0; ///< X coordinate
        T y = 0; ///< Y coordinate

        /**
         * @brief Default constructor - creates point at origin (0,0)
         */
        constexpr basic_point() = default;

        /**
         * @brief Construct point from coordinates
         * @param x_ X coordinate
         * @param y_ Y coordinate
         */
        constexpr basic_point(T x_, T y_)
            : x(x_), y(y_) {
        }

        /**
         * @brief Convert to SDL point type
         * @return SDL_Point for int, SDL_FPoint for float
         */
        [[nodiscard]] constexpr auto to_sdl() const {
            if constexpr (std::is_same_v <T, int>) {
                return SDL_Point{x, y};
            } else if constexpr (std::is_same_v <T, float>) {
                return SDL_FPoint{x, y};
            } else {
                // For other types, convert to the nearest SDL type
                if constexpr (std::is_integral_v <T>) {
                    return SDL_Point{static_cast <int>(x), static_cast <int>(y)};
                } else {
                    return SDL_FPoint{static_cast <float>(x), static_cast <float>(y)};
                }
            }
        }

        /**
         * @brief Create point from SDL type
         * @param p SDL point structure
         * @return basic_point instance
         */
        static constexpr basic_point from_sdl(const typename sdl_types <T>::point_type& p) {
            return {p.x, p.y};
        }

        /**
         * @brief Conversion constructor from different numeric type
         * @tparam U Source numeric type
         * @param other Source point
         */
        template<arithmetic U>
        constexpr explicit basic_point(const basic_point <U>& other)
            : x(static_cast <T>(other.x)), y(static_cast <T>(other.y)) {
        }

        // Arithmetic operators
        [[nodiscard]] constexpr basic_point operator+(const basic_point& p) const {
            return {x + p.x, y + p.y};
        }

        [[nodiscard]] constexpr basic_point operator-(const basic_point& p) const {
            return {x - p.x, y - p.y};
        }

        [[nodiscard]] constexpr basic_point operator*(T scalar) const {
            return {x * scalar, y * scalar};
        }

        [[nodiscard]] constexpr basic_point operator/(T scalar) const {
            return {x / scalar, y / scalar};
        }

        constexpr basic_point& operator+=(const basic_point& p) {
            x += p.x;
            y += p.y;
            return *this;
        }

        constexpr basic_point& operator-=(const basic_point& p) {
            x -= p.x;
            y -= p.y;
            return *this;
        }

        constexpr basic_point& operator*=(T scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        constexpr basic_point& operator/=(T scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        // Comparison operators
        [[nodiscard]] constexpr bool operator==(const basic_point&) const = default;

        // Unary operators
        [[nodiscard]] constexpr basic_point operator-() const {
            return {-x, -y};
        }

        /**
         * @brief Calculate squared distance to another point
         * @param p Target point
         * @return Squared distance (avoids sqrt for performance)
         */
        [[nodiscard]] constexpr auto distance_squared_to(const basic_point& p) const {
            T dx = x - p.x;
            T dy = y - p.y;
            if constexpr (std::is_integral_v <T>) {
                // Use larger type to avoid overflow for large values
                using larger_t = std::conditional_t <sizeof(T) <= 4, int64_t, T>;
                return static_cast <larger_t>(dx) * static_cast <larger_t>(dx) +
                       static_cast <larger_t>(dy) * static_cast <larger_t>(dy);
            } else {
                return dx * dx + dy * dy;
            }
        }

        /**
         * @brief Calculate distance to another point
         * @param p Target point
         * @return Euclidean distance
         */
        [[nodiscard]] T distance_to(const basic_point& p) const {
            if constexpr (std::is_same_v <T, float> || std::is_same_v <T, double>) {
                return std::sqrt(distance_squared_to(p));
            } else {
                return static_cast <T>(std::sqrt(static_cast <double>(distance_squared_to(p))));
            }
        }

        /**
         * @brief Get the magnitude (length) of the point vector
         * @return Distance from origin
         */
        [[nodiscard]] T magnitude() const {
            return distance_to({0, 0});
        }

        /**
         * @brief Get squared magnitude (avoids sqrt)
         * @return Squared distance from origin
         */
        [[nodiscard]] constexpr auto magnitude_squared() const {
            if constexpr (std::is_integral_v <T>) {
                // Use larger type to avoid overflow for large values
                using larger_t = std::conditional_t <sizeof(T) <= 4, int64_t, T>;
                return static_cast <larger_t>(x) * static_cast <larger_t>(x) +
                       static_cast <larger_t>(y) * static_cast <larger_t>(y);
            } else {
                return x * x + y * y;
            }
        }

        /**
         * @brief Dot product with another point/vector
         * @param p Other point
         * @return Dot product result
         */
        [[nodiscard]] constexpr T dot(const basic_point& p) const {
            return x * p.x + y * p.y;
        }

        /**
         * @brief Cross product magnitude (2D)
         * @param p Other point
         * @return Z-component of the cross product
         */
        [[nodiscard]] constexpr T cross(const basic_point& p) const {
            return x * p.y - y * p.x;
        }
    };

    /**
     * @brief Generic size with width and height
     * @tparam T Numeric type (typically int or float)
     */
    template<arithmetic T>
    struct basic_size {
        using value_type = T;

        T width = 0; ///< Width dimension
        T height = 0; ///< Height dimension

        /**
         * @brief Default constructor - creates zero size
         */
        constexpr basic_size() = default;

        /**
         * @brief Construct size from dimensions
         * @param w Width
         * @param h Height
         */
        constexpr basic_size(T w, T h)
            : width(w), height(h) {
        }

        /**
         * @brief Conversion constructor from different numeric type
         * @tparam U Source numeric type
         * @param other Source size
         */
        template<arithmetic U>
        constexpr explicit basic_size(const basic_size <U>& other)
            : width(static_cast <T>(other.width)), height(static_cast <T>(other.height)) {
        }

        /**
         * @brief Calculate area
         * @return width * height
         */
        [[nodiscard]] constexpr auto area() const {
            if constexpr (std::is_integral_v <T>) {
                // Use larger type to prevent overflow for integer types
                using larger_t = std::conditional_t <sizeof(T) <= 4, int64_t, T>;
                return static_cast <larger_t>(width) * static_cast <larger_t>(height);
            } else {
                return width * height;
            }
        }

        /**
         * @brief Check if size is empty (zero or negative)
         * @return true if width or height is <= 0
         */
        [[nodiscard]] constexpr bool empty() const {
            return width <= 0 || height <= 0;
        }

        /**
         * @brief Convert to point (for use with rect constructor)
         * @return Point with x=width, y=height
         */
        [[nodiscard]] constexpr basic_point <T> to_point() const {
            return {width, height};
        }

        // Comparison
        [[nodiscard]] constexpr bool operator==(const basic_size&) const = default;

        // Arithmetic operations
        [[nodiscard]] constexpr basic_size operator*(T scalar) const {
            return {width * scalar, height * scalar};
        }

        [[nodiscard]] constexpr basic_size operator/(T scalar) const {
            return {width / scalar, height / scalar};
        }
    };

    /**
     * @brief Generic rectangle with position and dimensions
     * @tparam T Numeric type (typically int or float)
     */
    template<arithmetic T>
    struct basic_rect {
        using value_type = T;

        T x = 0; ///< X position (left edge)
        T y = 0; ///< Y position (top edge)
        T w = 0; ///< Width
        T h = 0; ///< Height

        /**
         * @brief Default constructor - creates empty rect at origin
         */
        constexpr basic_rect() = default;

        /**
         * @brief Construct rect from position and dimensions
         * @param x_ Left edge X coordinate
         * @param y_ Top edge Y coordinate
         * @param w_ Width
         * @param h_ Height
         */
        constexpr basic_rect(T x_, T y_, T w_, T h_)
            : x(x_), y(y_), w(w_), h(h_) {
        }

        /**
         * @brief Construct rect from point and size
         * @param pos Top-left position
         * @param size Dimensions
         */
        constexpr basic_rect(const basic_point <T>& pos, const basic_size <T>& size)
            : x(pos.x), y(pos.y), w(size.width), h(size.height) {
        }

        /**
         * @brief Construct rect from two points (corners)
         * @param p1 First corner
         * @param p2 Opposite corner
         */
        static constexpr basic_rect from_corners(const basic_point <T>& p1, const basic_point <T>& p2) {
            T min_x = std::min(p1.x, p2.x);
            T min_y = std::min(p1.y, p2.y);
            T max_x = std::max(p1.x, p2.x);
            T max_y = std::max(p1.y, p2.y);
            return {min_x, min_y, max_x - min_x, max_y - min_y};
        }

        /**
         * @brief Convert to SDL rect type
         * @return SDL_Rect for int, SDL_FRect for float
         */
        [[nodiscard]] constexpr auto to_sdl() const {
            if constexpr (std::is_same_v <T, int>) {
                return SDL_Rect{x, y, w, h};
            } else if constexpr (std::is_same_v <T, float>) {
                return SDL_FRect{x, y, w, h};
            } else {
                // For other types, convert to the nearest SDL type
                if constexpr (std::is_integral_v <T>) {
                    return SDL_Rect{
                        static_cast <int>(x), static_cast <int>(y),
                        static_cast <int>(w), static_cast <int>(h)
                    };
                } else {
                    return SDL_FRect{
                        static_cast <float>(x), static_cast <float>(y),
                        static_cast <float>(w), static_cast <float>(h)
                    };
                }
            }
        }

        /**
         * @brief Create rect from SDL type
         * @param r SDL rect structure
         * @return basic_rect instance
         */
        static constexpr basic_rect from_sdl(const typename sdl_types <T>::rect_type& r) {
            return {r.x, r.y, r.w, r.h};
        }

        /**
         * @brief Conversion constructor from different numeric type
         * @tparam U Source numeric type
         * @param other Source rect
         */
        template<arithmetic U>
        constexpr explicit basic_rect(const basic_rect <U>& other)
            : x(static_cast <T>(other.x)), y(static_cast <T>(other.y)),
              w(static_cast <T>(other.w)), h(static_cast <T>(other.h)) {
        }

        // Properties
        [[nodiscard]] constexpr basic_point <T> position() const { return {x, y}; }
        [[nodiscard]] constexpr basic_size <T> size() const { return {w, h}; }
        [[nodiscard]] constexpr basic_point <T> center() const { return {x + w / 2, y + h / 2}; }

        [[nodiscard]] constexpr T left() const { return x; }
        [[nodiscard]] constexpr T right() const { return x + w; }
        [[nodiscard]] constexpr T top() const { return y; }
        [[nodiscard]] constexpr T bottom() const { return y + h; }

        [[nodiscard]] constexpr basic_point <T> top_left() const { return {x, y}; }
        [[nodiscard]] constexpr basic_point <T> top_right() const { return {x + w, y}; }
        [[nodiscard]] constexpr basic_point <T> bottom_left() const { return {x, y + h}; }
        [[nodiscard]] constexpr basic_point <T> bottom_right() const { return {x + w, y + h}; }

        [[nodiscard]] constexpr auto area() const {
            if constexpr (std::is_integral_v <T>) {
                // Use larger type to prevent overflow for integer types
                using larger_t = std::conditional_t <sizeof(T) <= 4, int64_t, T>;
                return static_cast <larger_t>(w) * static_cast <larger_t>(h);
            } else {
                return w * h;
            }
        }

        [[nodiscard]] constexpr bool empty() const { return w <= 0 || h <= 0; }

        /**
         * @brief Check if rect contains a point
         * @param p Point to test
         * @return true if point is inside rect (inclusive of edges)
         */
        [[nodiscard]] constexpr bool contains(const basic_point <T>& p) const {
            return p.x >= x && p.x < (x + w) &&
                   p.y >= y && p.y < (y + h);
        }

        /**
         * @brief Check if rect completely contains another rect
         * @param r Rect to test
         * @return true if r is completely inside this rect
         */
        [[nodiscard]] constexpr bool contains(const basic_rect& r) const {
            return r.x >= x && r.y >= y &&
                   r.right() <= right() && r.bottom() <= bottom();
        }

        /**
         * @brief Check if rect intersects with another rect
         * @param r Other rect
         * @return true if rects overlap
         */
        [[nodiscard]] constexpr bool intersects(const basic_rect& r) const {
            return !(r.x >= right() || r.right() <= x ||
                     r.y >= bottom() || r.bottom() <= y);
        }

        /**
         * @brief Calculate intersection with another rect
         * @param r Other rect
         * @return Intersection rect (empty if no intersection)
         */
        [[nodiscard]] constexpr basic_rect intersection(const basic_rect& r) const {
            if (!intersects(r)) return {};

            T ix = std::max(x, r.x);
            T iy = std::max(y, r.y);
            T iw = std::min(right(), r.right()) - ix;
            T ih = std::min(bottom(), r.bottom()) - iy;

            return {ix, iy, iw, ih};
        }

        /**
         * @brief Calculate union with another rect
         * @param r Other rect
         * @return Bounding rect containing both rects
         */
        [[nodiscard]] constexpr basic_rect union_with(const basic_rect& r) const {
            if (empty()) return r;
            if (r.empty()) return *this;

            T ux = std::min(x, r.x);
            T uy = std::min(y, r.y);
            T uw = std::max(right(), r.right()) - ux;
            T uh = std::max(bottom(), r.bottom()) - uy;

            return {ux, uy, uw, uh};
        }

        /**
         * @brief Create inflated/deflated rect
         * @param amount Amount to inflate (negative to deflate)
         * @return New rect expanded by amount on all sides
         */
        [[nodiscard]] constexpr basic_rect inflated(T amount) const {
            return {x - amount, y - amount, w + 2 * amount, h + 2 * amount};
        }

        /**
         * @brief Create inflated/deflated rect with different amounts
         * @param dx Horizontal inflation amount
         * @param dy Vertical inflation amount
         * @return New rect expanded by specified amounts
         */
        [[nodiscard]] constexpr basic_rect inflated(T dx, T dy) const {
            return {x - dx, y - dy, w + 2 * dx, h + 2 * dy};
        }

        /**
         * @brief Create moved rect
         * @param dx Horizontal offset
         * @param dy Vertical offset
         * @return New rect at offset position
         */
        [[nodiscard]] constexpr basic_rect moved(T dx, T dy) const {
            return {x + dx, y + dy, w, h};
        }

        /**
         * @brief Create moved rect
         * @param delta Offset vector
         * @return New rect at offset position
         */
        [[nodiscard]] constexpr basic_rect moved(const basic_point <T>& delta) const {
            return moved(delta.x, delta.y);
        }

        /**
         * @brief Create centered rect at position
         * @param center New center position
         * @return New rect centered at given point
         */
        [[nodiscard]] constexpr basic_rect centered_at(const basic_point <T>& center) const {
            return {center.x - w / 2, center.y - h / 2, w, h};
        }

        /**
         * @brief Clamp rect to be within bounds
         * @param bounds Bounding rect
         * @return New rect constrained to bounds
         */
        [[nodiscard]] constexpr basic_rect clamped_to(const basic_rect& bounds) const {
            T new_x = std::clamp(x, bounds.x, bounds.right() - w);
            T new_y = std::clamp(y, bounds.y, bounds.bottom() - h);
            return {new_x, new_y, w, h};
        }

        // Comparison
        [[nodiscard]] constexpr bool operator==(const basic_rect&) const = default;
    };

    // Type aliases for common use cases
    using point = basic_point <int>; ///< Integer 2D point
    using fpoint = basic_point <float>; ///< Floating-point 2D point
    using size = basic_size <int>; ///< Integer size
    using fsize = basic_size <float>; ///< Floating-point size
    using rect = basic_rect <int>; ///< Integer rectangle
    using frect = basic_rect <float>; ///< Floating-point rectangle

    // Utility functions

    /**
     * @brief Create rect from position and size
     * @tparam T Numeric type
     * @param pos Top-left position
     * @param sz Dimensions
     * @return Rectangle
     */
    template<arithmetic T>
    [[nodiscard]] constexpr basic_rect <T> make_rect(const basic_point <T>& pos, const basic_size <T>& sz) {
        return {pos, sz};
    }

    /**
     * @brief Linear interpolation between two points
     * @tparam T Numeric type
     * @param a Start point
     * @param b End point
     * @param t Interpolation factor (0-1)
     * @return Interpolated point
     */
    template<arithmetic T>
    [[nodiscard]] constexpr basic_point <T> lerp(const basic_point <T>& a, const basic_point <T>& b, T t) {
        return a + (b - a) * t;
    }

    /**
     * @brief Check if two rects have the same size
     * @tparam T Numeric type
     * @param a First rect
     * @param b Second rect
     * @return true if dimensions match
     */
    template<arithmetic T>
    [[nodiscard]] constexpr bool same_size(const basic_rect <T>& a, const basic_rect <T>& b) {
        return a.w == b.w && a.h == b.h;
    }

    /**
     * @brief Calculate distance between two points (generic version using concepts)
     * @tparam P1 First point type
     * @tparam P2 Second point type
     * @param p1 First point
     * @param p2 Second point
     * @return Distance between points
     */
    template<point_like P1, point_like P2>
        requires std::is_same_v <typename P1::value_type, typename P2::value_type>
    [[nodiscard]] auto distance_between(const P1& p1, const P2& p2) {
        using T = typename P1::value_type;
        T dx = p1.x - p2.x;
        T dy = p1.y - p2.y;
        if constexpr (std::is_floating_point_v <T>) {
            return std::sqrt(dx * dx + dy * dy);
        } else {
            return static_cast <T>(std::sqrt(static_cast <double>(dx * dx + dy * dy)));
        }
    }

    /**
     * @brief Check if a point is inside a rectangle (generic version)
     * @tparam P Point type
     * @tparam R Rectangle type
     * @param pt The point to test
     * @param r The rectangle to test against
     * @return true if point is inside rectangle
     */
    template<point_like P, rect_like R>
        requires std::is_same_v <typename P::value_type, typename R::value_type>
    [[nodiscard]] constexpr bool is_inside(const P& pt, const R& r) {
        return pt.x >= r.x && pt.x < (r.x + r.w) &&
               pt.y >= r.y && pt.y < (r.y + r.h);
    }

    /**
     * @brief Create a rectangle from center and size (generic version)
     * @tparam P Point type for center
     * @tparam S Size type
     * @param center Center point of the rectangle
     * @param sz Size of the rectangle
     * @return Rectangle centered at the given point
     */
    template<point_like P, size_like S>
        requires std::is_same_v <typename P::value_type, typename S::value_type>
    [[nodiscard]] constexpr auto rect_from_center(const P& center, const S& sz) {
        using T = typename P::value_type;
        return basic_rect <T>{
            center.x - sz.width / 2,
            center.y - sz.height / 2,
            sz.width,
            sz.height
        };
    }

    /**
     * @brief Scale a size by a factor (generic version)
     * @tparam S Size type
     * @tparam U Scalar type
     * @param sz The size to scale
     * @param factor The scaling factor
     * @return Scaled size
     */
    template<size_like S, arithmetic U>
    [[nodiscard]] constexpr auto scale_size(const S& sz, U factor) {
        using T = typename S::value_type;
        return basic_size <T>{
            static_cast <T>(sz.width * factor),
            static_cast <T>(sz.height * factor)
        };
    }

    /**
     * @brief Generic triangle with three vertices
     * @tparam T Numeric type (typically int or float)
     */
    template<arithmetic T>
    struct basic_triangle {
        using value_type = T;
        using point_type = basic_point <T>;

        point_type a; ///< First vertex
        point_type b; ///< Second vertex
        point_type c; ///< Third vertex

        /**
         * @brief Default constructor - creates degenerate triangle at origin
         */
        constexpr basic_triangle() = default;

        /**
         * @brief Construct triangle from three vertices
         * @param a_ First vertex
         * @param b_ Second vertex
         * @param c_ Third vertex
         */
        constexpr basic_triangle(const point_type& a_, const point_type& b_, const point_type& c_)
            : a(a_), b(b_), c(c_) {
        }

        /**
         * @brief Construct triangle from coordinates
         * @param ax First vertex X
         * @param ay First vertex Y
         * @param bx Second vertex X
         * @param by Second vertex Y
         * @param cx Third vertex X
         * @param cy Third vertex Y
         */
        constexpr basic_triangle(T ax, T ay, T bx, T by, T cx, T cy)
            : a(ax, ay), b(bx, by), c(cx, cy) {
        }

        /**
         * @brief Conversion constructor from different numeric type
         * @tparam U Source numeric type
         * @param other Source triangle
         */
        template<arithmetic U>
        constexpr explicit basic_triangle(const basic_triangle <U>& other)
            : a(other.a), b(other.b), c(other.c) {
        }

        /**
         * @brief Get vertex by index
         * @param index Vertex index (0-2)
         * @return Reference to vertex
         */
        [[nodiscard]] constexpr const point_type& operator[](size_t index) const {
            switch (index) {
                case 0: return a;
                case 1: return b;
                case 2: return c;
                default: return a; // Fallback for invalid index
            }
        }

        /**
         * @brief Get mutable vertex by index
         * @param index Vertex index (0-2)
         * @return Mutable reference to vertex
         */
        [[nodiscard]] constexpr point_type& operator[](size_t index) {
            switch (index) {
                case 0: return a;
                case 1: return b;
                case 2: return c;
                default: return a; // Fallback for invalid index
            }
        }

        /**
         * @brief Calculate centroid (center of mass)
         * @return Centroid point
         */
        [[nodiscard]] constexpr point_type centroid() const {
            return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
        }

        /**
         * @brief Calculate area using shoelace formula
         * @return Unsigned area (always positive)
         */
        [[nodiscard]] constexpr T area() const {
            if constexpr (std::is_floating_point_v <T>) {
                return std::abs(signed_area());
            } else {
                auto sa = signed_area();
                return sa < 0 ? -sa : sa;
            }
        }

        /**
         * @brief Calculate signed area (for winding order detection)
         * @return Signed area
         */
        [[nodiscard]] constexpr T signed_area() const {
            // Using the cross product formula: 0.5 * (AB x AC)
            if constexpr (std::is_floating_point_v <T>) {
                return static_cast <T>(0.5) * ((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y));
            } else {
                // For integers, calculate full cross product then divide by 2
                return ((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y)) / 2;
            }
        }

        /**
         * @brief Check if triangle is counter-clockwise
         * @return true if vertices are in CCW order
         */
        [[nodiscard]] constexpr bool is_ccw() const {
            return signed_area() > 0;
        }

        /**
         * @brief Calculate perimeter
         * @return Sum of all edge lengths
         */
        [[nodiscard]] T perimeter() const {
            return a.distance_to(b) + b.distance_to(c) + c.distance_to(a);
        }

        /**
         * @brief Get bounding rectangle
         * @return Smallest rectangle containing the triangle
         */
        [[nodiscard]] constexpr basic_rect <T> bounds() const {
            T min_x = std::min({a.x, b.x, c.x});
            T min_y = std::min({a.y, b.y, c.y});
            T max_x = std::max({a.x, b.x, c.x});
            T max_y = std::max({a.y, b.y, c.y});
            return {min_x, min_y, max_x - min_x, max_y - min_y};
        }

        /**
         * @brief Check if point is inside triangle using barycentric coordinates
         * @param p Point to test
         * @return true if point is inside triangle
         */
        [[nodiscard]] constexpr bool contains(const point_type& p) const {
            // First check if triangle is degenerate (zero area)
            if (signed_area() == 0) {
                return false;
            }

            if constexpr (std::is_floating_point_v <T>) {
                // Calculate barycentric coordinates for floating point
                T denominator = ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));

                T u = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) / denominator;
                T v = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) / denominator;
                T w = 1 - u - v;

                // Point is inside if all barycentric coordinates are non-negative
                return u >= 0 && v >= 0 && w >= 0;
            } else {
                // For integers, use sign of cross products to check if point is on same side of all edges
                auto sign = [](T val) -> int { return (val > 0) - (val < 0); };

                // Check if P is on the same side of edge AB as C
                T d1 = (p.x - a.x) * (b.y - a.y) - (b.x - a.x) * (p.y - a.y);
                T d2 = (c.x - a.x) * (b.y - a.y) - (b.x - a.x) * (c.y - a.y);
                if (sign(d1) != sign(d2) && d1 != 0 && d2 != 0) return false;

                // Check if P is on the same side of edge BC as A
                T d3 = (p.x - b.x) * (c.y - b.y) - (c.x - b.x) * (p.y - b.y);
                T d4 = (a.x - b.x) * (c.y - b.y) - (c.x - b.x) * (a.y - b.y);
                if (sign(d3) != sign(d4) && d3 != 0 && d4 != 0) return false;

                // Check if P is on the same side of edge CA as B
                T d5 = (p.x - c.x) * (a.y - c.y) - (a.x - c.x) * (p.y - c.y);
                T d6 = (b.x - c.x) * (a.y - c.y) - (a.x - c.x) * (b.y - c.y);
                if (sign(d5) != sign(d6) && d5 != 0 && d6 != 0) return false;

                return true;
            }
        }

        /**
         * @brief Create triangle translated by offset
         * @param offset Translation vector
         * @return Translated triangle
         */
        [[nodiscard]] constexpr basic_triangle translated(const point_type& offset) const {
            return {a + offset, b + offset, c + offset};
        }

        /**
         * @brief Create triangle scaled from origin
         * @param factor Scale factor
         * @return Scaled triangle
         */
        [[nodiscard]] constexpr basic_triangle scaled(T factor) const {
            return {a * factor, b * factor, c * factor};
        }

        /**
         * @brief Create triangle scaled from a point
         * @param center Center of scaling
         * @param factor Scale factor
         * @return Scaled triangle
         */
        [[nodiscard]] constexpr basic_triangle scaled_from(const point_type& center, T factor) const {
            return {
                center + (a - center) * factor,
                center + (b - center) * factor,
                center + (c - center) * factor
            };
        }

        /**
         * @brief Create triangle rotated around origin
         * @param angle Rotation angle in radians
         * @return Rotated triangle
         */
        [[nodiscard]] basic_triangle rotated(T angle) const {
            T cos_a = std::cos(angle);
            T sin_a = std::sin(angle);

            auto rotate_point = [cos_a, sin_a](const point_type& p) {
                return point_type{
                    p.x * cos_a - p.y * sin_a,
                    p.x * sin_a + p.y * cos_a
                };
            };

            return {rotate_point(a), rotate_point(b), rotate_point(c)};
        }

        /**
         * @brief Create triangle rotated around a point
         * @param center Center of rotation
         * @param angle Rotation angle in radians
         * @return Rotated triangle
         */
        [[nodiscard]] basic_triangle rotated_around(const point_type& center, T angle) const {
            T cos_a = std::cos(angle);
            T sin_a = std::sin(angle);

            auto rotate_point = [&](const point_type& p) {
                T dx = p.x - center.x;
                T dy = p.y - center.y;
                return point_type{
                    center.x + dx * cos_a - dy * sin_a,
                    center.y + dx * sin_a + dy * cos_a
                };
            };

            return {rotate_point(a), rotate_point(b), rotate_point(c)};
        }

        // Comparison
        [[nodiscard]] constexpr bool operator==(const basic_triangle&) const = default;
    };

    // Type aliases for common use cases
    using triangle = basic_triangle <int>; ///< Integer triangle
    using ftriangle = basic_triangle <float>; ///< Floating-point triangle

    /**
     * @brief Concept for triangle-like types
     */
    template<typename T>
    concept triangle_like = requires(T t)
    {
        requires point_like <typename T::point_type>;
        { t.a } -> std::convertible_to <typename T::point_type>;
        { t.b } -> std::convertible_to <typename T::point_type>;
        { t.c } -> std::convertible_to <typename T::point_type>;
        { t.area() } -> std::convertible_to <typename T::value_type>;
        { t.contains(typename T::point_type{}) } -> std::convertible_to <bool>;
        typename T::value_type;
        typename T::point_type;
        requires arithmetic <typename T::value_type>;
    };

    /**
     * @brief Create equilateral triangle centered at origin
     * @tparam T Numeric type
     * @param side_length Length of triangle side
     * @return Equilateral triangle
     */
    template<arithmetic T>
    [[nodiscard]] basic_triangle <T> make_equilateral_triangle(T side_length) {
        T height = side_length * static_cast <T>(std::sqrt(3)) / 2;
        T half_base = side_length / 2;
        return {
            {0, -height * 2 / 3}, // Top vertex
            {-half_base, height / 3}, // Bottom left
            {half_base, height / 3} // Bottom right
        };
    }

    /**
     * @brief Create right triangle
     * @tparam T Numeric type
     * @param base Base length
     * @param height Height
     * @return Right triangle with right angle at origin
     */
    template<arithmetic T>
    [[nodiscard]] constexpr basic_triangle <T> make_right_triangle(T base, T height) {
        return {{0, 0}, {base, 0}, {0, height}};
    }

    /**
     * @brief Check if three points are collinear
     * @tparam P Point type
     * @param p1 First point
     * @param p2 Second point
     * @param p3 Third point
     * @return true if points lie on the same line
     */
    template<point_like P>
    [[nodiscard]] constexpr bool are_collinear(const P& p1, const P& p2, const P& p3) {
        // Use cross product to check collinearity
        using T = typename P::value_type;
        T cross = (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y);

        if constexpr (std::is_floating_point_v <T>) {
            // For floating point, use relative epsilon based on the magnitude of points
            T max_coord = std::max({
                std::abs(p1.x), std::abs(p1.y),
                std::abs(p2.x), std::abs(p2.y),
                std::abs(p3.x), std::abs(p3.y)
            });
            T epsilon = std::numeric_limits <T>::epsilon() * max_coord * 100;
            return std::abs(cross) < epsilon;
        } else {
            return cross == 0;
        }
    }
} // namespace sdlpp
