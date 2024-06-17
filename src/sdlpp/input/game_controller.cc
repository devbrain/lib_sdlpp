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

	BEGIN_IMPL_OUTPUT(game_controller::type)
			case game_controller::type::UNKNOWN: return "UNKNOWN";
			case game_controller::type::XBOX360: return "XBOX360";
			case game_controller::type::XBOXONE: return "XBOXONE";
			case game_controller::type::PS3: return "PS3";
			case game_controller::type::PS4: return "PS4";
			case game_controller::type::NINTENDO_SWITCH_PRO: return "NINTENDO_SWITCH_PRO";
			case game_controller::type::VIRTUAL: return "VIRTUAL";
			case game_controller::type::PS5: return "PS5";
			case game_controller::type::AMAZON_LUNA: return "AMAZON_LUNA";
			case game_controller::type::GOOGLE_STADIA: return "GOOGLE_STADIA";
			case game_controller::type::NVIDIA_SHIELD: return "NVIDIA_SHIELD";
			case game_controller::type::NINTENDO_SWITCH_JOYCON_LEFT: return "NINTENDO_SWITCH_JOYCON_LEFT";
			case game_controller::type::NINTENDO_SWITCH_JOYCON_RIGHT: return "NINTENDO_SWITCH_JOYCON_RIGHT";
			case game_controller::type::NINTENDO_SWITCH_JOYCON_PAIR: return "NINTENDO_SWITCH_JOYCON_PAIR";
	END_IMPL_OUTPUT(game_controller::type)

	BEGIN_IMPL_OUTPUT(game_controller::button)
			case game_controller::button::A: return "A";
			case game_controller::button::B: return "B";
			case game_controller::button::X: return "X";
			case game_controller::button::Y: return "Y";
			case game_controller::button::BACK: return "BACK";
			case game_controller::button::GUIDE: return "GUIDE";
			case game_controller::button::START: return "START";
			case game_controller::button::LEFTSTICK: return "LEFTSTICK";
			case game_controller::button::RIGHTSTICK: return "RIGHTSTICK";
			case game_controller::button::LEFTSHOULDER: return "LEFTSHOULDER";
			case game_controller::button::RIGHTSHOULDER: return "RIGHTSHOULDER";
			case game_controller::button::DPAD_UP: return "DPAD_UP";
			case game_controller::button::DPAD_DOWN: return "DPAD_DOWN";
			case game_controller::button::DPAD_LEFT: return "DPAD_LEFT";
			case game_controller::button::DPAD_RIGHT: return "DPAD_RIGHT";
			case game_controller::button::MISC1: return "MISC1";
			case game_controller::button::PADDLE1: return "PADDLE1";
			case game_controller::button::PADDLE2: return "PADDLE2";
			case game_controller::button::PADDLE3: return "PADDLE3";
			case game_controller::button::PADDLE4: return "PADDLE4";
			case game_controller::button::TOUCHPAD: return "TOUCHPAD";
	END_IMPL_OUTPUT(game_controller::button)
}