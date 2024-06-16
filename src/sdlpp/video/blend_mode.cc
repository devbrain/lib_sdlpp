//
// Created by igor on 6/16/24.
//
#include <sdlpp/video/blend_mode.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(blend_mode)
			case blend_mode::NONE: return "NONE";
			case blend_mode::BLEND: return "BLEND";
			case blend_mode::ADD: return "ADD";
			case blend_mode::MOD: return "MOD";
	END_IMPL_OUTPUT(blend_mode)
}