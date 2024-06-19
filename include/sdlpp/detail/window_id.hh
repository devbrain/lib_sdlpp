//
// Created by igor on 6/19/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DETAIL_WINDOW_ID_HH_
#define SDLPP_INCLUDE_SDLPP_DETAIL_WINDOW_ID_HH_

#include <cstdint>
#include <strong_type/strong_type.hpp>

namespace neutrino::sdl {
	namespace detail {
		struct _window_id_;
	}
	using window_id_t = strong::type<uint32_t,
									 detail::_window_id_,
									 strong::ordered,
									 strong::equality,
									 strong::ostreamable>;
}

#endif //SDLPP_INCLUDE_SDLPP_DETAIL_WINDOW_ID_HH_
