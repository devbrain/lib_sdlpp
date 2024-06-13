//
// Created by igor on 6/13/24.
//
#include <sdlpp/system/sensors.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(sensor::type_t)
			case sensor::type_t::INVALID: return "INVALID";
			case sensor::type_t::UNKNOWN: return "UNKNOWN";
			case sensor::type_t::ACCEL: return "ACCEL";
			case sensor::type_t::GYRO: return "GYRO";
			case sensor::type_t::ACCEL_L: return "ACCEL_L";
			case sensor::type_t::GYRO_L: return "GYRO_L";
			case sensor::type_t::ACCEL_R: return "ACCEL_R";
			case sensor::type_t::GYRO_R: return "GYRO_R";
	END_IMPL_OUTPUT(sensor::type_t)
}
