//
// Created by igor on 6/8/24.
//
#include <ostream>
#include <sstream>
#include <sdlpp/system/cpu.hh>

namespace neutrino::sdl {

	std::string to_string(cpu::capability t) {
		std::ostringstream s;
		s << t;
		return s.str();
	}

	std::ostream & operator << (std::ostream &os, cpu::capability t) {
		bool first = true;

#define d_evalCap(NAME) 														\
		if (t & cpu::capability::PPCAT(HAS_, NAME)) { 							\
            if (first) {              											\
            	first = false;                          						\
			} else {                           									\
            	os << "|";                          							\
			}                          											\
	 		os << cpu::capability::PPCAT(HAS_, NAME).name;      				\
		}
		d_evalCap(3DNow)
		d_evalCap(AltiVec)
		d_evalCap(ARMSIMD)
		d_evalCap(AVX)
		d_evalCap(AVX2)
		d_evalCap(AVX512F)
		d_evalCap(LASX)
		d_evalCap(LSX)
		d_evalCap(MMX)
		d_evalCap(NEON)
		d_evalCap(RDTSC)
		d_evalCap(SSE)
		d_evalCap(SSE2)
		d_evalCap(SSE3)
		d_evalCap(SSE41)
		d_evalCap(SSE42)

		return os;
	}
}