//
// Created by igor on 5/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_TTF_HH_
#define SDLPP_INCLUDE_SDLPP_TTF_HH_

#include <tuple>
#include <string>
#include <optional>

#include <bitflags/bitflags.hpp>
#include <sdlpp/sdl2.hh>
#include <sdlpp/object.hh>
#include <sdlpp/io.hh>

namespace neutrino::sdl {
	class font : public object<TTF_Font> {
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
		font () = default;

		font (const std::string& path, int ptsize);
		font (const std::string& path, int ptsize, unsigned int hdpi, unsigned int vdpi);

		font (const std::string& path, int ptsize, int index);
		font (const std::string& path, int ptsize, int index, unsigned int hdpi, unsigned int vdpi);

		font (io& rwops, int ptsize);
		font (io& rwops, int ptsize, unsigned int hdpi, unsigned int vdpi);
		font (io& rwops, int ptsize, int index);
		font (io& rwops, int ptsize, int index, unsigned int hdpi, unsigned int vdpi);

		font (object<TTF_Font>&& other);
		font& operator= (object<TTF_Font>&& other) noexcept;
		font& operator= (font&& other) noexcept;

		font& operator= (const font& other) = delete;
		font (const font& other) = delete;

		void set_font_size (int ptsize);
		void set_font_size (int ptsize, unsigned int hdpi, unsigned int vdpi);

		[[nodiscard]] style_t get_style () const;
		void set_style (style_t style);

		[[nodiscard]] int get_outline () const;
		void set_outline (int outline);

		[[nodiscard]] hinting_t get_hinting () const;
		void set_hiniting (hinting_t hinting);

		[[nodiscard]] alignment_t get_alignment () const;
		void set_alignment (alignment_t alignemt);

		[[nodiscard]] int get_height () const;
		[[nodiscard]] int get_ascent () const;
		[[nodiscard]] int get_descent () const;
		[[nodiscard]] int get_line_skip () const;
		[[nodiscard]] bool get_kerning_enabled () const;
		void set_kerning_enabled (bool v);

		[[nodiscard]] std::size_t get_faces () const;
		[[nodiscard]] bool get_is_fixed_width () const;

		[[nodiscard]] std::string get_face_family_name () const;
		[[nodiscard]] std::string get_face_style_name () const;

		[[nodiscard]] bool has_glyph (wchar_t ch) const;
		// int *minx, int *maxx,
		// int *miny, int *maxy, int *advance

		using metrics_t = std::tuple<int, int, int, int, int>;
		[[nodiscard]] std::optional<metrics_t> get_metrics (wchar_t ch) const;

		// width, height in pixels
		using text_size_t = std::tuple<int, int>;
		[[nodiscard]] std::optional<text_size_t> get_text_size (const std::string& str) const;
		[[nodiscard]] std::optional<text_size_t> get_text_size (const std::wstring& str) const;

		// num of chars that can be rendered, latest calculated width
		[[nodiscard]] text_size_t measure_text (const std::string& str, int max_width_pixels) const;
		[[nodiscard]] text_size_t measure_text (const std::wstring& str, int max_width_pixels) const;

	};

}

// =============================================================================
// Implementation
// =============================================================================

namespace neutrino::sdl {

	inline
	font& font::operator= (object<TTF_Font>&& other) noexcept {
		object<TTF_Font>::operator= (std::move (other));
		return *this;;
	}

	inline
	font::font (object<TTF_Font>&& other)
		: object<TTF_Font> (std::move (other)) {

	}

	inline
	font& font::operator= (font&& other) noexcept {
		object<TTF_Font>::operator= (std::move (other));
		return *this;
	}

