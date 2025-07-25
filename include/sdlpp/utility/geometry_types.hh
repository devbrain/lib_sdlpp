//
// SDL++ Built-in Geometry Types
// Copyright (C) 2024 SDL++
//
// This header provides concrete geometry type implementations
//

#ifndef SDLPP_UTILITY_GEOMETRY_TYPES_HH
#define SDLPP_UTILITY_GEOMETRY_TYPES_HH

#include <sdlpp/utility/geometry_concepts.hh>
#include <algorithm>
#include <cmath>
#include <ostream>
#include <functional>

namespace sdlpp {

/**
 * @brief Built-in point type
 * @tparam T The coordinate type (int, float, double, etc.)
 */
template<typename T>
struct point {
    using value_type = T;
    T x{}, y{};
    
    constexpr point() = default;
    constexpr point(T x_, T y_) : x(x_), y(y_) {}
    
    // Conversion constructor
    template<typename U>
    explicit constexpr point(const point<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}
    
    // Arithmetic operators
    constexpr point operator+(const point& other) const {
        return {x + other.x, y + other.y};
    }
    
    constexpr point operator-(const point& other) const {
        return {x - other.x, y - other.y};
    }
    
    constexpr point operator*(T scalar) const {
        return {x * scalar, y * scalar};
    }
    
    constexpr point operator/(T scalar) const {
        return {x / scalar, y / scalar};
    }
    
    constexpr point& operator+=(const point& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    constexpr point& operator-=(const point& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    
    constexpr point& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    
    constexpr point& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }
    
    constexpr point operator-() const {
        return {-x, -y};
    }
    
    // Comparison
    constexpr bool operator==(const point&) const = default;
    constexpr auto operator<=>(const point&) const = default;
    
    // Utility methods
    constexpr T length_squared() const {
        return x * x + y * y;
    }
    
    T length() const {
        return static_cast<T>(std::sqrt(static_cast<double>(length_squared())));
    }
    
    constexpr T dot(const point& other) const {
        return x * other.x + y * other.y;
    }
    
    constexpr T cross(const point& other) const {
        return x * other.y - y * other.x;
    }
    
    point normalized() const {
        T len = length();
        return len > 0 ? point{x / len, y / len} : point{};
    }
};

// Stream operator
template<typename T>
std::ostream& operator<<(std::ostream& os, const point<T>& p) {
    return os << "(" << p.x << ", " << p.y << ")";
}

// Scalar multiplication (scalar first)
template<typename T>
constexpr point<T> operator*(T scalar, const point<T>& p) {
    return p * scalar;
}

/**
 * @brief Built-in size type
 * @tparam T The dimension type (int, float, double, etc.)
 */
template<typename T>
struct size {
    using value_type = T;
    T width{}, height{};
    
    constexpr size() = default;
    constexpr size(T w, T h) : width(w), height(h) {}
    
    // Conversion constructor
    template<typename U>
    explicit constexpr size(const size<U>& other)
        : width(static_cast<T>(other.width)), height(static_cast<T>(other.height)) {}
    
    constexpr T area() const { return width * height; }
    constexpr bool empty() const { return width <= 0 || height <= 0; }
    
    // Arithmetic operators
    constexpr size operator+(const size& other) const {
        return {width + other.width, height + other.height};
    }
    
    constexpr size operator-(const size& other) const {
        return {width - other.width, height - other.height};
    }
    
    constexpr size operator*(T scalar) const {
        return {width * scalar, height * scalar};
    }
    
    constexpr size operator/(T scalar) const {
        return {width / scalar, height / scalar};
    }
    
    constexpr size& operator+=(const size& other) {
        width += other.width;
        height += other.height;
        return *this;
    }
    
    constexpr size& operator-=(const size& other) {
        width -= other.width;
        height -= other.height;
        return *this;
    }
    
    constexpr size& operator*=(T scalar) {
        width *= scalar;
        height *= scalar;
        return *this;
    }
    
    constexpr size& operator/=(T scalar) {
        width /= scalar;
        height /= scalar;
        return *this;
    }
    
    // Comparison
    constexpr bool operator==(const size&) const = default;
    constexpr auto operator<=>(const size&) const = default;
    
    // Utility methods
    constexpr T aspect_ratio() const {
        return height > 0 ? width / height : 0;
    }
    
