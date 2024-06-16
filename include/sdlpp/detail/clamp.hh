//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DETAIL_CLAMP_HH_
#define SDLPP_INCLUDE_SDLPP_DETAIL_CLAMP_HH_

#include <bsw/errors.hh>

namespace neutrino::sdl {
	template <typename T>
	[[nodiscard]] constexpr auto clamp(const T& value,
									   const T& min,
									   const T& max) noexcept(noexcept(value < min)) -> T
	{
		ENFORCE(min <= max);
		if (value < min) {
			return min;
		}
		else if (max < value) {
			return max;
		}
		else {
			return value;
		}
	}
}

#endif //SDLPP_INCLUDE_SDLPP_DETAIL_CLAMP_HH_
