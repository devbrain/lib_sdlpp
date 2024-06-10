//
// Created by igor on 1/7/24.
//

#include <filesystem>
#include <vector>
#include <cstring>
#include "sdlpp/sdlpp.hh"

#include <sdlpp/events/events.hh>

#include "demo.hh"

demo::demo (int argc, char *argv[])
	: m_system (neutrino::sdl::init_flags::VIDEO),
	  m_window (WIDTH, HEIGHT, neutrino::sdl::window::flags::SHOWN),
	  m_render (m_window, neutrino::sdl::renderer::flags::ACCELERATED),
	  m_base_name (std::filesystem::path (argv[0]).filename ().u8string()),
	  m_vga (WIDTH * HEIGHT) {

	m_window.title (m_base_name);
	m_render.logical_size (WIDTH, HEIGHT);

}

void demo::run (int fps) {
	neutrino::sdl::surface surface (WIDTH, HEIGHT, neutrino::sdl::pixel_format (32, 0, 0, 0, 0));
	neutrino::sdl::texture texture (m_render,
									neutrino::sdl::pixel_format (neutrino::sdl::pixel_format::format::ARGB8888),
									WIDTH, HEIGHT,
									neutrino::sdl::texture::access::STREAMING);

	uint64_t now_counter = neutrino::sdl::get_performance_counter ();
	uint64_t last_counter = 0;

	bool done = false;
	while (!done) {
		last_counter = now_counter;
		now_counter = neutrino::sdl::get_performance_counter ();
		double delta_time = (double)(now_counter - last_counter) / neutrino::sdl::get_performance_frequency ();

		auto render_begin = neutrino::sdl::get_ticks ();
		// --------------------------------------------------------------------
		// Handle Input
		neutrino::sdl::handle_input (
			[&done] (neutrino::sdl::events::quit &) {
			  done = true;
			},
			[&done] (neutrino::sdl::events::keyboard &e) {
			  if (e.pressed && e.scan_code == neutrino::sdl::ESCAPE || e.scan_code == neutrino::sdl::Q) {
				  done = true;
			  }
			}
		);
		// --------------------------------------------------------------------
		// Update video
		std::memset (m_vga.data (), 0, m_vga.size ());
		int i = 0;
		for (const auto color : m_pal) {
			m_color_table[i++] = surface.map_color (color);
		}
		draw (m_vga.data(), delta_time);

		auto [pixels, pitch] = texture.lock();
		auto* pixels_ptr = reinterpret_cast<uint32_t*>(pixels);
		for (std::size_t i=0; i<WIDTH*HEIGHT; i++) {
			pixels_ptr[i] = m_color_table[m_vga[i]];
		}
		texture.unlock();
		m_render.clear();
		m_render.copy (texture);
		m_render.present();


		auto render_end = neutrino::sdl::get_ticks ();

		if (fps > 0 && ((render_end - render_begin) < (Uint32)1000 / fps)) {
			neutrino::sdl::delay ((1000 / fps) - (render_end - render_begin));
		}

	}
}

void demo::color (unsigned char color, unsigned char r, unsigned char g, unsigned char b) {
	m_pal[color].r = r;
	m_pal[color].g = g;
	m_pal[color].b = b;
}

void demo::pixel(unsigned char* video_mem, int x, int y, unsigned char color) {
	video_mem[WIDTH*y +x] = color;
}

unsigned char demo::pixel(const unsigned char* video_mem, int x, int y) {
	return video_mem[WIDTH*y + x];
}