//
// Created by igor on 6/11/24.
//

#include <sdlpp/system/power.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(power_state)
			case power_state::UNKNOWN: return "UNKNOWN";
			case power_state::ON_BATTERY: return "ON_BATTERY";
			case power_state::NO_BATTERY: return "NO_BATTERY";
			case power_state::CHARGING: return "CHARGING";
			case power_state::CHARGED: return "CHARGED";
	END_IMPL_OUTPUT(power_state)
}
