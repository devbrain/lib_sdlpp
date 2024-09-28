//
// Created by igor on 03/06/2020.
//

#ifndef NEUTRINO_SDL_GEOMETRY_HH
#define NEUTRINO_SDL_GEOMETRY_HH

#include <utility>
#include <cmath>
#include <vector>
#include <type_traits>
#include <array>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>

#include <sdlpp/geometry/point.hh>
#include <sdlpp/geometry/area.hh>
#include <sdlpp/geometry/rect.hh>


namespace neutrino::sdl {

	using area_type = generic::area_type<unsigned int>;
	using area_typef = generic::area_type<float>;

	using point2f = math::vector<float, 2>;
	using point = math::vector<int, 2>;

	inline
	area_type operator*(const area_type& a, int scale) {
		return {scale * static_cast <unsigned int>(a.w), scale * static_cast <unsigned int>(a.h)};
	}

	inline
	area_type operator*(int scale, const area_type& a) {
		return {scale * static_cast <unsigned int>(a.w), scale * static_cast <unsigned int>(a.h)};
	}

	namespace detail {
		template <typename T>
		static inline constexpr auto is_point_v = std::is_same_v<T, point> || std::is_same_v<T, point2f>
												  || std::is_same_v<T, SDL_Point> || std::is_same_v<T, SDL_FPoint>;
	}

	template <typename P>
	[[nodiscard]] float distance(const P& a, const P& b, std::enable_if_t<detail::is_point_v<P>>* = nullptr) {
		return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
	}

	template <typename P>
	[[nodiscard]]
	float distance(const P& a, std::enable_if_t<detail::is_point_v<P>>* = nullptr) {
		return std::sqrt(std::pow(a.x, 2) + std::pow(a.y, 2));
	}

	using rect = generic::rect<int>;
	using rectf = generic::rect<float>;


	d_SDLPP_OSTREAM_WITHOT_FROM_STRING(const rect&);
	d_SDLPP_OSTREAM_WITHOT_FROM_STRING(const rectf&);
} // ns sdl

#endif
