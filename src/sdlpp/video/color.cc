//
// Created by igor on 6/10/24.
//
#include <sstream>
#include <ostream>
#include <iomanip>
#include <sdlpp/video/color.hh>

namespace neutrino::sdl {
	std::ostream& operator << (std::ostream& os, const color& p) {
		os 	<< "("
			<< ((int)p.r & 0xFF)
			<< ", "
			<< ((int)p.g & 0xFF)
			<< ", "
			<< ((int)p.b & 0xFF)
			<< ", "
			<< ((int)p.a & 0xFF)
			<< " #"
			<< std::hex << std::setfill('0') << std::setw(2) << ((int)p.r & 0xFF)
			<< std::setfill('0') << std::setw(2) << ((int)p.g & 0xFF)
			<< std::setfill('0') << std::setw(2) << ((int)p.b & 0xFF)
			<< std::dec
		<< ")";
		return os;
	}
	std::string to_string(const color& p) {
		std::ostringstream os;
		os << p;
		return os.str();
	}
}