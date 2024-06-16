//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_

#include <string>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	class game_controller : public object<SDL_GameController> {
	 public:
		enum class axis {
			// SDL_CONTROLLER_AXIS_INVALID = -1,
			LEFTX = SDL_CONTROLLER_AXIS_LEFTX,
			LEFTY = SDL_CONTROLLER_AXIS_LEFTY,
			RIGHTX = SDL_CONTROLLER_AXIS_RIGHTX,
			RIGHTY = SDL_CONTROLLER_AXIS_RIGHTY,
			TRIGGERLEFT = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
			TRIGGERRIGHT = SDL_CONTROLLER_AXIS_TRIGGERRIGHT
		};
	 public:
		game_controller() = default;
		game_controller& operator= (object<SDL_GameController>&& other) noexcept;
		game_controller& operator= (game_controller&& other) noexcept;

		game_controller& operator= (const game_controller& other) = delete;
		game_controller (const game_controller& other) = delete;

		[[nodiscard]] bool add_mapping(const std::string& mapping);
		[[nodiscard]] bool is_connected() const;

		[[nodiscard]] int16_t get_axis(axis a) const;
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

	inline
	bool game_controller::add_mapping (const std::string& mapping) {
		auto rc = SDL_GameControllerAddMapping (mapping.c_str());
		if (rc == -1) {
			RAISE_SDL_EX("Failed to add mapping");
		}
		return rc == 1;
	}

	inline
	bool game_controller::is_connected () const {
		return SDL_GameControllerGetAttached (const_handle()) == SDL_TRUE;
	}

	inline
	int16_t game_controller::get_axis (game_controller::axis a) const {
		return SDL_GameControllerGetAxis(const_handle(), static_cast<SDL_GameControllerAxis>(a));
	}

	d_SDLPP_OSTREAM(game_controller::axis);
}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_
