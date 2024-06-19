//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DETAIL_JOYSTIC_ID_HH_
#define SDLPP_INCLUDE_SDLPP_DETAIL_JOYSTIC_ID_HH_

#include <sdlpp/detail/sdl2.hh>
#include <strong_type/strong_type.hpp>

namespace neutrino::sdl {
	namespace detail {
		struct _joystick_device_id_;
		struct _joystick_player_index_;
		struct _joystick_id_;
	}
	using joystick_device_id_t = strong::type<std::size_t,
											  detail::_joystick_device_id_,
											  strong::bicrementable,
											  strong::ordered,
											  strong::equality,
											  strong::ostreamable>;
	using joystick_player_index_t = strong::type<int,
												 detail::_joystick_player_index_,
												 strong::bicrementable,
												 strong::ordered,
												 strong::equality,
												 strong::ostreamable>;
	using joystick_id_t = strong::type<SDL_JoystickID,
									   detail::_joystick_id_,
									   strong::bicrementable,
									   strong::ordered,
									   strong::equality,
									   strong::ostreamable>;
}

#endif //SDLPP_INCLUDE_SDLPP_DETAIL_JOYSTIC_ID_HH_
