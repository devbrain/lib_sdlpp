//
// Created by igor on 5/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_TTF_HH_
#define SDLPP_INCLUDE_SDLPP_TTF_HH_

#include <tuple>
#include <string>
#include <array>
#include <type_traits>
#include <optional>

#include <bitflags/bitflags.hpp>
#include <bsw/errors.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ttf_helper.hh>
#include <sdlpp/io/io.hh>
#include <sdlpp/video/surface.hh>

namespace neutrino::sdl {
	/**
	 * @class ttf
	 * @brief Represents a TrueType font object.
	 *
	 * This provides functionality to load and manipulate TrueType fonts.
	 */
	class ttf : public object <TTF_Font> {
		public:
			BEGIN_BITFLAGS(style_t)
				FLAG(NORMAL)
				FLAG(BOLD)
				FLAG(ITALIC)
				FLAG(UNDERLINE)
			END_BITFLAGS(style_t)

			enum class hinting_t : int {
				NORMAL = TTF_HINTING_NORMAL,
				LIGHT = TTF_HINTING_LIGHT,
				MONO = TTF_HINTING_MONO,
				NONE = TTF_HINTING_NONE,
				LIGHT_SUBPIXEL = TTF_HINTING_LIGHT_SUBPIXEL
			};

			enum class alignment_t : int {
				LEFT = TTF_WRAPPED_ALIGN_LEFT,
				CENTER = TTF_WRAPPED_ALIGN_CENTER,
				RIGHT = TTF_WRAPPED_ALIGN_RIGHT
			};

		public:
			ttf() = default;

			ttf(const std::string& path, int ptsize);
			ttf(const std::string& path, int ptsize, unsigned int hdpi, unsigned int vdpi);

			ttf(const std::string& path, int ptsize, int index);
			ttf(const std::string& path, int ptsize, int index, unsigned int hdpi, unsigned int vdpi);

			ttf(object <SDL_RWops>& rwops, int ptsize);
			ttf(object <SDL_RWops>& rwops, int ptsize, unsigned int hdpi, unsigned int vdpi);
			ttf(object <SDL_RWops>& rwops, int ptsize, int index);
			ttf(object <SDL_RWops>& rwops, int ptsize, int index, unsigned int hdpi, unsigned int vdpi);

			explicit ttf(object <TTF_Font>&& other);
			ttf& operator=(object <TTF_Font>&& other) noexcept;
			ttf& operator=(ttf&& other) noexcept;

			ttf& operator=(const ttf& other) = delete;
			ttf(const ttf& other) = delete;

			void set_font_size(int ptsize);
			void set_font_size(int ptsize, unsigned int hdpi, unsigned int vdpi);

			[[nodiscard]] style_t get_style() const;
			void set_style(style_t style);

			[[nodiscard]] int get_outline() const;
			void set_outline(int outline);

			[[nodiscard]] hinting_t get_hinting() const;
			void set_hiniting(hinting_t hinting);

			[[nodiscard]] alignment_t get_alignment() const;
			void set_alignment(alignment_t alignemt);

			[[nodiscard]] int get_height() const;
			[[nodiscard]] int get_ascent() const;
			[[nodiscard]] int get_descent() const;
			[[nodiscard]] int get_line_skip() const;
			[[nodiscard]] bool get_kerning_enabled() const;
			void set_kerning_enabled(bool v);

			[[nodiscard]] bool get_sdf_enabled() const;
			void set_sdf_enabled(bool v);

			[[nodiscard]] std::size_t get_faces() const;
			[[nodiscard]] bool get_is_fixed_width() const;

			[[nodiscard]] std::string get_face_family_name() const;
			[[nodiscard]] std::string get_face_style_name() const;

			[[nodiscard]] bool has_glyph(wchar_t ch) const;

			// int minx, int maxx,
			// int miny, int maxy, int *advance
			using metrics_t = std::tuple <int, int, int, int, int>;
			[[nodiscard]] std::optional <metrics_t> get_metrics(wchar_t ch) const;

			// width, height in pixels
			using text_size_t = area_type;

