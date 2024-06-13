//
// Created by igor on 6/13/24.
//
#include <sdlpp/system/joystick.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(joystick::power_level_t)
			case joystick::power_level_t::UNKNOWN: return "UNKNOWN";
			case joystick::power_level_t::EMPTY: return "EMPTY";
			case joystick::power_level_t::LOW: return "LOW";
			case joystick::power_level_t::MEDIUM: return "MEDIUM";
			case joystick::power_level_t::FULL: return "FULL";
			case joystick::power_level_t::WIRED: return "WIRED";
			case joystick::power_level_t::MAX: return "MAX";
	END_IMPL_OUTPUT(joystick::power_level_t)

	BEGIN_IMPL_OUTPUT(joystick::type_t)
			case joystick::type_t::UNKNOWN: return "UNKNOWN";
			case joystick::type_t::GAMECONTROLLER: return "GAMECONTROLLER";
			case joystick::type_t::WHEEL: return "WHEEL";
			case joystick::type_t::ARCADE_STICK: return "ARCADE_STICK";
			case joystick::type_t::FLIGHT_STICK: return "FLIGHT_STICK";
			case joystick::type_t::DANCE_PAD: return "DANCE_PAD";
			case joystick::type_t::GUITAR: return "GUITAR";
			case joystick::type_t::DRUM_KIT: return "DRUM_KIT";
			case joystick::type_t::ARCADE_PAD: return "ARCADE_PAD";
			case joystick::type_t::THROTTLE: return "THROTTLE";
	END_IMPL_OUTPUT(joystick::type_t)
}