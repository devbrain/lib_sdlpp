//
// Created by igor on 6/13/24.
//
#include "sdlpp/system/joystick.hh"
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(joystick::power_level)
			case joystick::power_level::UNKNOWN: return "UNKNOWN";
			case joystick::power_level::EMPTY: return "EMPTY";
			case joystick::power_level::LOW: return "LOW";
			case joystick::power_level::MEDIUM: return "MEDIUM";
			case joystick::power_level::FULL: return "FULL";
			case joystick::power_level::WIRED: return "WIRED";
			case joystick::power_level::MAX: return "MAX";
	END_IMPL_OUTPUT(joystick::power_level)

	BEGIN_IMPL_OUTPUT(joystick::type)
			case joystick::type::UNKNOWN: return "UNKNOWN";
			case joystick::type::GAMECONTROLLER: return "GAMECONTROLLER";
			case joystick::type::WHEEL: return "WHEEL";
			case joystick::type::ARCADE_STICK: return "ARCADE_STICK";
			case joystick::type::FLIGHT_STICK: return "FLIGHT_STICK";
			case joystick::type::DANCE_PAD: return "DANCE_PAD";
			case joystick::type::GUITAR: return "GUITAR";
			case joystick::type::DRUM_KIT: return "DRUM_KIT";
			case joystick::type::ARCADE_PAD: return "ARCADE_PAD";
			case joystick::type::THROTTLE: return "THROTTLE";
	END_IMPL_OUTPUT(joystick::type)


}