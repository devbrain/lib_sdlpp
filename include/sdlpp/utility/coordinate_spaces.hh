//
// Typed coordinate spaces
//

#pragma once

/**
 * @file coordinate_spaces.hh
 * @brief Coordinates that know which space they belong to.
 *
 * SDL works in several coordinate spaces that are all a pair of numbers: the desktop, a
 * window, the renderer's logical space, its real output pixels, the pixels of a texture,
 * fractions of a texture, fractions of a window. Passing a value of one where another is expected compiles and
 * goes wrong silently — window coordinates handed to a draw call work until logical
 * presentation or HiDPI is switched on.
 *
 * Here every coordinate has a **space** (a tag type) and a **kind**:
 *
 * | kind            | is         | fields          |
 * |-----------------|------------|-----------------|
 * | @ref pos        | a place    | x, y            |
 * | @ref delta      | a displacement, or a point measured from another point | x, y |
 * | @ref extent     | a size     | width, height   |
 * | @ref area       | a rectangle| x, y, w, h      |
 *
 * Only the operations that mean something compile:
 *
 *     pos - pos         = delta        pos + pos         does not compile
 *     pos ± delta       = pos          pos * s           does not compile
 *     delta ± delta     = delta        render_pos - window_pos   does not compile
 *     delta * s, -delta = delta
 *     extent * s        = extent
 *
 * - **Int to float in the same space is implicit**; mixing int and float values gives float.
 *   **Float to int is explicit**, through round_to_int(), floor_to_int() and ceil_to_int().
 * - **Crossing spaces is a named call** on the object that knows the mapping:
 *   renderer::to_render(), renderer::to_window(), renderer::to_output(), window::to_window(),
 *   window::to_display(), texture::to_uv(), texture::to_texel().
 * - **Float types have the layout of SDL_FPoint / SDL_FRect** and int ones that of
 *   SDL_Point / SDL_Rect (checked below), so they can be handed to SDL without copying.
 * - **Entering a space from untyped data is explicit:** `render_pos{v.x, v.y}`.
 *
 * Libraries can add spaces of their own: a space is any type used as the tag.
 */

#include <SDL3/SDL_rect.h>

#include <cmath>
#include <concepts>
#include <cstddef>
#include <ostream>
#include <type_traits>

namespace sdlpp {
    // ---- spaces ---------------------------------------------------------------------------

    /// @brief The desktop: where windows sit, display bounds, the global mouse. Int.
    struct display_space {};
    /// @brief SDL's window coordinates ("points"): what mouse, pen and drop events carry.
    ///        Equal to the window's pixels unless its pixel density is not 1. Float.
    struct window_space {};
    /// @brief What draw calls take: the bound target's coordinates after logical presentation,
    ///        the viewport and the scale. Inside a texture target, its pixels. Float.
    struct render_space {};
    /// @brief The renderer's real output in pixels, ignoring logical presentation: the output
    ///        size, read-back pixels, the logical presentation rectangle. Int.
    struct output_space {};
    /// @brief Pixels inside a texture: source rectangles, atlas rectangles, texture sizes. Int.
    struct texel_space {};
    /// @brief Fractions of a texture, 0 to 1 across it (beyond that with the wrap address
    ///        mode): the texture coordinates of geometry. Float.
    struct uv_space {};
    /// @brief Fractions of the window, 0 to 1: touch finger positions. Float.
    struct normalized_space {};

    /// @brief A number a coordinate can hold: any arithmetic type but bool.
    template<typename T>
    concept coordinate_value = std::is_arithmetic_v <T> && !std::same_as <T, bool>;

    namespace detail {
        // An int value widens to a float one implicitly; nothing else converts implicitly.
        template<typename From, typename To>
        concept widens_to = std::integral <From> && std::floating_point <To>;

        // Coordinates mixed in one expression: int and float give float.
        template<typename A, typename B>
        using coord_common_t = std::common_type_t <A, B>;

        // A coordinate scaled by a number keeps a float type; an int one takes the scalar's.
        template<typename T, typename S>
        using scaled_t = std::conditional_t <std::floating_point <T>, T, std::common_type_t <T, S>>;
    }

    // ---- delta ----------------------------------------------------------------------------

    /**
     * @brief A displacement in @p Space: the gap between two places, a distance moved, or a
     *        point measured from another one (a pivot inside a rectangle, an anchor inside a
     *        sprite).
     */
    template<typename Space, coordinate_value T = float>
    struct delta {
        using space = Space;
        using value_type = T;

