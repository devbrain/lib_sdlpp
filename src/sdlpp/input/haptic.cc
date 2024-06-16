//
// Created by igor on 6/16/24.
//
#include <sdlpp/input/haptic.hh>

namespace neutrino::sdl {
#define d_FLAG(F)								\
    if (f.contains (haptic::features:: F)) {	\
        if (first) {   							\
            first = false;         				\
        } else {       							\
            os << "|";         					\
        }										\
        os << haptic::features:: F.name; }

	std::ostream& operator<< (std::ostream& os, const haptic::features f) {
		bool first = true;

		d_FLAG(CONSTANT_EFFECT)
		d_FLAG(SINE)
		d_FLAG(LEFT_RIGHT)
		d_FLAG(TRIANGLE)
		d_FLAG(SAWTOOTHUP)
		d_FLAG(SAWTOOTHDOWN)
		d_FLAG(RAMP)
		d_FLAG(SPRING)
		d_FLAG(DUMPER)
		d_FLAG(INERTIA)
		d_FLAG(FRICTION)
		d_FLAG(CUSTOM)
		d_FLAG(GAIN)
		d_FLAG(AUTOCENTER)
		d_FLAG(STATUS)
		d_FLAG(PAUSE)

		return os;
	}

	std::string to_string (const haptic::features d) {
		std::ostringstream os;
		os << d;
		return os.str ();
	}
}