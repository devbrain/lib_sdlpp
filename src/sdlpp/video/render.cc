//
// Created by igor on 6/6/24.
//

#include <vector>
#include <sdlpp/video/render.hh>
#include "thirdparty/gfx/SDL2_gfxPrimitives.h"

namespace neutrino::sdl {
	void renderer::draw_hline (int x1, int x2, int y) {
		const auto c = active_color ();
		hlineRGBA (handle (),
				   static_cast<Sint16>(x1),
				   static_cast<Sint16>(x2),
				   static_cast<Sint16>(y),
				   c.r, c.g, c.b, c.a);
	}

	void renderer::draw_vline (int y1, int y2, int x) {
		const auto c = active_color ();
		vlineRGBA (handle (),
				   static_cast<Sint16>(y1),
				   static_cast<Sint16>(y2),
				   static_cast<Sint16>(x),
				   c.r, c.g, c.b, c.a);
	}

	void renderer::draw_line_aa (int x1, int y1, int x2, int y2) {
		const auto c = active_color ();
		aalineRGBA (handle (),
					static_cast<Sint16>(x1),
					static_cast<Sint16>(y1),
					static_cast<Sint16>(x2),
					static_cast<Sint16>(y2),
					c.r, c.g, c.b, c.a);
	}

	void renderer::draw_rounded_rect (const rect& rec, unsigned radius) {
		const auto c = active_color ();
		const auto x1 = rec.x;
		const auto y1 = rec.y;
		const auto x2 = rec.x + rec.w;
		const auto y2 = rec.y + rec.h;

		roundedRectangleRGBA (handle (),
							  static_cast<Sint16>(x1),
							  static_cast<Sint16>(y1),
							  static_cast<Sint16>(x2),
							  static_cast<Sint16>(y2),
							  static_cast<Sint16 >(radius),
							  c.r, c.g, c.b, c.a);
	}

	void renderer::draw_rounded_rect_filled (const rect& rec, unsigned radius) {
		const auto c = active_color ();
		const auto x1 = rec.x;
		const auto y1 = rec.y;
		const auto x2 = rec.x + rec.w;
		const auto y2 = rec.y + rec.h;

		roundedBoxRGBA (handle (),
						static_cast<Sint16>(x1),
						static_cast<Sint16>(y1),
						static_cast<Sint16>(x2),
						static_cast<Sint16>(y2),
						static_cast<Sint16 >(radius),
						c.r, c.g, c.b, c.a);
	}

	void renderer::draw_thick_line (int x1, int y1, int x2, int y2, unsigned width) {
		const auto c = active_color ();
		thickLineRGBA (handle (),
					   static_cast<Sint16>(x1),
					   static_cast<Sint16>(y1),
					   static_cast<Sint16>(x2),
					   static_cast<Sint16>(y2),
					   static_cast<Uint8 >(width),
					   c.r, c.g, c.b, c.a);
	}

	void renderer::draw_circle (int x, int y, unsigned radius) {
		const auto c = active_color ();
		circleRGBA (handle (),
					static_cast<Sint16>(x),
					static_cast<Sint16>(y),
					static_cast<Sint16>(radius),
					c.r, c.g, c.b, c.a);
	}

	void renderer::draw_circle_aa (int x, int y, unsigned radius) {
		const auto c = active_color ();
		aacircleRGBA (handle (),
					  static_cast<Sint16>(x),
					  static_cast<Sint16>(y),
					  static_cast<Sint16>(radius),
					  c.r, c.g, c.b, c.a);
	}

	void renderer::draw_circle_filled (int x, int y, unsigned radius) {
		const auto c = active_color ();
		filledCircleRGBA (handle (),
						  static_cast<Sint16>(x),
						  static_cast<Sint16>(y),
						  static_cast<Sint16>(radius),
						  c.r, c.g, c.b, c.a);
	}

	void renderer::draw_arc (int x, int y, int start, int end, unsigned radius) {
		const auto c = active_color ();
		arcRGBA (handle (),
				 static_cast<Sint16>(x),
				 static_cast<Sint16>(y),
				 static_cast<Sint16>(radius),
				 static_cast<Sint16>(start),
				 static_cast<Sint16>(end),
				 c.r, c.g, c.b, c.a
		);
	}

