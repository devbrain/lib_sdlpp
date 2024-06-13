//
// Created by igor on 6/11/24.
//
#include <sdlpp/video/cursor.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(system_cursor)
			case system_cursor::ARROW: return "ARROW";
			case system_cursor::IBEAM: return "IBEAM";
			case system_cursor::WAIT: return "WAIT";
			case system_cursor::CROSSHAIR: return "CROSSHAIR";
			case system_cursor::WAIT_ARROW: return "WAIT_ARROW";
			case system_cursor::SIZE_NW_SE: return "SIZE_NW_SE";
			case system_cursor::SIZE_NE_SW: return "SIZE_NE_SW";
			case system_cursor::SIZE_WE: return "SIZE_WE";
			case system_cursor::SIZE_NS: return "SIZE_NS";
			case system_cursor::SIZE_ALL: return "SIZE_ALL";
			case system_cursor::NO: return "NO";
			case system_cursor::HAND: return "HAND";
	END_IMPL_OUTPUT(system_cursor)
}
