//
// Created by igor on 6/10/24.
//
#include <sdlpp/system/display.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(display::orientation)
			case display::orientation::UNKNOWN: return "UNKNOWN";
			case display::orientation::LANDSCAPE: return "LANDSCAPE";
			case display::orientation::LANDSCAPE_FLIPPED: return "LANDSCAPE_FLIPPED";
			case display::orientation::PORTRAIT: return "PORTRAIT";
			case display::orientation::PORTRAIT_FLIPPED: return "PORTRAIT_FLIPPED";
	END_IMPL_OUTPUT(display::orientation)

	BEGIN_CLASS_OUTPUT(display::mode)
		os << "\tpixel format: " << t.get_pixel_format () << '\n'
		   << "\tbounds: " << t.get_bounds () << '\n';
		const auto r = t.get_refresh_rate ();
		if (r) {
			os << "\trefresh rate: " << *r << '\n';
		}
	END_CLASS_OUTPUT

	std::ostream& operator<< (std::ostream& os, const display& d) {
		os << "index " << d.get_index() << ", " << d.get_name() << " desktop bounds " << d.get_desktop_bounds();
		return os;
	}

	std::string to_string (const display& d) {
		std::ostringstream os;
		os << d;
		return os.str ();
	}
}
