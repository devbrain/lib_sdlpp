//
// Created by igor on 6/10/24.
//
#include <ostream>
#include <sstream>
#include "sdlpp/video/geometry.hh"

namespace neutrino::sdl {


	std::ostream& operator << (std::ostream& os, const rect& r) {
		os << r.offset() << "x" << r.area();
		return os;
	}
	std::string to_string(const rect& r) {
		std::ostringstream os;
		os << r;
		return os.str();
	}

	std::ostream& operator << (std::ostream& os, const rectf& r) {
		os << r.offset() << "x" << r.area();
		return os;
	}
	std::string to_string(const rectf& r) {
		std::ostringstream os;
		os << r;
		return os.str();
	}

}