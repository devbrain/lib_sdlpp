//
// Created by igor on 6/6/24.
//
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>

#include <sdlpp/system.hh>
#include <sdlpp/window.hh>
#include <sdlpp/render.hh>
#include <sdlpp/io.hh>
#include <sdlpp/ttf.hh>
#include <sdlpp/events/events.hh>


int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "USAGE: " << argv[0] << " <path to ttf file" << std::endl;
		return 1;
	}




	std::ifstream ifs(argv[1], std::ios::in | std::ios::binary);
	if (!ifs) {
		std::cerr << "Can not open file " << argv[1] << std::endl;
		return 1;
	}

	try {

		using namespace neutrino;
		sdl::system initializer (sdl::init_flags::VIDEO);

		SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

		sdl::rw_istream is(ifs);
		sdl::ttf font(is, 28);


		auto image = font.render_blended ("Hello World", {255, 0,0});


		sdl::window window(640, 480, sdl::window::flags_t::SHOWN);
		sdl::renderer renderer(window, sdl::renderer::flags::ACCELERATED);

		sdl::texture texture(renderer, image);

		bool done = false;

		while (!done) {
			// Handle Input
			neutrino::sdl::handle_input (
				[&done] (neutrino::sdl::events::quit &) {
				  done = true;
				},
				[&done] (neutrino::sdl::events::keyboard &e) {
				  if (e.pressed) {
					  done = true;
				  }
				}
			);

			renderer.clear();
			renderer.copy (texture);
			renderer.present();
		}

	} catch (std::exception& e) {
		std::cerr << "Error occured : " << e.what() << std::endl;
	}
	return 0;
}

