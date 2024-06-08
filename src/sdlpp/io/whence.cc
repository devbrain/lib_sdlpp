//
// Created by igor on 6/8/24.
//
#include <sdlpp/io/whence.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(whence)
		case whence::SET: return "SET";
		case whence::CUR: return "CUR";
		case whence::END: return "END";
	END_IMPL_OUTPUT(whence)
}