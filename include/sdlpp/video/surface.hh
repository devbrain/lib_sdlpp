//
// Created by igor on 03/06/2020.
//

#ifndef NEUTRINO_SDL_SURFACE_HH
#define NEUTRINO_SDL_SURFACE_HH

#include <string>
#include <optional>

#include <bsw/array_view.hh>
#include <bsw/macros.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/lock.hh>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/io/io.hh>
#include <sdlpp/video/geometry.hh>
#include <sdlpp/video/palette.hh>
#include <sdlpp/video/pixel_format.hh>
#include <sdlpp/video/blend_mode.hh>

namespace neutrino::sdl {


	/**
	 * @class surface
	 * @brief A class representing a surface in SDL.
	 *
	 * This class is used for representing a collection of pixels used in software blitting.
	 * It provides functionality for manipulating and blitting surfaces, as well as changing properties
	 * like color, transparency, and blending.
	 */
	class SDLPP_EXPORT surface : public object<SDL_Surface> {
	 public:
		surface ();
		explicit surface (object<SDL_Surface>&& other);
		surface (surface&& other) noexcept;
		explicit surface (const object<SDL_Window>& other);
		surface& operator= (object<SDL_Surface>&& other) noexcept;
		surface& operator= (surface&& other) noexcept;

		surface& operator= (const surface& other) = delete;
		surface (const surface& other) = delete;

		surface (unsigned width, unsigned height, pixel_format format);
		// pitch - size of the scanline in bytes
		surface (void* data, unsigned width, unsigned height, unsigned pitch, pixel_format format);

		explicit surface (object<SDL_RWops>& rwops);

		[[nodiscard]] static surface make_8bit (unsigned width, unsigned height);
		[[nodiscard]] static surface make_rgba_32bit (unsigned width, unsigned height);

		void save_bmp (const std::string& path) const;
		void save_bmp (object<SDL_RWops>& stream) const;

		[[nodiscard]] surface clone () const;
		void lock () noexcept;
		void unlock () noexcept;
		[[nodiscard]] bool must_lock () const noexcept;

		[[nodiscard]] rect get_clip () const;
		/**
		 * @brief set clip rectangle
		 * @param r
		 * @return Returns true if the rectangle intersects the surface, otherwise false and blits will be completely clipped.
		 */
		[[nodiscard]] bool set_clip (const rect& r);

		void color_key (uint32_t c);
		void color_key (const color& c);
		void disable_color_key ();
		[[nodiscard]] std::optional<uint32_t> color_key () const;

		[[nodiscard]] blend_mode get_blend_mode () const;
		void set_blend_mode (blend_mode v);

		[[nodiscard]] uint8_t get_alpha_mod () const;
		void set_alpha_mod (uint8_t v);

		void set_rle_acceleration (bool enabled);

		void set_color_mod (uint8_t r, uint8_t g, uint8_t b);
		[[nodiscard]] std::optional<color> get_color_mod () const;

		/**
		 * @brief Blit this surface to @c dst surface
		 *
		 * This function blits an area enclosed by @c srect to the destination surface area @c drect.
		 * Scaling will be performed if needed.
		 *
		 * @param srect source area
		 * @param dst  destination surface
		 * @param drect destination area
		 * @throws @see sdl::exception on failure
		 */
		void blit (const rect& srect, object<SDL_Surface>& dst, const rect& drect) const;

		/**
		 * @brief Blit this surface to @c dst surface
		 *
		 * This function blits the whole surface to the destination surface @c dst.
		 * Scaling will be performed if needed.
		 *
		 * @param dst destination surface
		 * @throws @see sdl::exception on failure
		 */
		void blit_scaled (const rect& srect, object<SDL_Surface>& dst) const;
		void blit_scaled (object<SDL_Surface>& dst) const;
		void blit_scaled (const rect& srect, object<SDL_Surface>& dst, const rect& drect) const;

		/**
		 * @brief Blit this surface to @c dst surface
		 *
		 * This function blits the source area enclosed be @srect to the point @c dpoint in the destination surface.
		 * Clipping will be performed if needed.
		 *
		 * @param srect source area
		 * @param dst  destination surface
		 * @param dpoint destination point
		 * @throws @see sdl::exception on failure
		 */
		void blit (const rect& srect, object<SDL_Surface>& dst, const point& dpoint) const;

