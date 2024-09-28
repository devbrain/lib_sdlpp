//
// Created by igor on 6/6/24.
//

#include <sdlpp/video/surface.hh>
#include "thirdparty/gfx/SDL2_rotozoom.h"
#include <bsw/exception.hh>

namespace neutrino::sdl {

	surface surface::roto_zoom (double angle, double zoom, bool smooth) const {
		SDL_Surface* rotated = rotozoomSurface (const_cast<SDL_Surface*>(handle ()), angle, zoom,
												smooth ? SMOOTHING_ON : SMOOTHING_OFF);
		ENFORCE(rotated != nullptr);
		return surface (object<SDL_Surface> (rotated, true));
	}

	surface surface::roto_zoom (double angle, double zoom_x, double zoom_y, bool smooth) const {
		SDL_Surface* rotated = rotozoomSurfaceXY (const_cast<SDL_Surface*>(handle ()), angle, zoom_x, zoom_y,
												  smooth ? SMOOTHING_ON : SMOOTHING_OFF);
		ENFORCE(rotated != nullptr);
		return surface (object<SDL_Surface> (rotated, true));
	}

	area_type surface::roto_zoom_size (double angle, double zoom) const {
		auto [_, pitch, w, h] = pixels_data ();
		int dw, dh;
		rotozoomSurfaceSize (
			static_cast<int>(w),
			static_cast<int>(h),
			angle,
			zoom,
			&dw,
			&dh
		);
		return {static_cast<unsigned int>(dw), static_cast<unsigned int>(dh)};
	}

	area_type surface::roto_zoom_size (double angle, double zoom_x, double zoom_y) const {
		auto [_, pitch, w, h] = pixels_data ();
		int dw, dh;
		rotozoomSurfaceSizeXY (
			static_cast<int>(w),
			static_cast<int>(h),
			angle,
			zoom_x,
			zoom_y,
			&dw,
			&dh
		);
		return {static_cast<unsigned int>(dw), static_cast<unsigned int>(dh)};
	}
}