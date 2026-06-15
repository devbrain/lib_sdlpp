#pragma once

#include <simplex/dp.hh>
#include <simplex/detail/export.hh>
#include <sdlpp/utility/geometry_concepts.hh>
#include <euler/vector/vector.hh>

namespace simplex {
    struct point {
        using value_type = dp;

        dp x{};
        dp y{};

        constexpr point() = default;
        constexpr point(dp x_, dp y_) noexcept
            : x(x_), y(y_) {
        }

        constexpr point(const euler::vec2<dp>& v) noexcept
            : x(v.x()), y(v.y()) {
        }

        [[nodiscard]] constexpr operator euler::vec2<dp>() const noexcept {
            return {x, y};
        }

        [[nodiscard]] static point from_px(float x, float y) noexcept {
            return {dp::from_px(x), dp::from_px(y)};
        }

        [[nodiscard]] constexpr point operator+(const point& other) const noexcept {
            return {x + other.x, y + other.y};
        }

        [[nodiscard]] constexpr point operator-(const point& other) const noexcept {
            return {x - other.x, y - other.y};
        }

        [[nodiscard]] constexpr point operator*(float scalar) const noexcept {
            return {x * scalar, y * scalar};
        }

        [[nodiscard]] constexpr point operator/(float scalar) const noexcept {
            return {x / scalar, y / scalar};
        }

        constexpr point& operator+=(const point& other) noexcept {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr point& operator-=(const point& other) noexcept {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        friend constexpr auto operator<=>(point, point) = default;
    };

    [[nodiscard]] constexpr point operator*(float scalar, const point& p) noexcept {
        return p * scalar;
    }

    struct size {
        using value_type = dp;

        dp width{};
        dp height{};

        constexpr size() = default;
        constexpr size(dp width_, dp height_) noexcept
            : width(width_), height(height_) {
        }

        constexpr size(const euler::vec2<dp>& v) noexcept
            : width(v.x()), height(v.y()) {
        }

        [[nodiscard]] constexpr operator euler::vec2<dp>() const noexcept {
            return {width, height};
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return width <= dp{} || height <= dp{};
        }

        friend constexpr auto operator<=>(size, size) = default;
    };

    struct rect {
        using value_type = dp;

        dp x{};
        dp y{};
        dp w{};
        dp h{};

        constexpr rect() = default;
        constexpr rect(dp x_, dp y_, dp w_, dp h_) noexcept
            : x(x_), y(y_), w(w_), h(h_) {
        }

        constexpr rect(const point& position, const size& dimensions) noexcept
            : x(position.x), y(position.y), w(dimensions.width), h(dimensions.height) {
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return w <= dp{} || h <= dp{};
        }

        [[nodiscard]] constexpr point position() const noexcept {
            return {x, y};
        }

        [[nodiscard]] constexpr size dimensions() const noexcept {
            return {w, h};
        }

        [[nodiscard]] constexpr dp left() const noexcept { return x; }
        [[nodiscard]] constexpr dp top() const noexcept { return y; }
        [[nodiscard]] constexpr dp right() const noexcept { return x + w; }
        [[nodiscard]] constexpr dp bottom() const noexcept { return y + h; }

        [[nodiscard]] constexpr point top_left() const noexcept { return {left(), top()}; }
        [[nodiscard]] constexpr point top_right() const noexcept { return {right(), top()}; }
        [[nodiscard]] constexpr point bottom_left() const noexcept { return {left(), bottom()}; }
        [[nodiscard]] constexpr point bottom_right() const noexcept { return {right(), bottom()}; }
        [[nodiscard]] constexpr point center() const noexcept { return {x + w / 2.0f, y + h / 2.0f}; }

        [[nodiscard]] constexpr bool contains(const point& p) const noexcept {
            return p.x >= x && p.x < right() && p.y >= y && p.y < bottom();
        }

        friend constexpr auto operator<=>(rect, rect) = default;
    };

    struct line {
        using value_type = dp;

        dp x1{};
        dp y1{};
        dp x2{};
        dp y2{};

        constexpr line() = default;
        constexpr line(dp x1_, dp y1_, dp x2_, dp y2_) noexcept
            : x1(x1_), y1(y1_), x2(x2_), y2(y2_) {
        }

        constexpr line(const point& start, const point& end) noexcept
            : x1(start.x), y1(start.y), x2(end.x), y2(end.y) {
        }

        [[nodiscard]] constexpr point start() const noexcept {
            return {x1, y1};
        }

        [[nodiscard]] constexpr point end() const noexcept {
            return {x2, y2};
        }

        friend constexpr auto operator<=>(line, line) = default;
    };

    struct circle {
        using value_type = dp;

        dp x{};
        dp y{};
        dp radius{};

        constexpr circle() = default;
        constexpr circle(dp x_, dp y_, dp radius_) noexcept
            : x(x_), y(y_), radius(radius_) {
        }

        constexpr circle(const point& center, dp radius_) noexcept
            : x(center.x), y(center.y), radius(radius_) {
        }

        [[nodiscard]] constexpr point center() const noexcept {
            return {x, y};
        }

        friend constexpr auto operator<=>(circle, circle) = default;
    };

    static_assert(sdlpp::point_like<point>);
    static_assert(sdlpp::size_like<size>);
    static_assert(sdlpp::rect_like<rect>);
    static_assert(sdlpp::line_like<line>);
    static_assert(sdlpp::circle_like<circle>);
} // namespace simplex
