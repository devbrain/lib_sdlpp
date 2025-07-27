//
// SDL++ Geometry Concepts
// Copyright (C) 2024 SDL++
//
// This header defines geometry concepts with ZERO dependencies
// It can be used to make SDL++ work with any geometry library
//

#ifndef SDLPP_UTILITY_GEOMETRY_CONCEPTS_HH
#define SDLPP_UTILITY_GEOMETRY_CONCEPTS_HH

#include <concepts>
#include <type_traits>
#include <cstddef>

namespace sdlpp {

/**
 * @brief Concept for point-like types
 * 
 * A type satisfies point_like if it has:
 * - A nested type alias value_type
 * - Members x and y that are convertible to value_type
 * 
 * @tparam T The type to check
 */
template<typename T>
concept point_like = requires(T t) {
    typename T::value_type;
    { t.x } -> std::convertible_to<typename T::value_type>;
    { t.y } -> std::convertible_to<typename T::value_type>;
};

/**
 * @brief Concept for size-like types
 * 
 * A type satisfies size_like if it has:
 * - A nested type alias value_type
 * - Members width and height that are convertible to value_type
 * 
 * @tparam T The type to check
 */
template<typename T>
concept size_like = requires(T t) {
    typename T::value_type;
    { t.width } -> std::convertible_to<typename T::value_type>;
    { t.height } -> std::convertible_to<typename T::value_type>;
};

/**
 * @brief Concept for rectangle-like types (SDL style)
 * 
 * A type satisfies rect_like if it has:
 * - A nested type alias value_type
 * - Members x, y, w, h that are convertible to value_type
 * 
 * @tparam T The type to check
 */
template<typename T>
concept rect_like = requires(T t) {
    typename T::value_type;
    { t.x } -> std::convertible_to<typename T::value_type>;
    { t.y } -> std::convertible_to<typename T::value_type>;
    { t.w } -> std::convertible_to<typename T::value_type>;
    { t.h } -> std::convertible_to<typename T::value_type>;
};

/**
 * @brief Alternative rectangle concept for libraries using different naming
 * 
 * Some libraries use left/top/width/height instead of x/y/w/h
 */
template<typename T>
concept rect_like_alt = requires(T t) {
    typename T::value_type;
    { t.left } -> std::convertible_to<typename T::value_type>;
    { t.top } -> std::convertible_to<typename T::value_type>;
    { t.width } -> std::convertible_to<typename T::value_type>;
    { t.height } -> std::convertible_to<typename T::value_type>;
};

/**
 * @brief Combined rectangle concept accepting both styles
 */
template<typename T>
concept rectangle_like = rect_like<T> || rect_like_alt<T>;

/**
 * @brief Concept for line-like types
 * 
 * A type satisfies line_like if it has:
 * - A nested type alias value_type
 * - Members x1, y1, x2, y2 that are convertible to value_type
 * 
 * @tparam T The type to check
 */
template<typename T>
concept line_like = requires(T t) {
    typename T::value_type;
    { t.x1 } -> std::convertible_to<typename T::value_type>;
    { t.y1 } -> std::convertible_to<typename T::value_type>;
    { t.x2 } -> std::convertible_to<typename T::value_type>;
    { t.y2 } -> std::convertible_to<typename T::value_type>;
};

/**
 * @brief Concept for circle-like types
 * 
 * A type satisfies circle_like if it has:
 * - A nested type alias value_type
 * - Members x, y, radius that are convertible to value_type
 * 
 * @tparam T The type to check
 */
template<typename T>
concept circle_like = requires(T t) {
    typename T::value_type;
    { t.x } -> std::convertible_to<typename T::value_type>;
    { t.y } -> std::convertible_to<typename T::value_type>;
    { t.radius } -> std::convertible_to<typename T::value_type>;
};

/**
 * @brief Concept for triangle-like types
 */
template<typename T>
concept triangle_like = requires(T t) {
    requires point_like<decltype(t.a)>;
    requires point_like<decltype(t.b)>;
    requires point_like<decltype(t.c)>;
};

/**
 * @brief Concept for polygon-like types
 * 
 * A type satisfies polygon_like if it:
 * - Has a value_type typedef
 * - Has a size() method returning a size_t convertible value
 * - Can be indexed with operator[] returning a point_like type
 * 
 * @tparam T The type to check
 */
template<typename T>
concept polygon_like = requires(T t) {
    typename T::value_type;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t[0] } -> point_like;
};

