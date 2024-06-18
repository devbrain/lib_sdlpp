//
// Created by igor on 6/18/24.
//

#include <sdlpp/system/touch.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(touch_device::type)
			case touch_device::type::DIRECT: return "DIRECT";
			case touch_device::type::INDIRECT_ABSOLUTE: return "INDIRECT_ABSOLUTE";
			case touch_device::type::INDIRECT_RELATIVE: return "INDIRECT_RELATIVE";
	END_IMPL_OUTPUT(touch_device::type)
}