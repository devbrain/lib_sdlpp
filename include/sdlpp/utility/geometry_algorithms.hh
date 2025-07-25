//
// SDL++ Geometry Algorithms
// Copyright (C) 2024 SDL++
//
// Generic algorithms that work with any geometry types satisfying the concepts
//

#ifndef SDLPP_UTILITY_GEOMETRY_ALGORITHMS_HH
#define SDLPP_UTILITY_GEOMETRY_ALGORITHMS_HH

#include <sdlpp/utility/geometry_concepts.hh>
#include <cmath>
#include <algorithm>

namespace sdlpp {

// Distance calculations

/**
 * @brief Calculate distance between two points
 * @tparam P1 First point type (must satisfy point_like)
 * @tparam P2 Second point type (must satisfy point_like)
 * @param p1 First point
 * @param p2 Second point
 * @return Distance between the points
 */
template<point_like P1, point_like P2>
auto distance(const P1& p1, const P2& p2) {
    auto dx = p2.x - p1.x;
    auto dy = p2.y - p1.y;
    return std::sqrt(dx * dx + dy * dy);
}

/**
 * @brief Calculate squared distance between two points (avoids sqrt)
 * @tparam P1 First point type (must satisfy point_like)
 * @tparam P2 Second point type (must satisfy point_like)
 * @param p1 First point
 * @param p2 Second point
 * @return Squared distance between the points
 */
template<point_like P1, point_like P2>
constexpr auto distance_squared(const P1& p1, const P2& p2) {
    auto dx = p2.x - p1.x;
    auto dy = p2.y - p1.y;
    return dx * dx + dy * dy;
}

/**
 * @brief Calculate Manhattan distance between two points
 * @tparam P1 First point type (must satisfy point_like)
 * @tparam P2 Second point type (must satisfy point_like)
 * @param p1 First point
 * @param p2 Second point
 * @return Manhattan distance (|dx| + |dy|)
 */
template<point_like P1, point_like P2>
constexpr auto manhattan_distance(const P1& p1, const P2& p2) {
    auto dx = p2.x > p1.x ? p2.x - p1.x : p1.x - p2.x;
    auto dy = p2.y > p1.y ? p2.y - p1.y : p1.y - p2.y;
    return dx + dy;
}

// Bounding box calculations

/**
 * @brief Calculate bounding box for a collection of points
 * @tparam Container Container type holding point_like elements
 * @tparam R Rectangle type to return (must be constructible from x,y,w,h)
 * @param points Collection of points
 * @return Bounding rectangle containing all points
 */
template<typename R, typename Container>
requires requires(Container c) {
    { std::begin(c) } -> point_like;
    { std::end(c) };
}
R bounding_box(const Container& points) {
    if (std::begin(points) == std::end(points)) {
        return R{0, 0, 0, 0};
    }
    
    auto it = std::begin(points);
    auto min_x = it->x;
    auto max_x = it->x;
    auto min_y = it->y;
    auto max_y = it->y;
    
    for (++it; it != std::end(points); ++it) {
        min_x = std::min(min_x, it->x);
        max_x = std::max(max_x, it->x);
        min_y = std::min(min_y, it->y);
        max_y = std::max(max_y, it->y);
    }
    
    return R{min_x, min_y, max_x - min_x, max_y - min_y};
}

// Interpolation

/**
 * @brief Linear interpolation between two points
 * @tparam P Point type (must satisfy point_like)
 * @param p1 Start point
 * @param p2 End point
 * @param t Interpolation factor (0.0 = p1, 1.0 = p2)
 * @return Interpolated point
 */
template<point_like P>
P lerp(const P& p1, const P& p2, double t) {
    using T = typename P::value_type;
    return P{
        static_cast<T>(static_cast<double>(p1.x) + static_cast<double>(p2.x - p1.x) * t),
        static_cast<T>(static_cast<double>(p1.y) + static_cast<double>(p2.y - p1.y) * t)
    };
}

/**
 * @brief Cubic bezier interpolation
 * @tparam P Point type (must satisfy point_like)
 * @param p0 Start point
 * @param p1 First control point
 * @param p2 Second control point
 * @param p3 End point
 * @param t Interpolation factor (0.0 to 1.0)
 * @return Point on the bezier curve
 */
template<point_like P>
P bezier_cubic(const P& p0, const P& p1, const P& p2, const P& p3, double t) {
    double u = 1.0 - t;
    double u2 = u * u;
    double u3 = u2 * u;
    double t2 = t * t;
    double t3 = t2 * t;
    
    using T = typename P::value_type;
    return P{
        static_cast<T>(u3 * static_cast<double>(p0.x) + 3 * u2 * t * static_cast<double>(p1.x) + 3 * u * t2 * static_cast<double>(p2.x) + t3 * static_cast<double>(p3.x)),
        static_cast<T>(u3 * static_cast<double>(p0.y) + 3 * u2 * t * static_cast<double>(p1.y) + 3 * u * t2 * static_cast<double>(p2.y) + t3 * static_cast<double>(p3.y))
    };
}

// Geometric tests

/**
 * @brief Check if three points are collinear
 * @tparam P Point type (must satisfy point_like)
 * @param p1 First point
 * @param p2 Second point
 * @param p3 Third point
 * @param epsilon Tolerance for floating point comparison
 * @return true if points are on the same line
 */
template<point_like P>
bool are_collinear(const P& p1, const P& p2, const P& p3, 
                   typename P::value_type epsilon = 0) {
    // Cross product approach: (p2-p1) × (p3-p1) = 0
    auto cross = (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y);
    
    if constexpr (std::is_floating_point_v<typename P::value_type>) {
        return std::abs(cross) <= epsilon;
    } else {
        return cross == 0;
    }
}

/**
 * @brief Calculate the angle between three points
 * @tparam P Point type (must satisfy point_like)
 * @param p1 First point
 * @param vertex Vertex point (where angle is measured)
 * @param p2 Second point
 * @return Angle in radians
 */
template<point_like P>
double angle_between(const P& p1, const P& vertex, const P& p2) {
    double dx1 = p1.x - vertex.x;
    double dy1 = p1.y - vertex.y;
    double dx2 = p2.x - vertex.x;
    double dy2 = p2.y - vertex.y;
    
    return std::atan2(dy2, dx2) - std::atan2(dy1, dx1);
}

// Rectangle operations

/**
 * @brief Calculate the union of multiple rectangles
 * @tparam Container Container of rectangle_like elements
 * @tparam R Rectangle type to return
 * @param rects Collection of rectangles
 * @return Bounding rectangle containing all input rectangles
 */
template<typename R, typename Container>
requires requires(Container c) {
    { *std::begin(c) } -> rectangle_like;
}
R union_all(const Container& rects) {
    if (std::begin(rects) == std::end(rects)) {
        return R{0, 0, 0, 0};
    }
    
    auto it = std::begin(rects);
    
    // Get bounds from first rectangle
    typename R::value_type min_x, min_y, max_x, max_y;
    
    if constexpr (rect_like<decltype(*it)>) {
        min_x = it->x;
        min_y = it->y;
        max_x = it->x + it->w;
        max_y = it->y + it->h;
    } else {
        min_x = it->left;
        min_y = it->top;
        max_x = it->left + it->width;
        max_y = it->top + it->height;
    }
    
    // Process remaining rectangles
    for (++it; it != std::end(rects); ++it) {
        if constexpr (rect_like<decltype(*it)>) {
            min_x = std::min(min_x, it->x);
            min_y = std::min(min_y, it->y);
            max_x = std::max(max_x, it->x + it->w);
            max_y = std::max(max_y, it->y + it->h);
        } else {
            min_x = std::min(min_x, it->left);
            min_y = std::min(min_y, it->top);
            max_x = std::max(max_x, it->left + it->width);
            max_y = std::max(max_y, it->top + it->height);
        }
    }
    
    return R{min_x, min_y, max_x - min_x, max_y - min_y};
}

/**
 * @brief Scale a rectangle from its center
 * @tparam R Rectangle type (must satisfy rectangle_like)
 * @param rect Rectangle to scale
 * @param scale_x Horizontal scale factor
 * @param scale_y Vertical scale factor
 * @return Scaled rectangle
 */
template<rectangle_like R>
R scale_from_center(const R& rect, double scale_x, double scale_y) {
    using T = typename R::value_type;
    
    T cx, cy, w, h;
    if constexpr (rect_like<R>) {
        cx = rect.x + rect.w / 2;
        cy = rect.y + rect.h / 2;
        w = static_cast<T>(rect.w * scale_x);
        h = static_cast<T>(rect.h * scale_y);
    } else {
        cx = rect.left + rect.width / 2;
        cy = rect.top + rect.height / 2;
        w = static_cast<T>(rect.width * scale_x);
        h = static_cast<T>(rect.height * scale_y);
    }
    
    if constexpr (rect_like<R>) {
        return R{cx - w / 2, cy - h / 2, w, h};
    } else {
        // For rect_like_alt, we need to construct differently
        // This assumes the type has a constructor that takes left, top, width, height
        return R{cx - w / 2, cy - h / 2, w, h};
    }
}

// Aspect ratio utilities

/**
 * @brief Calculate aspect ratio of a size
 * @tparam S Size type (must satisfy size_like)
 * @param s Size to calculate ratio for
 * @return Aspect ratio (width/height)
 */
template<size_like S>
double aspect_ratio(const S& s) {
    return s.height > 0 ? static_cast<double>(s.width) / s.height : 0.0;
}

/**
 * @brief Fit a size within bounds while preserving aspect ratio
 * @tparam S Size type (must satisfy size_like)
 * @param size Original size
 * @param max_size Maximum bounds
 * @return Scaled size that fits within bounds
 */
template<size_like S>
S fit_within(const S& size, const S& max_size) {
    if (is_empty(size) || is_empty(max_size)) {
        return S{0, 0};
    }
    
    double scale_x = static_cast<double>(max_size.width) / size.width;
    double scale_y = static_cast<double>(max_size.height) / size.height;
    double scale = std::min(scale_x, scale_y);
    
    using T = typename S::value_type;
    return S{
        static_cast<T>(size.width * scale),
        static_cast<T>(size.height * scale)
    };
}

/**
 * @brief Fill bounds while preserving aspect ratio (may crop)
 * @tparam S Size type (must satisfy size_like)
 * @param size Original size
 * @param min_size Minimum bounds to fill
 * @return Scaled size that fills bounds
 */
template<size_like S>
S fill_bounds(const S& size, const S& min_size) {
    if (is_empty(size) || is_empty(min_size)) {
        return S{0, 0};
    }
    
    double scale_x = static_cast<double>(min_size.width) / size.width;
    double scale_y = static_cast<double>(min_size.height) / size.height;
    double scale = std::max(scale_x, scale_y);
    
    using T = typename S::value_type;
    return S{
        static_cast<T>(size.width * scale),
        static_cast<T>(size.height * scale)
    };
}

// Rotation utilities

/**
 * @brief Rotate a point around origin
 * @tparam P Point type (must satisfy point_like)
 * @param p Point to rotate
 * @param angle Rotation angle in radians
 * @return Rotated point
 */
template<point_like P>
P rotate(const P& p, double angle) {
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    
    using T = typename P::value_type;
    return P{
        static_cast<T>(p.x * cos_a - p.y * sin_a),
        static_cast<T>(p.x * sin_a + p.y * cos_a)
    };
}

/**
 * @brief Rotate a point around another point
 * @tparam P Point type (must satisfy point_like)
 * @param p Point to rotate
 * @param center Center of rotation
 * @param angle Rotation angle in radians
 * @return Rotated point
 */
template<point_like P>
P rotate_around(const P& p, const P& center, double angle) {
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    
    double dx = p.x - center.x;
    double dy = p.y - center.y;
    
    using T = typename P::value_type;
    return P{
        static_cast<T>(center.x + dx * cos_a - dy * sin_a),
        static_cast<T>(center.y + dx * sin_a + dy * cos_a)
    };
}

} // namespace sdlpp

#endif // SDLPP_UTILITY_GEOMETRY_ALGORITHMS_HH