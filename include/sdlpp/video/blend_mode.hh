//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_VIDEO_BLEND_MODE_HH_
#define SDLPP_INCLUDE_SDLPP_VIDEO_BLEND_MODE_HH_

#include <array>
#include <type_traits>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	enum class blend_mode : uint32_t {
		NONE = SDL_BLENDMODE_NONE,
		BLEND = SDL_BLENDMODE_BLEND,
		ADD = SDL_BLENDMODE_ADD,
		MOD = SDL_BLENDMODE_MOD
	};


	namespace detail {
		static inline constexpr std::array<blend_mode, 4> s_vals_of_blend_mode = {
			blend_mode::NONE,
			blend_mode::BLEND,
			blend_mode::ADD,
			blend_mode::MOD,
		};
	}
	template <typename T>
	static constexpr const decltype(detail::s_vals_of_blend_mode)&
	values(std::enable_if_t<std::is_same_v<blend_mode, T>>* = nullptr) {
		return detail::s_vals_of_blend_mode;
	}
	template <typename T>
	static constexpr auto
	begin(std::enable_if_t<std::is_same_v<blend_mode, T>>* = nullptr) {
		return detail::s_vals_of_blend_mode.begin();
	}
	template <typename T>
	static constexpr auto
	end(std::enable_if_t<std::is_same_v<blend_mode, T>>* = nullptr) {
		return detail::s_vals_of_blend_mode.end();
	}

	d_SDLPP_OSTREAM(blend_mode);
}

#endif //SDLPP_INCLUDE_SDLPP_VIDEO_BLEND_MODE_HH_
