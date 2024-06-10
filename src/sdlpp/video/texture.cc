//
// Created by igor on 6/10/24.
//
#include <sdlpp/video/texture.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(texture::access)
			case texture::access::STATIC: return "STATIC";
			case texture::access::STREAMING: return "STREAMING";
			case texture::access::TARGET: return "TARGET";
	END_IMPL_OUTPUT(texture::access)
}