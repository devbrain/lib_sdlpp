//
// Created by igor on 6/10/24.
//

#include <sdlpp/video/window.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(window::flags_t)
			case window::flags_t::FULL_SCREEN: return "FULL_SCREEN";
			case window::flags_t::FULL_SCREEN_DESKTOP: return "FULL_SCREEN_DESKTOP";
			case window::flags_t::OPENGL: return "OPENGL";
			case window::flags_t::VULKAN: return "VULKAN";
			case window::flags_t::SHOWN: return "SHOWN";
			case window::flags_t::HIDDEN: return "HIDDEN";
			case window::flags_t::BORDERLESS: return "BORDERLESS";
			case window::flags_t::RESIZABLE: return "RESIZABLE";
			case window::flags_t::MINIMIZED: return "MINIMIZED";
			case window::flags_t::MAXIMIZED: return "MAXIMIZED";
			case window::flags_t::INPUT_GRABBED: return "INPUT_GRABBED";
			case window::flags_t::INPUT_FOCUS: return "INPUT_FOCUS";
			case window::flags_t::MOUSE_FOCUS: return "MOUSE_FOCUS";
			case window::flags_t::HIGHDPI: return "HIGHDPI";
			case window::flags_t::MOUSE_CAPTURE: return "MOUSE_CAPTURE";
			case window::flags_t::NONE: return "NONE";
	END_IMPL_OUTPUT(window::flags_t)
}