        T x{};
        T y{};

        /// @brief The same delta with float values: int widens implicitly.
        template<coordinate_value U> requires detail::widens_to <T, U>
        constexpr operator delta <Space, U>() const noexcept {
            return {static_cast <U>(x), static_cast <U>(y)};
        }

        /// @brief The length. Float deltas only.
        [[nodiscard]] T length() const noexcept requires std::floating_point <T> {
            return std::sqrt(x * x + y * y);
        }

        /// @brief The squared length: compares lengths without a square root.
        [[nodiscard]] constexpr T length_sq() const noexcept {
            return x * x + y * y;
        }

        /// @brief The same direction with length 1; a zero delta stays zero. Float deltas only.
        [[nodiscard]] delta normalized() const noexcept requires std::floating_point <T> {
            const T len = length();
            return len > T{0} ? delta{x / len, y / len} : delta{};
        }

        friend constexpr bool operator==(const delta&, const delta&) = default;
    };

    // ---- extent ---------------------------------------------------------------------------

    /// @brief A size in @p Space. The field names are those of sdlpp::size and SDL.
    template<typename Space, coordinate_value T = float>
    struct extent {
        using space = Space;
        using value_type = T;

        T width{};
        T height{};

        /// @brief The same extent with float values: int widens implicitly.
        template<coordinate_value U> requires detail::widens_to <T, U>
        constexpr operator extent <Space, U>() const noexcept {
            return {static_cast <U>(width), static_cast <U>(height)};
        }

        /// @brief True if either side is zero or negative: it covers nothing.
        [[nodiscard]] constexpr bool empty() const noexcept {
            return width <= T{0} || height <= T{0};
        }

        friend constexpr bool operator==(const extent&, const extent&) = default;
    };

    // ---- pos ------------------------------------------------------------------------------

    /// @brief A place in @p Space. Two places do not add; their difference is a @ref delta.
    template<typename Space, coordinate_value T = float>
    struct pos {
        using space = Space;
        using value_type = T;

        T x{};
        T y{};

        /// @brief The same place with float values: int widens implicitly.
        template<coordinate_value U> requires detail::widens_to <T, U>
        constexpr operator pos <Space, U>() const noexcept {
            return {static_cast <U>(x), static_cast <U>(y)};
        }

        friend constexpr bool operator==(const pos&, const pos&) = default;
    };

    // ---- area -----------------------------------------------------------------------------

    /**
     * @brief A rectangle in @p Space: its top-left corner and its size, as SDL_Rect /
     *        SDL_FRect have them.
     *
     * contains() is half-open, for int and float alike: a rectangle holds its left and top
     * edges, not its right and bottom ones, so two rectangles side by side never share a
     * point.
     */
    template<typename Space, coordinate_value T = float>
    struct area {
        using space = Space;
        using value_type = T;

        T x{};
        T y{};
        T w{};
        T h{};

        constexpr area() noexcept = default;

        constexpr area(T x_, T y_, T w_, T h_) noexcept
            : x(x_), y(y_), w(w_), h(h_) {
        }

        /// @brief The rectangle at @p origin of size @p size.
        constexpr area(const pos <Space, T>& origin, const extent <Space, T>& size) noexcept
            : x(origin.x), y(origin.y), w(size.width), h(size.height) {
        }

        /// @brief The same rectangle with float values: int widens implicitly.
        template<coordinate_value U> requires detail::widens_to <T, U>
        constexpr operator area <Space, U>() const noexcept {
            return {static_cast <U>(x), static_cast <U>(y), static_cast <U>(w), static_cast <U>(h)};
        }

        /// @brief The top-left corner.
        [[nodiscard]] constexpr pos <Space, T> origin() const noexcept {
            return {x, y};
        }

        /// @brief The size.
        [[nodiscard]] constexpr extent <Space, T> size() const noexcept {
            return {w, h};
        }

        /// @brief True if either side is zero or negative: it covers nothing.
        [[nodiscard]] constexpr bool empty() const noexcept {
            return w <= T{0} || h <= T{0};
        }

        /// @brief True if @p p is inside: left and top edges included, right and bottom not.
        template<coordinate_value U>
        [[nodiscard]] constexpr bool contains(const pos <Space, U>& p) const noexcept {
            return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
        }

        /// @brief True if the two overlap by more than an edge. An empty rectangle overlaps
        ///        nothing.
        template<coordinate_value U>
        [[nodiscard]] constexpr bool intersects(const area <Space, U>& o) const noexcept {
            return !empty() && !o.empty()
                   && o.x < x + w && x < o.x + o.w && o.y < y + h && y < o.y + o.h;
        }