    constexpr size fit_within(const size& bounds) const {
        if (empty() || bounds.empty()) return {};
        
        T scale = std::min(bounds.width / width, bounds.height / height);
        return {width * scale, height * scale};
    }
};

// Stream operator
template<typename T>
std::ostream& operator<<(std::ostream& os, const size<T>& s) {
    return os << s.width << "x" << s.height;
}

// Scalar multiplication (scalar first)
template<typename T>
constexpr size<T> operator*(T scalar, const size<T>& s) {
    return s * scalar;
}

/**
 * @brief Built-in rectangle type
 * @tparam T The coordinate/dimension type (int, float, double, etc.)
 */
template<typename T>
struct rect {
    using value_type = T;
    T x{}, y{}, w{}, h{};
    
    constexpr rect() = default;
    constexpr rect(T x_, T y_, T w_, T h_) : x(x_), y(y_), w(w_), h(h_) {}
    constexpr rect(const point<T>& pos, const size<T>& sz)
        : x(pos.x), y(pos.y), w(sz.width), h(sz.height) {}
    
    // Conversion constructor
    template<typename U>
    explicit constexpr rect(const rect<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)),
          w(static_cast<T>(other.w)), h(static_cast<T>(other.h)) {}
    
    constexpr T area() const { return w * h; }
    constexpr bool empty() const { return w <= 0 || h <= 0; }
    
    constexpr point<T> position() const { return {x, y}; }
    constexpr sdlpp::size<T> dimensions() const { return {w, h}; }
    
    constexpr T left() const { return x; }
    constexpr T top() const { return y; }
    constexpr T right() const { return x + w; }
    constexpr T bottom() const { return y + h; }
    
    constexpr point<T> top_left() const { return {x, y}; }
    constexpr point<T> top_right() const { return {x + w, y}; }
    constexpr point<T> bottom_left() const { return {x, y + h}; }
    constexpr point<T> bottom_right() const { return {x + w, y + h}; }
    constexpr point<T> center() const { return {x + w / 2, y + h / 2}; }
    
    constexpr bool contains(const point<T>& p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }
    
    constexpr bool contains(const rect& other) const {
        return other.x >= x && other.y >= y &&
               other.x + other.w <= x + w &&
               other.y + other.h <= y + h;
    }
    
    constexpr bool intersects(const rect& other) const {
        return x < other.x + other.w && x + w > other.x &&
               y < other.y + other.h && y + h > other.y;
    }
    
    constexpr rect intersection(const rect& other) const {
        T x1 = std::max(x, other.x);
        T y1 = std::max(y, other.y);
        T x2 = std::min(x + w, other.x + other.w);
        T y2 = std::min(y + h, other.y + other.h);
        
        if (x2 > x1 && y2 > y1) {
            return {x1, y1, x2 - x1, y2 - y1};
        }
        return {};
    }
    
    constexpr rect unite(const rect& other) const {
        if (empty()) return other;
        if (other.empty()) return *this;
        
        T x1 = std::min(x, other.x);
        T y1 = std::min(y, other.y);
        T x2 = std::max(x + w, other.x + other.w);
        T y2 = std::max(y + h, other.y + other.h);
        
        return {x1, y1, x2 - x1, y2 - y1};
    }
    
    constexpr rect& move_by(const point<T>& offset) {
        x += offset.x;
        y += offset.y;
        return *this;
    }
    
    constexpr rect moved_by(const point<T>& offset) const {
        return {x + offset.x, y + offset.y, w, h};
    }
    
    constexpr rect& inflate(T dx, T dy) {
        x -= dx;
        y -= dy;
        w += 2 * dx;
        h += 2 * dy;
        return *this;
    }
    
    constexpr rect inflated(T dx, T dy) const {
        return {x - dx, y - dy, w + 2 * dx, h + 2 * dy};
    }
    
    // Comparison
    constexpr bool operator==(const rect&) const = default;
    constexpr auto operator<=>(const rect&) const = default;
};

// Stream operator
template<typename T>
std::ostream& operator<<(std::ostream& os, const rect<T>& r) {
    return os << "[" << r.x << ", " << r.y << ", " << r.w << ", " << r.h << "]";
}

/**
 * @brief Built-in line type
 * @tparam T The coordinate type (int, float, double, etc.)
 */
template<typename T>
struct line {
    using value_type = T;
    T x1{}, y1{}, x2{}, y2{};
    
