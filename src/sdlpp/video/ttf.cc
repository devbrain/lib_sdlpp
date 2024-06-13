//
// Created by igor on 6/11/24.
//
#include <sdlpp/video/ttf.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	std::string to_string (ttf::style_t t) {
		static std::array<ttf::style_t, 5> mask = {
			ttf::style_t::NORMAL,
			ttf::style_t::BOLD,
			ttf::style_t::ITALIC,
			ttf::style_t::UNDERLINE
		};

		static std::array<std::string_view, 5> names = {
			ttf::style_t::NORMAL.name,
			ttf::style_t::BOLD.name,
			ttf::style_t::ITALIC.name,
			ttf::style_t::UNDERLINE.name
		};
		std::string out;
		auto v = (uint16_t)t;
		bool first = true;
		for (auto i = 0u; i < mask.size (); i++) {
			if ((v & mask[i]) == mask[i]) {
				if (first) {
					first = false;
				} else {
					out += '|';
				}
				out += names[i];
			}
		}
		return out;
	}

	std::ostream& operator<< (std::ostream& os, ttf::style_t t) {
		os << to_string (t);
		return os;
	}

	BEGIN_IMPL_OUTPUT(ttf::hinting_t)
			case ttf::hinting_t::NORMAL: return "NORMAL";
			case ttf::hinting_t::LIGHT: return "LIGHT";
			case ttf::hinting_t::MONO: return "MONO";
			case ttf::hinting_t::NONE: return "NONE";
			case ttf::hinting_t::LIGHT_SUBPIXEL: return "LIGHT_SUBPIXEL";
	END_IMPL_OUTPUT(ttf::hinting_t)

	BEGIN_IMPL_OUTPUT(ttf::alignment_t)
			case ttf::alignment_t::LEFT: return "LEFT";
			case ttf::alignment_t::CENTER: return "CENTER";
			case ttf::alignment_t::RIGHT: return "RIGHT";
	END_IMPL_OUTPUT(ttf::alignment_t)
}