	void renderer::draw_arc_filled (int x, int y, int start, int end, unsigned radius) {
		const auto c = active_color ();
		pieRGBA (handle (),
				 static_cast<Sint16>(x),
				 static_cast<Sint16>(y),
				 static_cast<Sint16>(radius),
				 static_cast<Sint16>(start),
				 static_cast<Sint16>(end),
				 c.r, c.g, c.b, c.a
		);
	}

	void renderer::draw_ellipse (int x, int y, unsigned rx, unsigned ry) {
		const auto c = active_color ();
		ellipseRGBA (handle (),
					 static_cast<Sint16>(x),
					 static_cast<Sint16>(y),
					 static_cast<Sint16>(rx),
					 static_cast<Sint16>(ry),
					 c.r, c.g, c.b, c.a
		);
	}

	void renderer::draw_ellipse_aa (int x, int y, unsigned rx, unsigned ry) {
		const auto c = active_color ();
		aaellipseRGBA (handle (),
					   static_cast<Sint16>(x),
					   static_cast<Sint16>(y),
					   static_cast<Sint16>(rx),
					   static_cast<Sint16>(ry),
					   c.r, c.g, c.b, c.a
		);
	}

	void renderer::draw_ellipse_filled (int x, int y, unsigned rx, unsigned ry) {
		const auto c = active_color ();
		filledEllipseRGBA (handle (),
						   static_cast<Sint16>(x),
						   static_cast<Sint16>(y),
						   static_cast<Sint16>(rx),
						   static_cast<Sint16>(ry),
						   c.r, c.g, c.b, c.a
		);
	}

	void renderer::draw_triangle (int x1, int y1, int x2, int y2, int x3, int y3) {
		const auto c = active_color ();
		trigonRGBA (handle (),
					static_cast<Sint16>(x1),
					static_cast<Sint16>(y1),
					static_cast<Sint16>(x2),
					static_cast<Sint16>(y2),
					static_cast<Sint16>(x3),
					static_cast<Sint16>(y3),
					c.r, c.g, c.b, c.a
		);
	}

	void renderer::draw_triangle_aa (int x1, int y1, int x2, int y2, int x3, int y3) {
		const auto c = active_color ();
		aatrigonRGBA (handle (),
					  static_cast<Sint16>(x1),
					  static_cast<Sint16>(y1),
					  static_cast<Sint16>(x2),
					  static_cast<Sint16>(y2),
					  static_cast<Sint16>(x3),
					  static_cast<Sint16>(y3),
					  c.r, c.g, c.b, c.a
		);
	}

	void renderer::draw_triangle_filled (int x1, int y1, int x2, int y2, int x3, int y3) {
		const auto c = active_color ();
		filledTrigonRGBA (handle (),
						  static_cast<Sint16>(x1),
						  static_cast<Sint16>(y1),
						  static_cast<Sint16>(x2),
						  static_cast<Sint16>(y2),
						  static_cast<Sint16>(x3),
						  static_cast<Sint16>(y3),
						  c.r, c.g, c.b, c.a
		);
	}

	void renderer::draw_polygon (const bsw::array_view1d<point>& points) {
		std::vector<Sint16> vx (points.size ());
		std::vector<Sint16> vy (points.size ());
		for (const auto& p : points) {
			vx.emplace_back (static_cast<Sint16>(p.x));
			vy.emplace_back (static_cast<Sint16>(p.x));
		}
		const auto c = active_color ();
		polygonRGBA (handle (),
					 vx.data (),
					 vy.data (),
					 static_cast<int>(points.size ()),
					 c.r, c.g, c.b, c.a);
	}

	void renderer::draw_polygon_aa (const bsw::array_view1d<point>& points) {
		std::vector<Sint16> vx (points.size ());
		std::vector<Sint16> vy (points.size ());
		for (const auto& p : points) {
			vx.emplace_back (static_cast<Sint16>(p.x));
			vy.emplace_back (static_cast<Sint16>(p.x));
		}
		const auto c = active_color ();
		aapolygonRGBA (handle (),
					 vx.data (),
					 vy.data (),
					 static_cast<int>(points.size ()),
					 c.r, c.g, c.b, c.a);
	}