    constexpr line() = default;
    constexpr line(T x1_, T y1_, T x2_, T y2_) 
        : x1(x1_), y1(y1_), x2(x2_), y2(y2_) {}
    constexpr line(const point<T>& p1, const point<T>& p2)
        : x1(p1.x), y1(p1.y), x2(p2.x), y2(p2.y) {}
    
    // Conversion constructor
    template<typename U>
    explicit constexpr line(const line<U>& other)
        : x1(static_cast<T>(other.x1)), y1(static_cast<T>(other.y1)),
          x2(static_cast<T>(other.x2)), y2(static_cast<T>(other.y2)) {}
    
    constexpr point<T> start() const { return {x1, y1}; }
    constexpr point<T> end() const { return {x2, y2}; }
    
    constexpr point<T> vector() const { return {x2 - x1, y2 - y1}; }
    
    T length() const {
        T dx = x2 - x1;
        T dy = y2 - y1;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    constexpr T length_squared() const {
        T dx = x2 - x1;
        T dy = y2 - y1;
        return dx * dx + dy * dy;
    }
    
    constexpr point<T> midpoint() const {
        return {(x1 + x2) / 2, (y1 + y2) / 2};
    }
    
    // Comparison
    constexpr bool operator==(const line&) const = default;
    constexpr auto operator<=>(const line&) const = default;
};

// Stream operator
template<typename T>
std::ostream& operator<<(std::ostream& os, const line<T>& l) {
    return os << "(" << l.x1 << ", " << l.y1 << ") -> (" << l.x2 << ", " << l.y2 << ")";
}

/**
 * @brief Built-in circle type
 * @tparam T The coordinate/radius type (int, float, double, etc.)
 */
template<typename T>
struct circle {
    using value_type = T;
    T x{}, y{}, radius{};
    
    constexpr circle() = default;
    constexpr circle(T x_, T y_, T r) : x(x_), y(y_), radius(r) {}
    constexpr circle(const point<T>& center, T r) : x(center.x), y(center.y), radius(r) {}
    
    // Conversion constructor
    template<typename U>
    explicit constexpr circle(const circle<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)),
          radius(static_cast<T>(other.radius)) {}
    
    constexpr point<T> center() const { return {x, y}; }
    
    constexpr T area() const {
        return static_cast<T>(3.14159265358979323846) * radius * radius;
    }
    
    constexpr T circumference() const {
        return static_cast<T>(2 * 3.14159265358979323846) * radius;
    }
    
    constexpr bool contains(const point<T>& p) const {
        T dx = p.x - x;
        T dy = p.y - y;
        return dx * dx + dy * dy <= radius * radius;
    }
    
    constexpr bool intersects(const circle& other) const {
        T dx = other.x - x;
        T dy = other.y - y;
        T distance_squared = dx * dx + dy * dy;
        T radius_sum = radius + other.radius;
        return distance_squared <= radius_sum * radius_sum;
    }
    
    constexpr rect<T> bounding_rect() const {
        return {x - radius, y - radius, radius * 2, radius * 2};
    }
    
    // Comparison
    constexpr bool operator==(const circle&) const = default;
    constexpr auto operator<=>(const circle&) const = default;
};

// Stream operator
template<typename T>
std::ostream& operator<<(std::ostream& os, const circle<T>& c) {
    return os << "Circle(" << c.x << ", " << c.y << ", r=" << c.radius << ")";
}

/**
 * @brief Built-in triangle type
 * @tparam T The coordinate type (int, float, double, etc.)
 */
template<typename T>
struct triangle {
    point<T> a, b, c;
    
    constexpr triangle() = default;
    constexpr triangle(const point<T>& a_, const point<T>& b_, const point<T>& c_)
        : a(a_), b(b_), c(c_) {}
    constexpr triangle(T ax, T ay, T bx, T by, T cx, T cy)
        : a(ax, ay), b(bx, by), c(cx, cy) {}
    
    // Conversion constructor
    template<typename U>
    explicit constexpr triangle(const triangle<U>& other)
        : a(point<T>(other.a)), b(point<T>(other.b)), c(point<T>(other.c)) {}
    
    constexpr T area() const {
        // Using the cross product formula
        T cross = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
        return std::abs(cross) / 2;
    }
    
