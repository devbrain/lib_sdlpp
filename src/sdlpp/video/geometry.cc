//
// Created by igor on 6/10/24.
//
#include <ostream>
#include <sstream>
#include "sdlpp/video/geometry.hh"

namespace neutrino::sdl {
	// std::ostream& operator << (std::ostream& os, const point& p) {
	// 	os << "(" << p.x << ", " << p.y << ")";
	// 	return os;
	// }
	// std::string to_string(const point& p) {
	// 	std::ostringstream os;
	// 	os << p;
	// 	return os.str();
	// }
	//
	// std::ostream& operator << (std::ostream& os, const point2f& p) {
	// 	os << "(" << p.x << ", " << p.y << ")";
	// 	return os;
	// }
	// std::string to_string(const point2f& p) {
	// 	std::ostringstream os;
	// 	os << p;
	// 	return os.str();
	// }

	std::ostream& operator << (std::ostream& os, const area_type& p) {
		os << "(" << p.w << ", " << p.h << ")";
		return os;
	}
	std::string to_string(const area_type& p) {
		std::ostringstream os;
		os << p;
		return os.str();
	}

	std::ostream& operator << (std::ostream& os, const rect& r) {
		os << r.offset() << "x" << r.area();
		return os;
	}
	std::string to_string(const rect& r) {
		std::ostringstream os;
		os << r;
		return os.str();
	}

}