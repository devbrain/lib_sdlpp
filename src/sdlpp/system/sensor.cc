//
// Created by igor on 6/13/24.
//
#include <sdlpp/system/sensors.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(sensor::type)
			case sensor::type::UNKNOWN: return "UNKNOWN";
			case sensor::type::ACCEL: return "ACCEL";
			case sensor::type::GYRO: return "GYRO";
			case sensor::type::ACCEL_L: return "ACCEL_L";
			case sensor::type::GYRO_L: return "GYRO_L";
			case sensor::type::ACCEL_R: return "ACCEL_R";
			case sensor::type::GYRO_R: return "GYRO_R";
	END_IMPL_OUTPUT(sensor::type)
}