        friend constexpr bool operator==(const area&, const area&) = default;
    };

    // ---- the algebra ----------------------------------------------------------------------
    // Templates over the space, so values of two spaces never match. int and float values of
    // one space mix, and give float.

    /// @brief Two places: the delta from @p from to @p to.
    template<typename S, typename A, typename B>
    [[nodiscard]] constexpr delta <S, detail::coord_common_t <A, B>> operator-(
        const pos <S, A>& to, const pos <S, B>& from) noexcept {
        return {to.x - from.x, to.y - from.y};
    }

    /// @brief A place, displaced.
    template<typename S, typename A, typename B>
    [[nodiscard]] constexpr pos <S, detail::coord_common_t <A, B>> operator+(
        const pos <S, A>& p, const delta <S, B>& d) noexcept {
        return {p.x + d.x, p.y + d.y};
    }

    /// @copydoc operator+(const pos<S, A>&, const delta<S, B>&)
    template<typename S, typename A, typename B>
    [[nodiscard]] constexpr pos <S, detail::coord_common_t <A, B>> operator+(
        const delta <S, A>& d, const pos <S, B>& p) noexcept {
        return p + d;
    }

    /// @brief A place, displaced backwards.
    template<typename S, typename A, typename B>
    [[nodiscard]] constexpr pos <S, detail::coord_common_t <A, B>> operator-(
        const pos <S, A>& p, const delta <S, B>& d) noexcept {
        return {p.x - d.x, p.y - d.y};
    }

    /// @brief Moves @p p by @p d. An int place takes only an int delta.
    template<typename S, typename A, typename B>
        requires std::same_as <detail::coord_common_t <A, B>, A>
    constexpr pos <S, A>& operator+=(pos <S, A>& p, const delta <S, B>& d) noexcept {
        p.x += d.x;
        p.y += d.y;
        return p;
    }

    /// @brief Moves @p p back by @p d. An int place takes only an int delta.
    template<typename S, typename A, typename B>
        requires std::same_as <detail::coord_common_t <A, B>, A>
    constexpr pos <S, A>& operator-=(pos <S, A>& p, const delta <S, B>& d) noexcept {
        p.x -= d.x;
        p.y -= d.y;
        return p;
    }

    template<typename S, typename A, typename B>
    [[nodiscard]] constexpr delta <S, detail::coord_common_t <A, B>> operator+(
        const delta <S, A>& a, const delta <S, B>& b) noexcept {
        return {a.x + b.x, a.y + b.y};
    }

    template<typename S, typename A, typename B>
    [[nodiscard]] constexpr delta <S, detail::coord_common_t <A, B>> operator-(
        const delta <S, A>& a, const delta <S, B>& b) noexcept {
        return {a.x - b.x, a.y - b.y};
    }

    template<typename S, typename T>
    [[nodiscard]] constexpr delta <S, T> operator-(const delta <S, T>& d) noexcept {
        return {-d.x, -d.y};
    }

    template<typename S, typename A, typename B>
        requires std::same_as <detail::coord_common_t <A, B>, A>
    constexpr delta <S, A>& operator+=(delta <S, A>& a, const delta <S, B>& b) noexcept {
        a.x += b.x;
        a.y += b.y;
        return a;
    }

    template<typename S, typename A, typename B>
        requires std::same_as <detail::coord_common_t <A, B>, A>
    constexpr delta <S, A>& operator-=(delta <S, A>& a, const delta <S, B>& b) noexcept {
        a.x -= b.x;
        a.y -= b.y;
        return a;
    }

    /// @brief A delta scaled. A float delta stays float; an int one takes the scalar's type.
    template<typename S, typename T, coordinate_value U>
    [[nodiscard]] constexpr delta <S, detail::scaled_t <T, U>> operator*(
        const delta <S, T>& d, U s) noexcept {
        using R = detail::scaled_t <T, U>;
        return {static_cast <R>(d.x * s), static_cast <R>(d.y * s)};
    }

    /// @copydoc operator*(const delta<S, T>&, U)
    template<typename S, typename T, coordinate_value U>
    [[nodiscard]] constexpr delta <S, detail::scaled_t <T, U>> operator*(
        U s, const delta <S, T>& d) noexcept {
        return d * s;
    }