/**
 * @brief Concept for arithmetic point types
 * 
 * A point type with arithmetic value_type
 */
template<typename T>
concept arithmetic_point_like = point_like<T> && 
    std::is_arithmetic_v<typename T::value_type>;

/**
 * @brief Concept for arithmetic size types
 * 
 * A size type with arithmetic value_type
 */
template<typename T>
concept arithmetic_size_like = size_like<T> && 
    std::is_arithmetic_v<typename T::value_type>;

/**
 * @brief Concept for arithmetic rect types
 * 
 * A rect type with arithmetic value_type
 */
template<typename T>
concept arithmetic_rect_like = rect_like<T> && 
    std::is_arithmetic_v<typename T::value_type>;

/**
 * @brief Helper to extract value type from geometric types
 */
template<typename T>
struct geometry_value_type {
    using type = typename T::value_type;
};

template<typename T>
using geometry_value_type_t = typename geometry_value_type<T>::type;

// Utility functions that work with any geometry type

/**
 * @brief Get x coordinate from a point-like type
 */
template<point_like P>
constexpr auto get_x(const P& p) noexcept { 
    return p.x; 
}

/**
 * @brief Get y coordinate from a point-like type
 */
template<point_like P>
constexpr auto get_y(const P& p) noexcept { 
    return p.y; 
}

/**
 * @brief Get width from a size-like type
 */
template<size_like S>
constexpr auto get_width(const S& s) noexcept { 
    return s.width; 
}

/**
 * @brief Get height from a size-like type
 */
template<size_like S>
constexpr auto get_height(const S& s) noexcept { 
    return s.height; 
}

/**
 * @brief Get width from a rect-like type
 */
template<rect_like R>
constexpr auto get_width(const R& r) noexcept {
    return r.w;
}

/**
 * @brief Get height from a rect-like type
 */
template<rect_like R>
constexpr auto get_height(const R& r) noexcept {
    return r.h;
}

/**
 * @brief Get width from a rect-like-alt type
 */
template<rect_like_alt R>
requires (!size_like<R>)  // Disambiguate from size_like
constexpr auto get_width(const R& r) noexcept {
    return r.width;
}

/**
 * @brief Get height from a rect-like-alt type
 */
template<rect_like_alt R>
requires (!size_like<R>)  // Disambiguate from size_like
constexpr auto get_height(const R& r) noexcept {
    return r.height;
}

/**
 * @brief Calculate area of a size-like type
 */
template<size_like S>
constexpr auto get_area(const S& s) noexcept {
    return s.width * s.height;
}

/**
 * @brief Calculate area of a rect-like type
 */
template<rect_like R>
constexpr auto get_area(const R& r) noexcept {
    return r.w * r.h;
}

/**
 * @brief Calculate area of a rect-like-alt type
 */
template<rect_like_alt R>
requires (!size_like<R>)  // Disambiguate from size_like
constexpr auto get_area(const R& r) noexcept {
    return r.width * r.height;
}

/**
 * @brief Check if a size is empty (zero or negative area)
 */
template<size_like S>
constexpr bool is_empty(const S& s) noexcept {
    return s.width <= 0 || s.height <= 0;
}

/**
 * @brief Check if a rect is empty (zero or negative area)
 */
template<rect_like R>
constexpr bool is_empty(const R& r) noexcept {
    return r.w <= 0 || r.h <= 0;
}

/**
 * @brief Check if a rect-alt is empty (zero or negative area)
 */
