//
// Created by igor on 1/4/24.
//

#ifndef DEMO_DEMO_HH
#define DEMO_DEMO_HH

#include <cstdlib>
#include <string>
#include <vector>

#include <sdlpp/window.hh>
#include <sdlpp/system.hh>
#include <sdlpp/surface.hh>
#include <sdlpp/texture.hh>
#include <sdlpp/render.hh>

#include <vga_demo_export.h>

constexpr inline int WIDTH = 320;
constexpr inline int HEIGHT = 240;
constexpr inline int MAX_TEXTURES = 240;

template <typename T>
static inline auto rad2deg (T rad) -> T {
	return rad * static_cast<T>(M_PI / 180.0);
}

static inline int random (int n) {
	const float p = (static_cast<float>(std::rand ()) / (float)RAND_MAX);
	return static_cast<int> (p * static_cast<float>(n));
}

static inline int clamp (int n, int l, int h) {
	return (n < l ? l : (n > (h - 1) ? (h - 1) : n));
}

static inline int clamp128 (int n) {
	return clamp (n, 0, 128);
}

static inline int clamp256 (int n) {
	return clamp (n, 0, 256);
}

static inline int clamp_width (int n) {
	return clamp (n, 0, WIDTH);
}

static inline int clamp_height (int n) {
	return clamp (n, 0, WIDTH);
}

class VGA_DEMO_EXPORT demo {
 public:
  demo (int argc, char *argv[]);
  virtual ~demo () = default;

  void run (int fps = 60);
 protected:
  void color (unsigned char color, unsigned char r, unsigned char g, unsigned char b);
  virtual void draw(unsigned char* video_mem, double delta_time) = 0;

  static void pixel(unsigned char* video_mem, int x, int y, unsigned char color);
  static void pixel(unsigned char* video_mem, const neutrino::sdl::point& p, unsigned char color) {
	  pixel (video_mem, p.x, p.y, color);
  }
  static unsigned char pixel(const unsigned char* video_mem, int x, int y);
  static unsigned char pixel(const unsigned char* video_mem, const neutrino::sdl::point& p) {
	  return pixel (video_mem, p.x, p.y);
  }
 private:
  std::vector<unsigned char> m_vga;
  neutrino::sdl::color m_pal[256];
  uint32_t m_color_table[256];
 private:
  neutrino::sdl::system m_system;
  neutrino::sdl::window m_window;
  neutrino::sdl::renderer m_render;
  std::string m_base_name;
};

#endif //DEMO_DEMO_HH