    /// @brief Scales @p d in place. An int delta takes only an int scalar.
    template<typename S, typename T, coordinate_value U>
        requires std::same_as <detail::scaled_t <T, U>, T>
    constexpr delta <S, T>& operator*=(delta <S, T>& d, U s) noexcept {
        d.x = static_cast <T>(d.x * s);
        d.y = static_cast <T>(d.y * s);
        return d;
    }

    /// @brief An extent scaled. A float extent stays float; an int one takes the scalar's type.
    template<typename S, typename T, coordinate_value U>
    [[nodiscard]] constexpr extent <S, detail::scaled_t <T, U>> operator*(
        const extent <S, T>& e, U s) noexcept {
        using R = detail::scaled_t <T, U>;
        return {static_cast <R>(e.width * s), static_cast <R>(e.height * s)};
    }

    /// @copydoc operator*(const extent<S, T>&, U)
    template<typename S, typename T, coordinate_value U>
    [[nodiscard]] constexpr extent <S, detail::scaled_t <T, U>> operator*(
        U s, const extent <S, T>& e) noexcept {
        return e * s;
    }

    /// @brief A rectangle moved by @p d; its size is unchanged.
    template<typename S, typename A, typename B>
    [[nodiscard]] constexpr area <S, detail::coord_common_t <A, B>> operator+(
        const area <S, A>& a, const delta <S, B>& d) noexcept {
        using R = detail::coord_common_t <A, B>;
        return {static_cast <R>(a.x + d.x), static_cast <R>(a.y + d.y), static_cast <R>(a.w),
                static_cast <R>(a.h)};
    }

    /// @brief A rectangle moved back by @p d; its size is unchanged.
    template<typename S, typename A, typename B>
    [[nodiscard]] constexpr area <S, detail::coord_common_t <A, B>> operator-(
        const area <S, A>& a, const delta <S, B>& d) noexcept {
        return a + (-d);
    }

    // ---- float to int: explicit, naming the rounding ----------------------------------------

    /// @brief Each value rounded to the nearest int, halves away from zero.
    template<typename S, std::floating_point T>
    [[nodiscard]] pos <S, int> round_to_int(const pos <S, T>& p) noexcept {
        return {static_cast <int>(std::round(p.x)), static_cast <int>(std::round(p.y))};
    }
    /// @copydoc round_to_int(const pos<S, T>&)
    template<typename S, std::floating_point T>
    [[nodiscard]] delta <S, int> round_to_int(const delta <S, T>& d) noexcept {
        return {static_cast <int>(std::round(d.x)), static_cast <int>(std::round(d.y))};
    }
    /// @copydoc round_to_int(const pos<S, T>&)
    template<typename S, std::floating_point T>
    [[nodiscard]] extent <S, int> round_to_int(const extent <S, T>& e) noexcept {
        return {static_cast <int>(std::round(e.width)), static_cast <int>(std::round(e.height))};
    }
    /// @brief The rectangle with its corners rounded to the nearest int, so rectangles that
    ///        share an edge still share it after rounding.
    template<typename S, std::floating_point T>
    [[nodiscard]] area <S, int> round_to_int(const area <S, T>& a) noexcept {
        const int x0 = static_cast <int>(std::round(a.x));
        const int y0 = static_cast <int>(std::round(a.y));
        const int x1 = static_cast <int>(std::round(a.x + a.w));
        const int y1 = static_cast <int>(std::round(a.y + a.h));
        return {x0, y0, x1 - x0, y1 - y0};
    }

    /// @brief Each value rounded down.
    template<typename S, std::floating_point T>
    [[nodiscard]] pos <S, int> floor_to_int(const pos <S, T>& p) noexcept {
        return {static_cast <int>(std::floor(p.x)), static_cast <int>(std::floor(p.y))};
    }
    /// @copydoc floor_to_int(const pos<S, T>&)
    template<typename S, std::floating_point T>
    [[nodiscard]] delta <S, int> floor_to_int(const delta <S, T>& d) noexcept {
        return {static_cast <int>(std::floor(d.x)), static_cast <int>(std::floor(d.y))};
    }
    /// @copydoc floor_to_int(const pos<S, T>&)
    template<typename S, std::floating_point T>
    [[nodiscard]] extent <S, int> floor_to_int(const extent <S, T>& e) noexcept {
        return {static_cast <int>(std::floor(e.width)), static_cast <int>(std::floor(e.height))};
    }