template<rect_like_alt R>
constexpr bool is_empty(const R& r) noexcept {
    return r.width <= 0 || r.height <= 0;
}

/**
 * @brief Check if a point is inside a rectangle
 */
template<point_like P, rect_like R>
constexpr bool contains(const R& r, const P& p) noexcept {
    return p.x >= r.x && p.x < r.x + r.w &&
           p.y >= r.y && p.y < r.y + r.h;
}

/**
 * @brief Check if a point is inside a rectangle (alt format)
 */
template<point_like P, rect_like_alt R>
constexpr bool contains(const R& r, const P& p) noexcept {
    return p.x >= r.left && p.x < r.left + r.width &&
           p.y >= r.top && p.y < r.top + r.height;
}

/**
 * @brief Check if two rectangles intersect
 */
template<rect_like R1, rect_like R2>
constexpr bool intersects(const R1& a, const R2& b) noexcept {
    return a.x < b.x + b.w && a.x + a.w > b.x && 
           a.y < b.y + b.h && a.y + a.h > b.y;
}

/**
 * @brief Check if two rectangles intersect (alt format)
 */
template<rect_like_alt R1, rect_like_alt R2>
constexpr bool intersects(const R1& a, const R2& b) noexcept {
    return a.left < b.left + b.width && a.left + a.width > b.left && 
           a.top < b.top + b.height && a.top + a.height > b.top;
}

/**
 * @brief Check if two rectangles intersect (mixed formats)
 */
template<rect_like R1, rect_like_alt R2>
constexpr bool intersects(const R1& a, const R2& b) noexcept {
    return a.x < b.left + b.width && a.x + a.w > b.left && 
           a.y < b.top + b.height && a.y + a.h > b.top;
}

/**
 * @brief Check if two rectangles intersect (mixed formats)
 */
template<rect_like_alt R1, rect_like R2>
constexpr bool intersects(const R1& a, const R2& b) noexcept {
    return a.left < b.x + b.w && a.left + a.width > b.x && 
           a.top < b.y + b.h && a.top + a.height > b.y;
}

/**
 * @brief Extract position from a rect as a point-like type
 * 
 * @tparam P Point type to create (must be constructible from two values)
 * @param r Rectangle to extract position from
 * @return Point of type P
 */
template<typename P, rect_like R>
requires std::constructible_from<P, typename R::value_type, typename R::value_type>
constexpr P get_position(const R& r) {
    return P{r.x, r.y};
}

/**
 * @brief Extract position from a rect-alt as a point-like type
 */
template<typename P, rect_like_alt R>
requires std::constructible_from<P, typename R::value_type, typename R::value_type>
constexpr P get_position(const R& r) {
    return P{r.left, r.top};
}

/**
 * @brief Extract size from a rect as a size-like type
 * 
 * @tparam S Size type to create (must be constructible from two values)
 * @param r Rectangle to extract size from
 * @return Size of type S
 */
template<typename S, rect_like R>
requires std::constructible_from<S, typename R::value_type, typename R::value_type>
constexpr S get_size(const R& r) {
    return S{r.w, r.h};
}

/**
 * @brief Extract size from a rect-alt as a size-like type
 */
template<typename S, rect_like_alt R>
requires std::constructible_from<S, typename R::value_type, typename R::value_type>
constexpr S get_size(const R& r) {
    return S{r.width, r.height};
}

/**
 * @brief Create a rect from point and size
 * 
 * @tparam R Rect type to create
 * @param p Position (point-like)
 * @param s Size (size-like)
 * @return Rectangle of type R
 */
template<typename R, point_like P, size_like S>
requires std::constructible_from<R, 
    typename P::value_type, typename P::value_type,
    typename S::value_type, typename S::value_type>
constexpr R make_rect(const P& p, const S& s) {
    return R{p.x, p.y, s.width, s.height};
}

} // namespace sdlpp

#endif // SDLPP_UTILITY_GEOMETRY_CONCEPTS_HH