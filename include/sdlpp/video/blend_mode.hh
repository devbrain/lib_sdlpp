//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_VIDEO_BLEND_MODE_HH_
#define SDLPP_INCLUDE_SDLPP_VIDEO_BLEND_MODE_HH_

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	enum class blend_mode : uint32_t {
		NONE = SDL_BLENDMODE_NONE,
		BLEND = SDL_BLENDMODE_BLEND,
		ADD = SDL_BLENDMODE_ADD,
		MOD = SDL_BLENDMODE_MOD
	};

	d_SDLPP_OSTREAM(blend_mode);
}

#endif //SDLPP_INCLUDE_SDLPP_VIDEO_BLEND_MODE_HH_