    constexpr point<T> centroid() const {
        return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
    }
    
    constexpr bool contains(const point<T>& p) const {
        // Barycentric coordinates method
        auto sign = [](const point<T>& p1, const point<T>& p2, const point<T>& p3) {
            return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
        };
        
        T d1 = sign(p, a, b);
        T d2 = sign(p, b, c);
        T d3 = sign(p, c, a);
        
        bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        
        return !(has_neg && has_pos);
    }
    
    constexpr rect<T> bounding_rect() const {
        T min_x = std::min({a.x, b.x, c.x});
        T min_y = std::min({a.y, b.y, c.y});
        T max_x = std::max({a.x, b.x, c.x});
        T max_y = std::max({a.y, b.y, c.y});
        
        return {min_x, min_y, max_x - min_x, max_y - min_y};
    }
    
    // Comparison
    constexpr bool operator==(const triangle&) const = default;
};

// Stream operator
template<typename T>
std::ostream& operator<<(std::ostream& os, const triangle<T>& t) {
    return os << "Triangle(" << t.a << ", " << t.b << ", " << t.c << ")";
}

// Verify our types satisfy our concepts
static_assert(point_like<point<int>>);
static_assert(point_like<point<float>>);
static_assert(size_like<size<int>>);
static_assert(size_like<size<float>>);
static_assert(rect_like<rect<int>>);
static_assert(rect_like<rect<float>>);
static_assert(line_like<line<int>>);
static_assert(line_like<line<float>>);
static_assert(circle_like<circle<int>>);
static_assert(circle_like<circle<float>>);
static_assert(triangle_like<triangle<int>>);
static_assert(triangle_like<triangle<float>>);

// Common type aliases
using point_i = point<int>;
using point_f = point<float>;
using point_d = point<double>;

using size_i = size<int>;
using size_f = size<float>;
using size_d = size<double>;

using rect_i = rect<int>;
using rect_f = rect<float>;
using rect_d = rect<double>;

using line_i = line<int>;
using line_f = line<float>;
using line_d = line<double>;

using circle_i = circle<int>;
using circle_f = circle<float>;
using circle_d = circle<double>;

using triangle_i = triangle<int>;
using triangle_f = triangle<float>;
using triangle_d = triangle<double>;

// Deduction guides
template<typename T> point(T, T) -> point<T>;
template<typename T> size(T, T) -> size<T>;
template<typename T> rect(T, T, T, T) -> rect<T>;
template<typename T> line(T, T, T, T) -> line<T>;
template<typename T> circle(T, T, T) -> circle<T>;

} // namespace sdlpp

// Hash specializations for use in unordered containers
namespace std {

template<typename T>
struct hash<sdlpp::point<T>> {
    size_t operator()(const sdlpp::point<T>& p) const noexcept {
        size_t h1 = hash<T>{}(p.x);
        size_t h2 = hash<T>{}(p.y);
        return h1 ^ (h2 << 1);
    }
};

template<typename T>
struct hash<sdlpp::size<T>> {
    size_t operator()(const sdlpp::size<T>& s) const noexcept {
        size_t h1 = hash<T>{}(s.width);
        size_t h2 = hash<T>{}(s.height);
        return h1 ^ (h2 << 1);
    }
};

template<typename T>
struct hash<sdlpp::rect<T>> {
    size_t operator()(const sdlpp::rect<T>& r) const noexcept {
        size_t h1 = hash<T>{}(r.x);
        size_t h2 = hash<T>{}(r.y);
        size_t h3 = hash<T>{}(r.w);
        size_t h4 = hash<T>{}(r.h);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

template<typename T>
struct hash<sdlpp::line<T>> {
    size_t operator()(const sdlpp::line<T>& l) const noexcept {
        size_t h1 = hash<T>{}(l.x1);
        size_t h2 = hash<T>{}(l.y1);
        size_t h3 = hash<T>{}(l.x2);
        size_t h4 = hash<T>{}(l.y2);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

template<typename T>
struct hash<sdlpp::circle<T>> {
    size_t operator()(const sdlpp::circle<T>& c) const noexcept {
        size_t h1 = hash<T>{}(c.x);
        size_t h2 = hash<T>{}(c.y);
        size_t h3 = hash<T>{}(c.radius);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

} // namespace std

#endif // SDLPP_UTILITY_GEOMETRY_TYPES_HH