	inline
	font::font (const std::string& path, int ptsize)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFont, path.c_str (), ptsize), true) {

	}

	inline
	font::font (const std::string& path, int ptsize, int index)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontIndex, path.c_str (), ptsize, index), true) {
	}

	inline
	font::font (io& rwops, int ptsize)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontRW, rwops.handle (), 0, ptsize), true) {
	}

	inline
	font::font (io& rwops, int ptsize, int index)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontIndexRW, rwops.handle (), 0, ptsize, index), true) {
	}

	inline
	font::font (const std::string& path, int ptsize, unsigned int hdpi, unsigned int vdpi)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontDPI, path.c_str (), ptsize, hdpi, vdpi), true) {

	}

	inline
	font::font (const std::string& path, int ptsize, int index, unsigned int hdpi, unsigned int vdpi)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontIndexDPI, path.c_str (), ptsize, index, hdpi, vdpi), true) {

	}

	inline
	font::font (io& rwops, int ptsize, unsigned int hdpi, unsigned int vdpi)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontDPIRW, rwops.handle (), 0, ptsize, hdpi, vdpi), true) {
	}

	inline
	font::font (io& rwops, int ptsize, int index, unsigned int hdpi, unsigned int vdpi)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontIndexDPIRW, rwops
		.handle (), 0, ptsize, index, hdpi, vdpi), true) {
	}

	inline
	void font::set_font_size (int ptsize) {
		SAFE_SDL_CALL(TTF_SetFontSize, handle (), ptsize);
	}

	inline
	void font::set_font_size (int ptsize, unsigned int hdpi, unsigned int vdpi) {
		SAFE_SDL_CALL(TTF_SetFontSizeDPI, handle (), ptsize, hdpi, vdpi);
	}

	inline
	font::style_t font::get_style () const {
		style_t f = style_t::NORMAL;
		auto s = TTF_GetFontStyle (handle ());
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
	void font::set_style (font::style_t style) {
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
		TTF_SetFontStyle (handle (), s);
	}

	inline
	int font::get_outline () const {
		return TTF_GetFontOutline (handle ());
	}

	inline
	void font::set_outline (int outline) {
		TTF_SetFontOutline (handle (), outline);
	}

	inline
	font::hinting_t font::get_hinting () const {
		return (hinting_t)TTF_GetFontHinting (handle ());
	}

	inline
	void font::set_hiniting (font::hinting_t hinting) {
		TTF_SetFontHinting (handle (), (int)hinting);
	}

	inline
	font::alignment_t font::get_alignment () const {
		return (alignment_t)TTF_GetFontWrappedAlign (handle ());
	}

	inline
	void font::set_alignment (font::alignment_t alignemt) {
		TTF_SetFontWrappedAlign (handle (), (int)alignemt);
	}

	inline
	int font::get_height () const {
		return TTF_FontHeight (handle ());
	}

	inline
	int font::get_ascent () const {
		return TTF_FontAscent (handle ());
	}

	inline
	int font::get_descent () const {
		return TTF_FontDescent (handle ());
	}

	inline
	int font::get_line_skip () const {
		return TTF_FontLineSkip (handle ());
	}

	inline
	bool font::get_kerning_enabled () const {
		return TTF_GetFontKerning (handle ()) != 0;
	}

	inline
	void font::set_kerning_enabled (bool v) {
		TTF_SetFontKerning (handle (), v ? 1 : 0);
	}

	inline
	std::size_t font::get_faces () const {
		return (size_t)TTF_FontFaces (handle ());
	}

	inline
	bool font::get_is_fixed_width () const {
		return TTF_FontFaceIsFixedWidth (handle ()) != 0;
	}

	inline
	std::string font::get_face_family_name () const {
		return TTF_FontFaceFamilyName (handle ());
	}

	inline
	std::string font::get_face_style_name () const {
		return TTF_FontFaceStyleName (handle ());
	}

	inline
	bool font::has_glyph (wchar_t ch) const {
		static_assert (sizeof (ch) == 4 || sizeof (ch) == 32);
		if constexpr (sizeof (ch) == 2) {
			return TTF_GlyphIsProvided (const_cast<TTF_Font*>(handle ()), (Uint16)ch);
		} else {
			return TTF_GlyphIsProvided32 (const_cast<TTF_Font*>(handle ()), (Uint32)ch);
		}
	}

	inline
	std::optional<font::metrics_t> font::get_metrics (wchar_t ch) const {
		static_assert (sizeof (ch) == 4 || sizeof (ch) == 32);

		int minx;
		int maxx;
		int miny;
		int maxy;
		int advance;
		bool rc = false;
		if constexpr (sizeof (ch) == 2) {
			rc = TTF_GlyphMetrics (const_cast<TTF_Font*>(handle ()), (Uint16)ch, &minx, &maxx, &miny, &maxy, &advance)
				 != 0;
		} else {
			rc = TTF_GlyphMetrics32 (const_cast<TTF_Font*>(handle ()), (Uint32)ch, &minx, &maxx, &miny, &maxy, &advance)
				 != 0;
		}
		if (rc) {
			return std::make_tuple (minx, maxx, miny, maxy, advance);
		}
		return std::nullopt;
	}

	inline
	std::optional<font::text_size_t> font::get_text_size (const std::wstring& str) const {
		int w;
		int h;
		if constexpr (sizeof (wchar_t) == 2) {
			if (0 != TTF_SizeUNICODE (const_cast<TTF_Font*>(handle ()), (Uint16*)str.c_str (), &w, &h)) {
				return std::make_tuple (w, h);
			}
		} else {
			std::vector<Uint16> text (str.size (), 0);
			for (std::size_t i = 0; i < str.size (); i++) {
				text[i] = (Uint16)str[i];
			}
			if (0 != TTF_SizeUNICODE (const_cast<TTF_Font*>(handle ()), text.data (), &w, &h)) {
				return std::make_tuple (w, h);
			}
		}
		return std::nullopt;
	}

	inline
	std::optional<font::text_size_t> font::get_text_size (const std::string& str) const {
		int w;
		int h;

		if (0 != TTF_SizeUTF8 (const_cast<TTF_Font*>(handle ()), str.c_str (), &w, &h)) {
			return std::make_tuple (w, h);
		}
		return std::nullopt;
	}

	inline
	font::text_size_t font::measure_text (const std::string& str, int max_width_pixels) const {
		int extent;
		int count;
		auto* f = const_cast<TTF_Font*>(handle ());
		SAFE_SDL_CALL(TTF_MeasureUTF8, f, str.c_str (), max_width_pixels, &extent, &count);
		return std::make_tuple (count, extent);
	}

	inline
	font::text_size_t font::measure_text (const std::wstring& str, int max_width_pixels) const {
		int extent;
		int count;
		auto* f = const_cast<TTF_Font*>(handle ());
		if constexpr (sizeof (wchar_t) == 2) {
			SAFE_SDL_CALL(TTF_MeasureUNICODE, f, (Uint16*)str.c_str (), max_width_pixels, &extent, &count);
		} else {
			std::vector<Uint16> text (str.size (), 0);
			for (std::size_t i = 0; i < str.size (); i++) {
				text[i] = (Uint16)str[i];
			}
			SAFE_SDL_CALL(TTF_MeasureUNICODE, f, text.data (), max_width_pixels, &extent, &count);
		}
		return std::make_tuple (count, extent);
	}

}

#endif //SDLPP_INCLUDE_SDLPP_TTF_HH_