		/**
		 * @brief Blit this surface to @c dst surface
		 *
		 * This function blits the source area enclosed be @srect to the (0,0) point in the destination surface.
		 * Clipping will be performed if needed.
		 *
		 * @param srect source area
		 * @param dst destination surface
		 * @throws @see sdl::exception on failure
		 */
		void blit (const rect& srect, object<SDL_Surface>& dst) const;
		/**
		 * @brief Blit this surface to @c dst surface
		 *
		 * This function blits the whole surface to the (0,0) point in the destination surface.
		 * Clipping will be performed if needed.
		 *
		 * @param dst destination surface
		 * @throws @see sdl::exception on failure
		 */
		void blit (object<SDL_Surface>& dst) const;

		// returns pixels, pitch, w, h
		[[nodiscard]] std::tuple<void*, std::size_t, unsigned, unsigned> pixels_data () const;
		[[nodiscard]] area_type get_dimanesions() const;

		void fill (const rect& r, uint32_t c);
		void fill (const rect& r, const color& c);
		void fill (uint32_t c);
		void fill (const color& c);
		void fill (const bsw::array_view1d<rect>& rects, uint32_t c);
		void fill (const bsw::array_view1d<rect>& rects, const color& c);

		[[nodiscard]] surface convert (const pixel_format& fmt) const;

		[[nodiscard]] pixel_format get_pixel_format () const;
		[[nodiscard]] uint32_t map_color (const color& c);

		// this method returns reference to the actual palette
		[[nodiscard]] palette get_palette () const;
		void set_palette (const palette& pal);

		[[nodiscard]] surface roto_zoom (double angle, double zoom, bool smooth) const;
		[[nodiscard]] surface roto_zoom (double angle, double zoom_x, double zoom_y, bool smooth) const;

		[[nodiscard]] area_type roto_zoom_size (double angle, double zoom) const;
		[[nodiscard]] area_type roto_zoom_size (double angle, double zoom_x, double zoom_y) const;
	};

	namespace detail {
		template <>
		struct locker_traits<surface> {
			static bool must_lock(const surface& s) {
				return s.must_lock();
			}
			static void lock(surface& s) {
				s.lock();
			}
			static void unlock(surface& s) {
				s.unlock();
			}
		};
	}
}

// ===================================================================================================
// Implementation
// ===================================================================================================
namespace neutrino::sdl {

	inline
	surface::surface () = default;

