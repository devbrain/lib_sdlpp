//
// Created by igor on 1/4/24.
//

#include <stdexcept>
#include <iostream>
#include "demo.hh"

class blob : public demo {
  static constexpr int NUM_BLOBS = 160;
  static constexpr int BLOB_RADIUS = 20;
  static constexpr int BLOB_DRADIUS = (BLOB_RADIUS * 2);
  static constexpr int BLOB_SRADIUS = (BLOB_RADIUS * BLOB_RADIUS);
 public:
  blob(int argc, char *argv[])
  : demo (argc, argv) {
	  for (int i = 0; i < 256; i++) {
		  color(i, i / 2.5, i / 1.5, i);
	  }

	  for (auto & Blob : m_coords) {
		  Blob.x = (WIDTH / 2) - BLOB_RADIUS;
		  Blob.y = (HEIGHT / 2) - BLOB_RADIUS;
	  }

	  for (int y = 0; y < BLOB_DRADIUS; y++) {
		  for (int x = 0; x < BLOB_DRADIUS; x++) {
			  float distance = (y - BLOB_RADIUS) * (y - BLOB_RADIUS) + (x - BLOB_RADIUS) * (x - BLOB_RADIUS);

			  if (distance <= BLOB_SRADIUS) {
				  float fraction = distance / BLOB_SRADIUS;
				  m_blob[y * BLOB_DRADIUS + x] = (unsigned char) (pow((0.7 - (fraction * fraction)), 3.3) * 255.0);
			  } else {
				  m_blob[y * BLOB_DRADIUS + x] = 0;
			  }
		  }
	  }
  }
 private:

  void draw(unsigned char* video_mem, double delta_time) override {
	  for (auto & m_coord : m_coords) {
		  m_coord.x += -2 + (5.0 * (rand() / (RAND_MAX + 2.0)));
		  m_coord.y += -2 + (5.0 * (rand() / (RAND_MAX + 2.0)));
	  }

	  // Draw blobs
	  for (int i = 0; i < NUM_BLOBS; i++) {
		  if (m_coords[i].x > 0 && m_coords[i].x < WIDTH - BLOB_DRADIUS && m_coords[i].y > 0 && m_coords[i].y < HEIGHT - BLOB_DRADIUS) {
			  for (int y = 0; y < BLOB_DRADIUS; y++) {
				  for (int x = 0; x < BLOB_DRADIUS; x++) {
					  unsigned char color = clamp256(pixel(video_mem, m_coords[i].x + x, m_coords[i].y + y) + m_blob[y * BLOB_DRADIUS + x]);
					  pixel(video_mem, m_coords[i].x + x, m_coords[i].y + y, color);
				  }
			  }
		  } else {
			  m_coords[i].x = (WIDTH / 2) - BLOB_RADIUS;
			  m_coords[i].y = (HEIGHT / 2) - BLOB_RADIUS;
		  }
	  }
  }
 private:
  unsigned char m_blob [BLOB_DRADIUS * BLOB_DRADIUS];
  neutrino::sdl::point m_coords[NUM_BLOBS];
};

int main (int argc, char *argv[]) {
	try {
		blob vga_demo (argc, argv);
		vga_demo.run ();
	} catch (std::exception &e) {
		std::cout << "Error: " << e.what () << std::endl;
	}
}