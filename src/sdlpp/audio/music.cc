//
// Created by igor on 6/16/24.
//

#include <sdlpp/audio/music.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(music::format)
			case music::format::NONE: return "NONE";
			case music::format::CMD: return "CMD";
			case music::format::WAV: return "WAV";
			case music::format::MOD: return "MOD";
			case music::format::MID: return "MID";
			case music::format::OGG: return "OGG";
			case music::format::MP3: return "MP3";
			case music::format::MP3_MAD_UNUSED: return "MP3_MAD_UNUSED";
			case music::format::FLAC: return "FLAC";
			case music::format::MODPLUG_UNUSED: return "MODPLUG_UNUSED";
			case music::format::OPUS: return "OPUS";
			case music::format::WAVPACK: return "WAVPACK";
			case music::format::GME: return "GME";
	END_IMPL_OUTPUT(music::format)
}