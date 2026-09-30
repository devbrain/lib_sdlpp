#pragma once

/**
 * @file geometry_conversion.hh
 * @brief SDL geometry conversion utilities
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/utility/geometry_concepts.hh>
#include <optional>

namespace sdlpp::detail {

    template<point_like P>
    [[nodiscard]] constexpr SDL_Point to_sdl_point(const P& p) {
        return SDL_Point{
            static_cast<int>(get_x(p)),
            static_cast<int>(get_y(p))
        };
    }

    template<point_like P>
    [[nodiscard]] constexpr SDL_FPoint to_sdl_fpoint(const P& p) {
        return SDL_FPoint{
            static_cast<float>(get_x(p)),
            static_cast<float>(get_y(p))
        };
    }

    template<rect_like R>
    [[nodiscard]] constexpr SDL_Rect to_sdl_rect(const R& r) {
        return SDL_Rect{
            static_cast<int>(get_x(r)),
            static_cast<int>(get_y(r)),
            static_cast<int>(get_width(r)),
            static_cast<int>(get_height(r))
        };
    }

    template<rect_like R>
    [[nodiscard]] constexpr SDL_FRect to_sdl_frect(const R& r) {
        return SDL_FRect{
            static_cast<float>(get_x(r)),
            static_cast<float>(get_y(r)),
            static_cast<float>(get_width(r)),
            static_cast<float>(get_height(r))
        };
    }

    // Optional converters

    inline constexpr std::optional<SDL_Point> to_optional_sdl_point(std::nullopt_t) noexcept {
        return std::nullopt;
    }

    template<point_like P>
    [[nodiscard]] constexpr std::optional<SDL_Point> to_optional_sdl_point(const P& p) {
        return to_sdl_point(p);
    }

    template<point_like P>
    [[nodiscard]] constexpr std::optional<SDL_Point> to_optional_sdl_point(const std::optional<P>& p) {
        if (p) {
            return to_sdl_point(*p);
        }
        return std::nullopt;
    }

    inline constexpr std::optional<SDL_FPoint> to_optional_sdl_fpoint(std::nullopt_t) noexcept {
        return std::nullopt;
    }

    template<point_like P>
    [[nodiscard]] constexpr std::optional<SDL_FPoint> to_optional_sdl_fpoint(const P& p) {
        return to_sdl_fpoint(p);
    }

    template<point_like P>
    [[nodiscard]] constexpr std::optional<SDL_FPoint> to_optional_sdl_fpoint(const std::optional<P>& p) {
        if (p) {
            return to_sdl_fpoint(*p);
        }
        return std::nullopt;
    }

    inline constexpr std::optional<SDL_Rect> to_optional_sdl_rect(std::nullopt_t) noexcept {
        return std::nullopt;
    }

    template<rect_like R>
    [[nodiscard]] constexpr std::optional<SDL_Rect> to_optional_sdl_rect(const R& r) {
        return to_sdl_rect(r);
    }

    template<rect_like R>
    [[nodiscard]] constexpr std::optional<SDL_Rect> to_optional_sdl_rect(const std::optional<R>& r) {
        if (r) {
            return to_sdl_rect(*r);
        }
        return std::nullopt;
    }

    inline constexpr std::optional<SDL_FRect> to_optional_sdl_frect(std::nullopt_t) noexcept {
        return std::nullopt;
    }

    template<rect_like R>
    [[nodiscard]] constexpr std::optional<SDL_FRect> to_optional_sdl_frect(const R& r) {
        return to_sdl_frect(r);
    }

    template<rect_like R>
    [[nodiscard]] constexpr std::optional<SDL_FRect> to_optional_sdl_frect(const std::optional<R>& r) {
        if (r) {
            return to_sdl_frect(*r);
        }
        return std::nullopt;
    }

} // namespace sdlpp::detail
