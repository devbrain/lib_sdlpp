//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/object.hh>

namespace neutrino::sdl {
	class game_controller : public object<SDL_GameController> {
	 public:
		game_controller() = default;
		game_controller& operator= (object<SDL_GameController>&& other) noexcept;
		game_controller& operator= (game_controller&& other) noexcept;

		game_controller& operator= (const game_controller& other) = delete;
		game_controller (const game_controller& other) = delete;
	};

	inline
	game_controller& game_controller::operator= (object<SDL_GameController>&& other) noexcept {
		object<SDL_GameController>::operator= (std::move (other));
		return *this;;
	}

	inline
	game_controller& game_controller::operator= (game_controller&& other) noexcept {
		object<SDL_GameController>::operator= (std::move (other));
		return *this;
	}

}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_