    /// @brief Each value rounded up.
    template<typename S, std::floating_point T>
    [[nodiscard]] pos <S, int> ceil_to_int(const pos <S, T>& p) noexcept {
        return {static_cast <int>(std::ceil(p.x)), static_cast <int>(std::ceil(p.y))};
    }
    /// @copydoc ceil_to_int(const pos<S, T>&)
    template<typename S, std::floating_point T>
    [[nodiscard]] delta <S, int> ceil_to_int(const delta <S, T>& d) noexcept {
        return {static_cast <int>(std::ceil(d.x)), static_cast <int>(std::ceil(d.y))};
    }
    /// @copydoc ceil_to_int(const pos<S, T>&)
    template<typename S, std::floating_point T>
    [[nodiscard]] extent <S, int> ceil_to_int(const extent <S, T>& e) noexcept {
        return {static_cast <int>(std::ceil(e.width)), static_cast <int>(std::ceil(e.height))};
    }

    // ---- printing (test messages, logs) ------------------------------------------------------

    template<typename S, typename T>
    std::ostream& operator<<(std::ostream& os, const pos <S, T>& p) {
        return os << "pos(" << p.x << ", " << p.y << ")";
    }

    template<typename S, typename T>
    std::ostream& operator<<(std::ostream& os, const delta <S, T>& d) {
        return os << "delta(" << d.x << ", " << d.y << ")";
    }

    template<typename S, typename T>
    std::ostream& operator<<(std::ostream& os, const extent <S, T>& e) {
        return os << "extent(" << e.width << " x " << e.height << ")";
    }

    template<typename S, typename T>
    std::ostream& operator<<(std::ostream& os, const area <S, T>& a) {
        return os << "area(" << a.x << ", " << a.y << ", " << a.w << " x " << a.h << ")";
    }

    // ---- the names in use ----------------------------------------------------------------
    // Each space with its own value type: int for display, output and texel, float for the
    // others. The other combinations are spelled out, e.g. extent<render_space, int>.

    using display_pos = pos <display_space, int>;
    using display_delta = delta <display_space, int>;
    using display_extent = extent <display_space, int>;
    using display_rect = area <display_space, int>;

    using window_pos = pos <window_space>;
    using window_delta = delta <window_space>;
    using window_extent = extent <window_space>;
    using window_rect = area <window_space>;

    using render_pos = pos <render_space>;
    using render_delta = delta <render_space>;
    using render_extent = extent <render_space>;
    using render_rect = area <render_space>;

    using output_pos = pos <output_space, int>;
    using output_delta = delta <output_space, int>;
    using output_extent = extent <output_space, int>;
    using output_rect = area <output_space, int>;

    using texel_pos = pos <texel_space, int>;
    using texel_delta = delta <texel_space, int>;
    using texel_extent = extent <texel_space, int>;
    using texel_rect = area <texel_space, int>;

    using uv_pos = pos <uv_space>;

    using normalized_pos = pos <normalized_space>;
    using normalized_delta = delta <normalized_space>;

    // ---- layout: handed to SDL without copying ------------------------------------------------

    namespace detail {
        using layout_fpos = pos <render_space, float>;
        using layout_ipos = pos <display_space, int>;
        using layout_farea = area <render_space, float>;
        using layout_iarea = area <texel_space, int>;

        static_assert(std::is_standard_layout_v <layout_fpos> && std::is_trivially_copyable_v <layout_fpos>);
        static_assert(sizeof(layout_fpos) == sizeof(SDL_FPoint)
                      && offsetof(layout_fpos, x) == offsetof(SDL_FPoint, x)
                      && offsetof(layout_fpos, y) == offsetof(SDL_FPoint, y));
        static_assert(sizeof(layout_ipos) == sizeof(SDL_Point)
                      && offsetof(layout_ipos, x) == offsetof(SDL_Point, x)
                      && offsetof(layout_ipos, y) == offsetof(SDL_Point, y));
        static_assert(std::is_standard_layout_v <layout_farea> && std::is_trivially_copyable_v <layout_farea>);
        static_assert(sizeof(layout_farea) == sizeof(SDL_FRect)
                      && offsetof(layout_farea, x) == offsetof(SDL_FRect, x)
                      && offsetof(layout_farea, y) == offsetof(SDL_FRect, y)
                      && offsetof(layout_farea, w) == offsetof(SDL_FRect, w)
                      && offsetof(layout_farea, h) == offsetof(SDL_FRect, h));
        static_assert(sizeof(layout_iarea) == sizeof(SDL_Rect)
                      && offsetof(layout_iarea, x) == offsetof(SDL_Rect, x)
                      && offsetof(layout_iarea, w) == offsetof(SDL_Rect, w));
    }
}
