//
// Created by igor on 1/10/24.
//

#include "demo.hh"
#include <iostream>

class plasma : public demo {
 public:
  plasma(int argc, char* argv[])
  : demo (argc, argv) {
	for (int i = 0; i < SINE_VALUES; i++) {
		sin_table[i] = (float)cos(i * M_PI / 180);
	}

	  unsigned char r = 0, g = 0, b = 0;
	  for (int i = 0; i < 256; i++) {
		  color(i, 0, 0, 0);
	  }
	  for (int i = 0; i < 42; i++, r++) {
		  color(i, r * 4, g * 4, b * 4);
	  }
	  for (int i = 42; i < 84; i++, g++) {
		  color(i, r * 4, g * 4, b * 4);
	  }
	  for (int i = 84; i < 126; i++, b++) {
		  color(i, r * 4, g * 4, b * 4);
	  }
	  for (int i = 126; i < 168; i++, r--) {
		  color(i, r * 4, g * 4, b * 4);
	  }
	  for (int i = 168; i < 210; i++, g--) {
		  color(i, r * 4, g * 4, b * 4);
	  }
	  for (int i = 210; i < 252; i++, b--) {
		  color(i, r * 4, g * 4, b * 4);
	  }
  }
 private:
  void draw(unsigned char* video_mem, double delta_time) override {
	  static double frame_counter = 0;
	  frame_counter += delta_time * 100;
	  int frame = (int)frame_counter % FRAMES;

	  // Draw plasma
	  for (int y = 0; y < HEIGHT; y++) {
		  float yc = 75 + sin_table[y + frame * 2] * 2 + sin_table[y * 2 + frame / 2] + sin_table[y + frame] * 2;

		  for (int x = 0; x < WIDTH; x++) {
			  float xc = 75 + sin_table[x * 2 + frame / 2] + sin_table[x + frame * 2] + sin_table[x / 2 + frame] * 2;

			  unsigned char color = xc * yc;
			  pixel(video_mem, x, y, color);
		  }
	  }
  }
 private:
  static constexpr int FRAMES = 720;
  static constexpr int SINE_VALUES = WIDTH + FRAMES*2;
 private:
  float sin_table[SINE_VALUES];
};

int main (int argc, char *argv[]) {
	try {
		plasma vga_demo (argc, argv);
		vga_demo.run ();
		return 0;
	} catch (std::exception &e) {
		std::cout << "Error: " << e.what () << std::endl;
	}
	return 1;
}