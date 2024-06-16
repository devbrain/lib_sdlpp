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
	std::cout << "\tCapabilities: " << cpu::capabilities() << std::endl;


	std::cout << "Display Info" << std::endl;
	std::cout << "\tScreen saver enabled: " << display::screen_saver_enabled() << std::endl;
	auto vn = display::count_video_drivers();
	std::cout << "\tVideo Drivers: " << std::endl;
	for (display::driver_index_t i{0}; i < vn; i++) {
		auto d = display::video_driver (i);
		std::cout << "\t\t" << i << ") " << (d ? *d : "N/A") << std::endl;
	}
	auto cvd = display::video_driver();
	std::cout << "\tCurrent Video Driver " << (cvd ? *cvd : "N/A") << std::endl;

	auto n = display::count();
	std::cout << "\tDisplays:" << std::endl;
	for (display::index_t i{0}; i<n; i++) {
		display d(i);
		std::cout << "\t\t\t" << d << std::endl;
		for (display::mode_index_t j{0}; j< d.count_modes (); j++) {
			auto m = d.get_mode (j);
			std::cout << "MODE #" << j << std::endl
			<< m << std::endl;
		}
	}

	return 0;
}