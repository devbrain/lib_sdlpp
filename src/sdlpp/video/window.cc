//
// Created by igor on 6/10/24.
//

#include <sdlpp/video/window.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(window::flags)
			case window::flags::FULL_SCREEN: return "FULL_SCREEN";
			case window::flags::FULL_SCREEN_DESKTOP: return "FULL_SCREEN_DESKTOP";
			case window::flags::OPENGL: return "OPENGL";
			case window::flags::VULKAN: return "VULKAN";
			case window::flags::SHOWN: return "SHOWN";
			case window::flags::HIDDEN: return "HIDDEN";
			case window::flags::BORDERLESS: return "BORDERLESS";
			case window::flags::RESIZABLE: return "RESIZABLE";
			case window::flags::MINIMIZED: return "MINIMIZED";
			case window::flags::MAXIMIZED: return "MAXIMIZED";
			case window::flags::INPUT_GRABBED: return "INPUT_GRABBED";
			case window::flags::INPUT_FOCUS: return "INPUT_FOCUS";
			case window::flags::MOUSE_FOCUS: return "MOUSE_FOCUS";
			case window::flags::HIGHDPI: return "HIGHDPI";
			case window::flags::MOUSE_CAPTURE: return "MOUSE_CAPTURE";
			case window::flags::NONE: return "NONE";
	END_IMPL_OUTPUT(window::flags)
}