			template<typename LikeAString>
			[[nodiscard]] std::optional <text_size_t> get_text_size(LikeAString&& str,
			                                                        ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAChar>
			[[nodiscard]] int get_kerning(LikeAChar a, LikeAChar b, ensure_char <LikeAChar>  = nullptr) const;
			// num of chars that can be rendered, latest calculated width
			template<typename LikeAString>
			[[nodiscard]] text_size_t measure_text(LikeAString&& str, int max_width_pixels,
			                                       ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAString>
			[[nodiscard]] surface render_transparent(LikeAString&& str, color fg,
			                                         ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAChar>
			[[nodiscard]] surface render_transparent(LikeAChar ch, color fg, ensure_char <LikeAChar>  = nullptr) const;

			template<typename LikeAString>
			[[nodiscard]] surface render_transparent(LikeAString&& str, color fg, int max_width_pixels,
			                                         ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAString>
			[[nodiscard]] surface render_blended(LikeAString&& str, color fg,
			                                     ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAString>
			[[nodiscard]] surface render_blended(LikeAString&& str, color fg, int max_width_pixels,
			                                     ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAChar>
			[[nodiscard]] surface render_blended(LikeAChar ch, color fg, ensure_char <LikeAChar>  = nullptr) const;

			template<typename LikeAString>
			[[nodiscard]] surface render_lcd(LikeAString&& str, color fg, color bg,
			                                 ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAString>
			[[nodiscard]] surface render_lcd(LikeAString&& str, color fg, color bg, int max_width_pixels,
			                                 ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAChar>
			[[nodiscard]] surface render_lcd(LikeAChar ch, color fg, color bg,
			                                 ensure_char <LikeAChar>  = nullptr) const;

			template<typename LikeAString>
			[[nodiscard]] surface render_shaded(LikeAString&& str, color fg, color bg,
			                                    ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAString>
			[[nodiscard]] surface render_shaded(LikeAString&& str, color fg, color bg, int max_width_pixels,
			                                    ensure_string <LikeAString>  = nullptr) const;

			template<typename LikeAChar>
			[[nodiscard]] surface render_shaded(LikeAChar ch, color fg, color bg,
			                                    ensure_char <LikeAChar>  = nullptr) const;
	};

	d_SDLPP_OSTREAM(ttf::style_t);
	d_SDLPP_OSTREAM(ttf::alignment_t);
	d_SDLPP_OSTREAM(ttf::hinting_t);
}

// =============================================================================
// Implementation
// =============================================================================

namespace neutrino::sdl {
	inline
	ttf& ttf::operator=(object <TTF_Font>&& other) noexcept {
		object <TTF_Font>::operator=(std::move(other));
		return *this;;
	}

	inline
	ttf::ttf(object <TTF_Font>&& other)
		: object <TTF_Font>(std::move(other)) {
	}

	inline
	ttf& ttf::operator=(ttf&& other) noexcept {
		object <TTF_Font>::operator=(std::move(other));
		return *this;
	}

	inline
	ttf::ttf(const std::string& path, int ptsize)
		: object <TTF_Font>(SAFE_SDL_CALL(TTF_OpenFont, path.c_str (), ptsize), true) {
	}

	inline
	ttf::ttf(const std::string& path, int ptsize, int index)
		: object <TTF_Font>(SAFE_SDL_CALL(TTF_OpenFontIndex, path.c_str (), ptsize, index), true) {
	}

	inline
	ttf::ttf(object <SDL_RWops>& rwops, int ptsize)
		: object <TTF_Font>(SAFE_SDL_CALL(TTF_OpenFontRW, rwops.handle (), 0, ptsize), true) {
	}

	inline
	ttf::ttf(object <SDL_RWops>& rwops, int ptsize, int index)
		: object <TTF_Font>(SAFE_SDL_CALL(TTF_OpenFontIndexRW, rwops.handle (), 0, ptsize, index), true) {
	}

	inline
	ttf::ttf(const std::string& path, int ptsize, unsigned int hdpi, unsigned int vdpi)
		: object <TTF_Font>(SAFE_SDL_CALL(TTF_OpenFontDPI, path.c_str (), ptsize, hdpi, vdpi), true) {
	}

	inline
	ttf::ttf(const std::string& path, int ptsize, int index, unsigned int hdpi, unsigned int vdpi)
		: object <TTF_Font>(SAFE_SDL_CALL(TTF_OpenFontIndexDPI, path.c_str (), ptsize, index, hdpi, vdpi), true) {
	}

	inline
	ttf::ttf(object <SDL_RWops>& rwops, int ptsize, unsigned int hdpi, unsigned int vdpi)
		: object <TTF_Font>(SAFE_SDL_CALL(TTF_OpenFontDPIRW, rwops.handle (), 0, ptsize, hdpi, vdpi), true) {
	}

	inline
	ttf::ttf(object <SDL_RWops>& rwops, int ptsize, int index, unsigned int hdpi, unsigned int vdpi)
		: object <TTF_Font>(SAFE_SDL_CALL(TTF_OpenFontIndexDPIRW, rwops
		                                  .handle (), 0, ptsize, index, hdpi, vdpi)		                    , true) {
	}

	inline
	void ttf::set_font_size(int ptsize) {
		SAFE_SDL_CALL(TTF_SetFontSize, handle (), ptsize);
	}

	inline
	void ttf::set_font_size(int ptsize, unsigned int hdpi, unsigned int vdpi) {
		SAFE_SDL_CALL(TTF_SetFontSizeDPI, handle (), ptsize, hdpi, vdpi);
	}

	inline
	ttf::style_t ttf::get_style() const {
		style_t f = style_t::NORMAL;
		auto s = TTF_GetFontStyle(handle());
		if ((s & TTF_STYLE_NORMAL) == TTF_STYLE_NORMAL) {
			f |= style_t::NORMAL;
		}
		if ((s & TTF_STYLE_BOLD) == TTF_STYLE_BOLD) {
			f |= style_t::BOLD;
		}
		if ((s & TTF_STYLE_ITALIC) == TTF_STYLE_ITALIC) {
			f |= style_t::ITALIC;
		}
		if ((s & TTF_STYLE_UNDERLINE) == TTF_STYLE_UNDERLINE) {
			f |= style_t::UNDERLINE;
		}
		return f;
	}

	inline
	void ttf::set_style(ttf::style_t style) {
		int s = TTF_STYLE_NORMAL;
		if ((style & style_t::NORMAL)) {
			s |= TTF_STYLE_NORMAL;
		}
		if ((style & style_t::BOLD)) {
			s |= TTF_STYLE_BOLD;
		}
		if ((style & style_t::ITALIC)) {
			s |= TTF_STYLE_ITALIC;
		}
		if ((style & style_t::UNDERLINE)) {
			s |= TTF_STYLE_UNDERLINE;
		}
		TTF_SetFontStyle(handle(), s);
	}

	inline
	int ttf::get_outline() const {
		return TTF_GetFontOutline(handle());
	}

	inline
	void ttf::set_outline(int outline) {
		TTF_SetFontOutline(handle(), outline);
	}

	inline
	ttf::hinting_t ttf::get_hinting() const {
		return (hinting_t)TTF_GetFontHinting(handle());
	}

	inline
	void ttf::set_hiniting(ttf::hinting_t hinting) {
		TTF_SetFontHinting(handle(), (int)hinting);
	}

	inline
	ttf::alignment_t ttf::get_alignment() const {
		return (alignment_t)TTF_GetFontWrappedAlign(handle());
	}

	inline
	void ttf::set_alignment(ttf::alignment_t alignemt) {
		TTF_SetFontWrappedAlign(handle(), (int)alignemt);
	}

	inline
	int ttf::get_height() const {
		return TTF_FontHeight(handle());
	}

	inline
	int ttf::get_ascent() const {
		return TTF_FontAscent(handle());
	}

	inline
	int ttf::get_descent() const {
		return TTF_FontDescent(handle());
	}

	inline
	int ttf::get_line_skip() const {
		return TTF_FontLineSkip(handle());
	}

	inline
	bool ttf::get_kerning_enabled() const {
		return TTF_GetFontKerning(handle()) != 0;
	}

	inline
	void ttf::set_kerning_enabled(bool v) {
		TTF_SetFontKerning(handle(), v ? 1 : 0);
	}

	inline
	bool ttf::get_sdf_enabled() const {
		return TTF_GetFontSDF(handle()) == SDL_TRUE;
	}

	inline
	void ttf::set_sdf_enabled(bool v) {
		TTF_SetFontSDF(handle(), v ? SDL_TRUE : SDL_FALSE);
	}

	inline
	std::size_t ttf::get_faces() const {
		return (size_t)TTF_FontFaces(handle());
	}

	inline
	bool ttf::get_is_fixed_width() const {
		return TTF_FontFaceIsFixedWidth(handle()) != 0;
	}

	inline
	std::string ttf::get_face_family_name() const {
		return TTF_FontFaceFamilyName(handle());
	}

	inline
	std::string ttf::get_face_style_name() const {
		return TTF_FontFaceStyleName(handle());
	}

	inline
	bool ttf::has_glyph(wchar_t ch) const {
		static_assert(sizeof (ch) == 4 || sizeof (ch) == 2);
		if constexpr (sizeof (ch) == 2) {
			return TTF_GlyphIsProvided(const_cast <TTF_Font*>(handle()), (Uint16)ch);
		} else {
			return TTF_GlyphIsProvided32(const_cast <TTF_Font*>(handle()), (Uint32)ch);
		}
	}

	inline
	std::optional <ttf::metrics_t> ttf::get_metrics(wchar_t ch) const {
		static_assert(sizeof (ch) == 4 || sizeof (ch) == 2);

		int minx;
		int maxx;
		int miny;
		int maxy;
		int advance;
		bool rc = false;
		if constexpr (sizeof (ch) == 2) {
			rc = TTF_GlyphMetrics(const_cast <TTF_Font*>(handle()), (Uint16)ch, &minx, &maxx, &miny, &maxy, &advance)
			     != 0;
		} else {
			rc = TTF_GlyphMetrics32(const_cast <TTF_Font*>(handle()), (Uint32)ch, &minx, &maxx, &miny, &maxy, &advance)
			     != 0;
		}
		if (rc) {
			return std::make_tuple(minx, maxx, miny, maxy, advance);
		}
		return std::nullopt;
	}

	template<typename LikeAString>
	[[nodiscard]] std::optional <ttf::text_size_t> ttf::get_text_size(LikeAString&& str,
	                                                                  ensure_string <LikeAString>) const {
		int w;
		int h;
		if (0 != detail::TTF_Size_impl::call(const_cast <TTF_Font*>(handle()), std::forward <LikeAString&&>(str), &w,
		                                     &h)) {
			return area_type(w, h);
		}
		return std::nullopt;
	}

	template<typename LikeAString>
	[[nodiscard]] ttf::text_size_t ttf::measure_text(LikeAString&& str, int max_width_pixels,
	                                                 ensure_string <LikeAString>) const {
		int extent;
		int count;
		auto* f = const_cast <TTF_Font*>(handle());
		detail::TTF_Measure_impl::call(f, std::forward <LikeAString&&>(str), max_width_pixels, &extent, &count);
		return {count, extent};
	}

	template<typename LikeAString>
	surface ttf::render_transparent(LikeAString&& str, color fg, ensure_string <LikeAString>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = detail::TTF_RenderSolid_impl::call(f, std::forward <LikeAString&&>(str), fg);
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAString>
	surface ttf::render_transparent(LikeAString&& str, color fg, int max_width_pixels,
	                                ensure_string <LikeAString>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = detail::TTF_RenderSolid_Wrapped_impl::call(
			f, std::forward <LikeAString&&>(str), fg, max_width_pixels);
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAChar>
	surface ttf::render_transparent(LikeAChar ch, color fg, ensure_char <LikeAChar>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = nullptr;
		if constexpr (std::is_same_v <LikeAChar, char> || sizeof (ch) == 2) {
			rendered_surface = TTF_RenderGlyph_Solid(f, detail::proxy(ch), fg);
		} else {
			rendered_surface = TTF_RenderGlyph32_Solid(f, detail::proxy(ch), fg);
		}
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAString>
	surface ttf::render_blended(LikeAString&& str, color fg, ensure_string <LikeAString>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = detail::TTF_RenderBlended_impl::call(f, std::forward <LikeAString&&>(str), fg);
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAString>
	surface ttf::render_blended(LikeAString&& str, color fg, int max_width_pixels, ensure_string <LikeAString>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = detail::TTF_RenderBlended_Wrapped_impl::call(
			f, std::forward <LikeAString&&>(str), fg, max_width_pixels);
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAChar>
	surface ttf::render_blended(LikeAChar ch, color fg, ensure_char <LikeAChar>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = nullptr;
		if constexpr (std::is_same_v <LikeAChar, char> || sizeof (ch) == 2) {
			rendered_surface = TTF_RenderGlyph_Blended(f, detail::proxy(ch), fg);
		} else {
			rendered_surface = TTF_RenderGlyph32_Blended(f, detail::proxy(ch), fg);
		}
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAString>
	surface ttf::render_lcd(LikeAString&& str, color fg, color bg, ensure_string <LikeAString>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = detail::TTF_RenderLCD_impl::call(f, std::forward <LikeAString&&>(str), fg, bg);
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAString>
	surface ttf::render_lcd(LikeAString&& str, color fg, color bg, int max_width_pixels,
	                        ensure_string <LikeAString>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = detail::TTF_RenderLCD_Wrapped_impl::call(
			f, std::forward <LikeAString&&>(str), fg, bg, max_width_pixels);
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAChar>
	surface ttf::render_lcd(LikeAChar ch, color fg, color bg, ensure_char <LikeAChar>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = nullptr;
		if constexpr (std::is_same_v <LikeAChar, char> || sizeof (ch) == 2) {
			rendered_surface = TTF_RenderGlyph_LCD(f, detail::proxy(ch), fg, bg);
		} else {
			rendered_surface = TTF_RenderGlyph32_LCD(f, detail::proxy(ch), fg, bg);
		}
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAString>
	surface ttf::render_shaded(LikeAString&& str, color fg, color bg, ensure_string <LikeAString>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = detail::TTF_RenderShaded_impl::call(
			f, std::forward <LikeAString&&>(str), fg, bg);
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAString>
	surface ttf::render_shaded(LikeAString&& str, color fg, color bg, int max_width_pixels,
	                           ensure_string <LikeAString>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = detail::TTF_RenderShaded_Wrapped_impl::call(
			f, std::forward <LikeAString&&>(str), fg, bg, max_width_pixels);
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAChar>
	surface ttf::render_shaded(LikeAChar ch, color fg, color bg, ensure_char <LikeAChar>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		SDL_Surface* rendered_surface = nullptr;
		if constexpr (std::is_same_v <LikeAChar, char> || sizeof (ch) == 2) {
			rendered_surface = TTF_RenderGlyph_Shaded(f, detail::proxy(ch), fg, bg);
		} else {
			rendered_surface = TTF_RenderGlyph32_Shaded(f, detail::proxy(ch), fg, bg);
		}
		ENFORCE(rendered_surface)
		return surface(object <SDL_Surface>(rendered_surface, true));
	}

	template<typename LikeAChar>
	int ttf::get_kerning(LikeAChar a, LikeAChar b, ensure_char <LikeAChar>) const {
		auto* f = const_cast <TTF_Font*>(handle());
		if constexpr (std::is_same_v <LikeAChar, char> || sizeof (a) == 2) {
			return TTF_GetFontKerningSizeGlyphs(f, detail::proxy(a), detail::proxy(b));
		} else {
			return TTF_GetFontKerningSizeGlyphs32(f, detail::proxy(a), detail::proxy(b));
		}
	}

	namespace detail {
		static inline constexpr std::array <ttf::hinting_t, 5> s_vals_of_ttf_hinting_t = {
			ttf::hinting_t::NORMAL,
			ttf::hinting_t::LIGHT,
			ttf::hinting_t::MONO,
			ttf::hinting_t::NONE,
			ttf::hinting_t::LIGHT_SUBPIXEL,
		};
	}

	template<typename T>
	static constexpr const decltype(detail::s_vals_of_ttf_hinting_t)&
	values(std::enable_if_t <std::is_same_v <ttf::hinting_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_hinting_t;
	}

	template<typename T>
	static constexpr auto
	begin(std::enable_if_t <std::is_same_v <ttf::hinting_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_hinting_t.begin();
	}

	template<typename T>
	static constexpr auto
	end(std::enable_if_t <std::is_same_v <ttf::hinting_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_hinting_t.end();
	}

	namespace detail {
		static inline constexpr std::array <ttf::alignment_t, 3> s_vals_of_ttf_alignment_t = {
			ttf::alignment_t::LEFT,
			ttf::alignment_t::CENTER,
			ttf::alignment_t::RIGHT,
		};
	}

	template<typename T>
	static constexpr const decltype(detail::s_vals_of_ttf_alignment_t)&
	values(std::enable_if_t <std::is_same_v <ttf::alignment_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_alignment_t;
	}

	template<typename T>
	static constexpr auto
	begin(std::enable_if_t <std::is_same_v <ttf::alignment_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_alignment_t.begin();
	}

	template<typename T>
	static constexpr auto
	end(std::enable_if_t <std::is_same_v <ttf::alignment_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_alignment_t.end();
	}

	namespace detail {
		static inline constexpr std::array <ttf::style_t::flag_type, 4> s_vals_of_ttf_style_t = {
			ttf::style_t::NORMAL,
			ttf::style_t::BOLD,
			ttf::style_t::ITALIC,
			ttf::style_t::UNDERLINE,
		};
	}

	template<typename T>
	static constexpr const decltype(detail::s_vals_of_ttf_style_t)&
	values(std::enable_if_t <std::is_same_v <ttf::style_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_style_t;
	}

	template<typename T>
	static constexpr auto
	begin(std::enable_if_t <std::is_same_v <ttf::style_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_style_t.begin();
	}

	template<typename T>
	static constexpr auto
	end(std::enable_if_t <std::is_same_v <ttf::style_t, T>>* = nullptr) {
		return detail::s_vals_of_ttf_style_t.end();
	}
}

#endif //SDLPP_INCLUDE_SDLPP_TTF_HH_