	void renderer::draw_polygon (const bsw::array_view1d<point>& points, const object<SDL_Surface>& tex,
								 int texture_dx, int texture_dy) {
		std::vector<Sint16> vx (points.size ());
		std::vector<Sint16> vy (points.size ());
		for (const auto& p : points) {
			vx.emplace_back (static_cast<Sint16>(p.x));
			vy.emplace_back (static_cast<Sint16>(p.x));
		}
		texturedPolygon (handle (),
						 vx.data (),
						 vy.data (),
						 static_cast<int>(points.size ()),
						 const_cast<SDL_Surface*>(tex.handle ()),
						 static_cast<Sint16>(texture_dx),
						 static_cast<Sint16>(texture_dy)
		);
	}

	void renderer::draw_polygon_filled (const bsw::array_view1d<point>& points) {
		std::vector<Sint16> vx (points.size ());
		std::vector<Sint16> vy (points.size ());
		for (const auto& p : points) {
			vx.emplace_back (static_cast<Sint16>(p.x));
			vy.emplace_back (static_cast<Sint16>(p.x));
		}
		const auto c = active_color ();
		filledPolygonRGBA (handle (),
						   vx.data (),
						   vy.data (),
						   static_cast<int>(points.size ()),
						   c.r, c.g, c.b, c.a);
	}

	void renderer::draw_polygon (const int16_t* vx, const int16_t* vy, std::size_t n) {
		const auto c = active_color ();
		polygonRGBA (handle (),
					 vx,
					 vy,
					 static_cast<int>(n),
					 c.r, c.g, c.b, c.a);
	}

	void renderer::draw_polygon_aa (const int16_t* vx, const int16_t* vy, std::size_t n) {
		const auto c = active_color ();
		aapolygonRGBA (handle (),
					 vx,
					 vy,
					 static_cast<int>(n),
					 c.r, c.g, c.b, c.a);
	}

	void renderer::draw_polygon_filled (const int16_t* vx, const int16_t* vy, std::size_t n) {
		const auto c = active_color ();
		filledPolygonRGBA (handle (),
						   vx,
						   vy,
						   static_cast<int>(n),
						   c.r, c.g, c.b, c.a);
	}

	void renderer::draw_polygon (const int16_t* vx, const int16_t* vy, std::size_t n, const object<SDL_Surface>& tex,
								 int texture_dx, int texture_dy) {
		texturedPolygon (handle (),
						 vx,
						 vy,
						 static_cast<int>(n),
						 const_cast<SDL_Surface*>(tex.handle ()),
						 static_cast<Sint16>(texture_dx),
						 static_cast<Sint16>(texture_dy)
		);
	}

	void renderer::draw_bezier (const bsw::array_view1d<point>& points, unsigned steps) {
		std::vector<Sint16> vx (points.size ());
		std::vector<Sint16> vy (points.size ());
		for (const auto& p : points) {
			vx.emplace_back (static_cast<Sint16>(p.x));
			vy.emplace_back (static_cast<Sint16>(p.x));
		}
		const auto c = active_color ();
		bezierRGBA (handle (),
					vx.data (),
					vy.data (),
					static_cast<int>(points.size ()),
					static_cast<int>(steps),
					c.r, c.g, c.b, c.a);
	}

	void renderer::draw_bezier (const int16_t* vx, const int16_t* vy, std::size_t n, unsigned steps) {
		const auto c = active_color ();
		bezierRGBA (handle (),
					vx,
					vy,
					static_cast<int>(n),
					static_cast<int>(steps),
					c.r, c.g, c.b, c.a);
	}

	void renderer::draw_latin1_string (int x, int y, const std::string& s) {
		const auto c = active_color ();
		stringRGBA (handle (),
					static_cast<Sint16>(x),
					static_cast<Sint16>(y),
					s.c_str (),
					c.r, c.g, c.b, c.a);
	}
}
