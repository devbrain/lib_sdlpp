//
// Created by igor on 6/6/24.
//
#include <iostream>
#include "sdlpp/sdlpp.hh"


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
	using namespace neutrino::sdl;
	neutrino::sdl::system initializer (init_flags::VIDEO);

	std::cout << "CPU INFO" << std::endl;
	std::cout << "\tCPU count " << cpu::count () << std::endl;
	std::cout << "\tCache line " << cpu::cache_line () << " bytes" << std::endl;
	std::cout << "\tRAM " << neutrino::sdl::system::ram_in_mb() << " MB" << std::endl;

	std::cout << "\tCapabilities: ";
	auto c = cpu::capabilities();

#define d_evalCap(NAME) 														\
		if (c & cpu::capability_t::PPCAT(HAS_, NAME)) { 						\
	 		std::cout << " " << cpu::capability_t::PPCAT(HAS_, NAME).name;      \
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

	std::cout << std::endl;

	std::cout << "Display Info" << std::endl;
	std::cout << "\tScreen saver enabled: " << display::screen_saver_enabled() << std::endl;
	auto vn = display::count_video_drivers();
	std::cout << "\tVideo Drivers: " << std::endl;
	for (std::size_t i=0; i<vn; i++) {
		auto d = display::video_driver (i);
		std::cout << "\t\t" << i << ") " << (d ? *d : "N/A") << std::endl;
	}
	auto cvd = display::video_driver();
	std::cout << "\tCurrent Video Driver " << (cvd ? *cvd : "N/A") << std::endl;

	auto n = display::count();
	std::cout << "\tDisplays:" << std::endl;
	for (std::size_t i = 0; i<n; i++) {
		std::cout << "\t\tDisplay " << i << std::endl;
		display d(i);
		std::cout << "\t\t\tName " << d.get_name() << std::endl;
	}

	return 0;
}