//
// Created by igor on 6/16/24.
//
#include <sdlpp/input/game_controller.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(game_controller::axis)
			case game_controller::axis::LEFTX: return "LEFTX";
			case game_controller::axis::LEFTY: return "LEFTY";
			case game_controller::axis::RIGHTX: return "RIGHTX";
			case game_controller::axis::RIGHTY: return "RIGHTY";
			case game_controller::axis::TRIGGERLEFT: return "TRIGGERLEFT";
			case game_controller::axis::TRIGGERRIGHT: return "TRIGGERRIGHT";
	END_IMPL_OUTPUT(game_controller::axis)
}