	// ----------------------------------------------------------------------------------------------
	inline
	surface::surface (object<SDL_Surface>&& other)
		: object<SDL_Surface> (std::move (other)) {

	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface::surface (surface&& other) noexcept
		: object<SDL_Surface> (nullptr, false) {
		other.swap (*this);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface& surface::operator= (surface&& other) noexcept {
		object<SDL_Surface>::operator= (std::move (other));
		return *this;
	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface::surface (const object<SDL_Window>& other)
		: object<SDL_Surface> (SAFE_SDL_CALL(SDL_GetWindowSurface, const_cast<SDL_Window*>(other.handle ())), false) {

	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface& surface::operator= (object<SDL_Surface>&& other) noexcept {
		object<SDL_Surface>::operator= (std::move (other));
		return *this;
	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface::surface (unsigned width, unsigned height, pixel_format format)
		: object<SDL_Surface> (SAFE_SDL_CALL(SDL_CreateRGBSurfaceWithFormat, 0,
											 static_cast<int>(width), static_cast<int>(height), format
												 .get_bits_per_pixels (), format.value ()), true) {

	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface::surface (void* data, unsigned width, unsigned height, unsigned pitch, pixel_format format)
		: object<SDL_Surface> (SAFE_SDL_CALL(SDL_CreateRGBSurfaceWithFormatFrom, data,
											 static_cast<int>(width),
											 static_cast<int>(height),
											 format.get_bits_per_pixels (),
											 static_cast<int>(pitch),
											 format.value ()), true) {

	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface::surface (object<SDL_RWops>& rwops)
		: object<SDL_Surface> (SAFE_SDL_CALL(IMG_Load_RW, rwops.handle (), SDL_FALSE), true) {

	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface surface::make_8bit (unsigned width, unsigned height) {
		return {width, height, pixel_format::make_8bit ()};
	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface surface::make_rgba_32bit (unsigned width, unsigned height) {
		return {width, height, pixel_format::make_rgba_32bit ()};
	}

	// ----------------------------------------------------------------------------------------------


	inline
	surface surface::clone () const {
		return surface (object<SDL_Surface> (SAFE_SDL_CALL(SDL_ConvertSurface, const_cast<SDL_Surface*>(handle ()), handle ()
			->format, SDL_SWSURFACE), true));
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::save_bmp (const std::string& path) const {
		SAFE_SDL_CALL(SDL_SaveBMP_RW, const_handle (), SDL_RWFromFile (path.c_str (), "wb"), 1);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::save_bmp (object<SDL_RWops>& stream) const {
		SAFE_SDL_CALL(SDL_SaveBMP_RW, const_handle (), stream.handle (), 0);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::lock () noexcept {
		SDL_LockSurface (handle ());
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::unlock () noexcept {
		SDL_UnlockSurface (handle ());
	}

	// ----------------------------------------------------------------------------------------------
	inline
	bool surface::must_lock () const noexcept {
		return SDL_MUSTLOCK(handle ());
	}

	// ----------------------------------------------------------------------------------------------
	inline
	rect surface::get_clip () const {
		SDL_Rect r;
		SDL_GetClipRect (const_handle (), &r);
		return rect{r};
	}

	// ----------------------------------------------------------------------------------------------
	inline
	bool surface::set_clip (const rect& r) {
		return SDL_TRUE == SDL_SetClipRect (handle (), &r);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::color_key (uint32_t c) {
		SAFE_SDL_CALL(SDL_SetColorKey, handle (), SDL_TRUE, c);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::color_key (const color& c) {
		SAFE_SDL_CALL(SDL_SetColorKey, handle (), SDL_TRUE, map_color (c));
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::disable_color_key () {
		SAFE_SDL_CALL(SDL_SetColorKey, handle (), SDL_FALSE, 0);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	std::optional<uint32_t> surface::color_key () const {
		uint32_t c;
		auto rc = SDL_GetColorKey (const_handle (), &c);
		if (0 == rc) {
			return c;
		}
		return std::nullopt;
	}

	// ----------------------------------------------------------------------------------------------
	inline
	blend_mode surface::get_blend_mode () const {
		SDL_BlendMode x;
		SAFE_SDL_CALL(SDL_GetSurfaceBlendMode, const_handle (), &x);
		return static_cast<blend_mode>(x);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::set_blend_mode (blend_mode v) {
		SAFE_SDL_CALL(SDL_SetSurfaceBlendMode, handle (), static_cast<SDL_BlendMode>(v));
	}

	// ----------------------------------------------------------------------------------------------
	inline
	uint8_t surface::get_alpha_mod () const {
		uint8_t x;
		SAFE_SDL_CALL(SDL_GetSurfaceAlphaMod, const_handle (), &x);
		return x;
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::set_alpha_mod (uint8_t v) {
		SAFE_SDL_CALL(SDL_SetSurfaceAlphaMod, handle (), v);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::set_rle_acceleration (bool enabled) {
		SAFE_SDL_CALL(SDL_SetSurfaceRLE, handle (), enabled ? 1 : 0);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::set_color_mod (uint8_t r, uint8_t g, uint8_t b) {
		SAFE_SDL_CALL(SDL_SetSurfaceColorMod, handle (), r, g, b);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	std::optional<color> surface::get_color_mod () const {
		color c{0, 0, 0, 0};
		if (0 == SDL_GetSurfaceColorMod (const_handle (), &c.r, &c.g, &c.b)) {
			return c;
		}
		return std::nullopt;
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::blit (const rect& srect, object<SDL_Surface>& dst, const rect& drect) const {
		SAFE_SDL_CALL(SDL_BlitSurface,
					  const_handle (), const_cast<rect*>(&srect),
					  dst.handle (), const_cast<rect*>(&drect));
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::blit_scaled (const rect& srect, object<SDL_Surface>& dst) const {
		SAFE_SDL_CALL(SDL_BlitScaled, const_handle (), &srect, dst.handle (), nullptr);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::blit_scaled (object<SDL_Surface>& dst) const {
		SAFE_SDL_CALL(SDL_BlitScaled, const_handle (), nullptr, dst.handle (), nullptr);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::blit_scaled (const rect& srect, object<SDL_Surface>& dst, const rect& drect) const {
		SAFE_SDL_CALL(SDL_BlitScaled, const_handle (), &srect, dst.handle (), const_cast<rect*>(&drect));
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::blit (const rect& srect, object<SDL_Surface>& dst, const point& dpoint) const {
		rect dstrect{dpoint, srect.w, srect.h};
		blit (srect, dst, dstrect);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::blit (const rect& srect, object<SDL_Surface>& dst) const {
		SAFE_SDL_CALL(SDL_BlitSurface,
					  const_handle (), &srect,
					  dst.handle (), nullptr);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::blit (object<SDL_Surface>& dst) const {
		SAFE_SDL_CALL(SDL_BlitSurface,
					  const_handle (), nullptr,
					  dst.handle (), nullptr);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	std::tuple<void*, std::size_t, unsigned, unsigned> surface::pixels_data () const {
		const auto* s = handle ();
		return {s->pixels, s->pitch, s->w, s->h};
	}
	// ----------------------------------------------------------------------------------------------
	inline
	area_type surface::get_dimanesions() const {
		const auto* s = handle ();
		return {s->w, s->h};
	}
	// ----------------------------------------------------------------------------------------------
	inline
	void surface::fill (const rect& r, uint32_t c) {
		SAFE_SDL_CALL(SDL_FillRect, handle (), &r, c);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::fill (const rect& r, const color& c) {
		SAFE_SDL_CALL(SDL_FillRect, handle (), &r, map_color (c));
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::fill (uint32_t c) {
		SAFE_SDL_CALL(SDL_FillRect, handle (), nullptr, c);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::fill (const color& c) {
		SAFE_SDL_CALL(SDL_FillRect, handle (), nullptr, map_color (c));
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::fill (const bsw::array_view1d<rect>& rects, uint32_t c) {
#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable : 4267 )
#endif
		SAFE_SDL_CALL(SDL_FillRects, handle (), rects.begin (), rects.size (), c);
#if defined(_MSC_VER)
#pragma warning ( pop )
#endif
	}

	// ----------------------------------------------------------------------------------------------
	inline
	void surface::fill (const bsw::array_view1d<rect>& rects, const color& c) {
#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable : 4267 )
#endif
		SAFE_SDL_CALL(SDL_FillRects, handle (), rects.begin (), rects.size (), map_color (c));
#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

	}

	// ----------------------------------------------------------------------------------------------
	inline
	surface surface::convert (const pixel_format& fmt) const {
		SDL_Surface* s = SAFE_SDL_CALL(SDL_ConvertSurfaceFormat, const_handle (), fmt.value (), 0);
		return surface{object<SDL_Surface> (s, true)};
	}

	// ----------------------------------------------------------------------------------------------
	inline
	pixel_format surface::get_pixel_format () const {
		return pixel_format (handle ()->format->format);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	uint32_t surface::map_color (const color& c) {
		if (get_pixel_format ().is_alpha ()) {
			return SDL_MapRGBA (handle ()->format, c.r, c.g, c.b, c.a);
		}
		return SDL_MapRGB (handle ()->format, c.r, c.g, c.b);
	}

	// --------------------------------------------------------------------------------------------
	inline
	palette surface::get_palette () const {
		return palette (*this);
	}

	// --------------------------------------------------------------------------------------------
	inline
	void surface::set_palette (const palette& pal) {
		SAFE_SDL_CALL(SDL_SetSurfacePalette, handle (), pal.const_handle ());
	}
}
#endif
