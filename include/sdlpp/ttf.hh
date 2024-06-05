//
// Created by igor on 5/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_TTF_HH_
#define SDLPP_INCLUDE_SDLPP_TTF_HH_

#include <tuple>
#include <type_traits>
#include <string>
#include <string_view>
#include <optional>

#include <bitflags/bitflags.hpp>
#include <sdlpp/sdl2.hh>
#include <sdlpp/object.hh>
#include <sdlpp/io.hh>
#include <bsw/macros.hh>
#include <bsw/mp/if_then_else.hh>

namespace neutrino::sdl {

	namespace detail {
		template <class T>
		struct remove_cvref {
			using type = std::remove_cv_t<std::remove_reference_t<T>>;
		};

		template <class From, class To>
		struct is_compatible
		{
			static constexpr bool value = std::is_constructible_v<From, To>
										  || std::is_convertible_v<From, To>
										  || std::is_same_v<typename remove_cvref<From>::type, To>;
		};

		template <typename T>
		struct string_traits {
			using from = std::decay_t<T>;
			static constexpr bool is_ucs = is_compatible<from, std::wstring>::value;
			static constexpr bool is_utf8 = is_compatible<from, std::string>::value;
			static constexpr bool is_string = is_ucs || is_utf8;
		};
	}

	template<typename LikeAString>
	using ensure_string = typename std::enable_if<detail::string_traits<LikeAString>::is_string>::type*;

	class ttf : public object<TTF_Font> {
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
		ttf () = default;

		ttf (const std::string& path, int ptsize);
		ttf (const std::string& path, int ptsize, unsigned int hdpi, unsigned int vdpi);

		ttf (const std::string& path, int ptsize, int index);
		ttf (const std::string& path, int ptsize, int index, unsigned int hdpi, unsigned int vdpi);

		ttf (io& rwops, int ptsize);
		ttf (io& rwops, int ptsize, unsigned int hdpi, unsigned int vdpi);
		ttf (io& rwops, int ptsize, int index);
		ttf (io& rwops, int ptsize, int index, unsigned int hdpi, unsigned int vdpi);

		ttf (object<TTF_Font>&& other);
		ttf& operator= (object<TTF_Font>&& other) noexcept;
		ttf& operator= (ttf&& other) noexcept;

		ttf& operator= (const ttf& other) = delete;
		ttf (const ttf& other) = delete;

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

		[[nodiscard]] bool get_sdf_enabled () const;
		void set_sdf_enabled (bool v);

		[[nodiscard]] std::size_t get_faces () const;
		[[nodiscard]] bool get_is_fixed_width () const;

		[[nodiscard]] std::string get_face_family_name () const;
		[[nodiscard]] std::string get_face_style_name () const;

		[[nodiscard]] bool has_glyph (wchar_t ch) const;

		// int minx, int maxx,
		// int miny, int maxy, int *advance
		using metrics_t = std::tuple<int, int, int, int, int>;
		[[nodiscard]] std::optional<metrics_t> get_metrics (wchar_t ch) const;

		// width, height in pixels
		using text_size_t = std::tuple<int, int>;

		template<typename LikeAString>
		[[nodiscard]] std::optional<text_size_t> get_text_size (LikeAString&& str,
																ensure_string<LikeAString> = nullptr) const;


		// num of chars that can be rendered, latest calculated width
		template<typename LikeAString>
		[[nodiscard]] text_size_t measure_text (LikeAString&& str, int max_width_pixels, ensure_string<LikeAString> = nullptr) const;

	};

}

// =============================================================================
// Implementation
// =============================================================================

namespace neutrino::sdl {

	inline
	ttf& ttf::operator= (object<TTF_Font>&& other) noexcept {
		object<TTF_Font>::operator= (std::move (other));
		return *this;;
	}

	inline
	ttf::ttf (object<TTF_Font>&& other)
		: object<TTF_Font> (std::move (other)) {

	}

	inline
	ttf& ttf::operator= (ttf&& other) noexcept {
		object<TTF_Font>::operator= (std::move (other));
		return *this;
	}

	inline
	ttf::ttf (const std::string& path, int ptsize)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFont, path.c_str (), ptsize), true) {

	}

	inline
	ttf::ttf (const std::string& path, int ptsize, int index)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontIndex, path.c_str (), ptsize, index), true) {
	}

	inline
	ttf::ttf (io& rwops, int ptsize)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontRW, rwops.handle (), 0, ptsize), true) {
	}

	inline
	ttf::ttf (io& rwops, int ptsize, int index)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontIndexRW, rwops.handle (), 0, ptsize, index), true) {
	}

	inline
	ttf::ttf (const std::string& path, int ptsize, unsigned int hdpi, unsigned int vdpi)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontDPI, path.c_str (), ptsize, hdpi, vdpi), true) {

	}

	inline
	ttf::ttf (const std::string& path, int ptsize, int index, unsigned int hdpi, unsigned int vdpi)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontIndexDPI, path.c_str (), ptsize, index, hdpi, vdpi), true) {

	}

	inline
	ttf::ttf (io& rwops, int ptsize, unsigned int hdpi, unsigned int vdpi)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontDPIRW, rwops.handle (), 0, ptsize, hdpi, vdpi), true) {
	}

	inline
	ttf::ttf (io& rwops, int ptsize, int index, unsigned int hdpi, unsigned int vdpi)
		: object<TTF_Font> (SAFE_SDL_CALL(TTF_OpenFontIndexDPIRW, rwops
		.handle (), 0, ptsize, index, hdpi, vdpi), true) {
	}

	inline
	void ttf::set_font_size (int ptsize) {
		SAFE_SDL_CALL(TTF_SetFontSize, handle (), ptsize);
	}

	inline
	void ttf::set_font_size (int ptsize, unsigned int hdpi, unsigned int vdpi) {
		SAFE_SDL_CALL(TTF_SetFontSizeDPI, handle (), ptsize, hdpi, vdpi);
	}

	inline
	ttf::style_t ttf::get_style () const {
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
	void ttf::set_style (ttf::style_t style) {
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
	int ttf::get_outline () const {
		return TTF_GetFontOutline (handle ());
	}

	inline
	void ttf::set_outline (int outline) {
		TTF_SetFontOutline (handle (), outline);
	}

	inline
	ttf::hinting_t ttf::get_hinting () const {
		return (hinting_t)TTF_GetFontHinting (handle ());
	}

	inline
	void ttf::set_hiniting (ttf::hinting_t hinting) {
		TTF_SetFontHinting (handle (), (int)hinting);
	}

	inline
	ttf::alignment_t ttf::get_alignment () const {
		return (alignment_t)TTF_GetFontWrappedAlign (handle ());
	}

	inline
	void ttf::set_alignment (ttf::alignment_t alignemt) {
		TTF_SetFontWrappedAlign (handle (), (int)alignemt);
	}

	inline
	int ttf::get_height () const {
		return TTF_FontHeight (handle ());
	}

	inline
	int ttf::get_ascent () const {
		return TTF_FontAscent (handle ());
	}

	inline
	int ttf::get_descent () const {
		return TTF_FontDescent (handle ());
	}

	inline
	int ttf::get_line_skip () const {
		return TTF_FontLineSkip (handle ());
	}

	inline
	bool ttf::get_kerning_enabled () const {
		return TTF_GetFontKerning (handle ()) != 0;
	}

	inline
	void ttf::set_kerning_enabled (bool v) {
		TTF_SetFontKerning (handle (), v ? 1 : 0);
	}

	inline
	bool ttf::get_sdf_enabled () const {
		return TTF_GetFontSDF (handle ()) == SDL_TRUE;
	}

	inline
	void ttf::set_sdf_enabled (bool v) {
		TTF_SetFontSDF (handle (), v ? SDL_TRUE : SDL_FALSE);
	}

	inline
	std::size_t ttf::get_faces () const {
		return (size_t)TTF_FontFaces (handle ());
	}

	inline
	bool ttf::get_is_fixed_width () const {
		return TTF_FontFaceIsFixedWidth (handle ()) != 0;
	}

	inline
	std::string ttf::get_face_family_name () const {
		return TTF_FontFaceFamilyName (handle ());
	}

	inline
	std::string ttf::get_face_style_name () const {
		return TTF_FontFaceStyleName (handle ());
	}

	inline
	bool ttf::has_glyph (wchar_t ch) const {
		static_assert (sizeof (ch) == 4 || sizeof (ch) == 32);
		if constexpr (sizeof (ch) == 2) {
			return TTF_GlyphIsProvided (const_cast<TTF_Font*>(handle ()), (Uint16)ch);
		} else {
			return TTF_GlyphIsProvided32 (const_cast<TTF_Font*>(handle ()), (Uint32)ch);
		}
	}

	inline
	std::optional<ttf::metrics_t> ttf::get_metrics (wchar_t ch) const {
		static_assert (sizeof (ch) == 4 || sizeof (ch) == 2);

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

	namespace detail {

		template <bool wchar_is_4_bytes>
		struct conv_buffer_t : public std::vector<Uint16> {
			void init (const std::wstring& str) {
				resize (str.size ());
			}

			void init (const std::wstring_view& str) {
				resize (str.size ());
			}

			void init (const wchar_t* str) {
				if (!str) {
					resize (0);
				} else {
					resize (wcslen (str));
				}
			}

			void put (std::size_t i, wchar_t x) {
				(*this)[i] = (Uint16)(x & 0xFFFF);
			}

			[[nodiscard]] const Uint16* text () const {
				return data ();
			}
		};

		template <>
		struct conv_buffer_t<false> {
			const wchar_t* m_ptr;
			std::size_t n;

			conv_buffer_t ()
				: m_ptr (nullptr), n (0) {}

			void init (const std::wstring& str) {
				m_ptr = str.c_str ();
				n = str.size ();
			}

			void init (const wchar_t* str) {
				m_ptr = str;
				n = wcslen (str);
			}

			void init (const std::wstring_view& str) {
				m_ptr = str.data();
				n = str.size();
			}

			void put ([[maybe_unused]] std::size_t i, [[maybe_unused]] wchar_t x) {
			}

			[[nodiscard]] constexpr size_t size () const noexcept {
				return n;
			}

			[[nodiscard]] const Uint16* text () const {
				return reinterpret_cast<const Uint16*>(m_ptr);
			}
		};

		using conv_buffer = conv_buffer_t<sizeof (wchar_t) == 4>;

		std::vector<Uint16> ucs4_to_ucs2 (const std::wstring& str) {
			static_assert (sizeof (std::wstring::value_type) == 4);
			std::vector<Uint16> text (str.size (), 0);
			for (std::size_t i = 0; i < str.size (); i++) {
				text[i] = (Uint16)(str[i] & 0xFFFF);
			}
			return text;
		}




		template <typename What, typename ... Args>
		struct is_present {
			static constexpr bool value{(is_compatible<typename remove_cvref<Args>::type, What>::value || ...)};
		};



		template <typename T>
		constexpr T proxy (T arg,
						   [[maybe_unused]] conv_buffer& buff,
						   typename std::enable_if<!string_traits<T>::is_string>::type* = nullptr) {
			return arg;
		}

		template <typename T>
		const Uint16* proxy (T arg,
							 conv_buffer& buff,
							 typename std::enable_if<string_traits<T>::is_ucs>::type* = nullptr) {
			buff.init (arg);
			for (std::size_t i = 0; i < buff.size (); i++) {
				buff.put (i, arg[i]);
			}
			return buff.text ();
		}

		template <typename T>
		const char* proxy (T arg,
						   [[maybe_unused]] conv_buffer& buff,
						   typename std::enable_if<string_traits<T>::is_utf8>::type* = nullptr) {
			if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
				return arg.c_str ();
			} else if constexpr (std::is_same_v<std::decay_t<T>, std::string_view>) {
				return arg.data ();
			} else {
				return arg;
			}
		}


#define d_TTF_CALLER_PROXY(NAME, UTF8_FN, UNICODE_FN)                                                                \
        struct NAME {                                                                                                \
            static constexpr auto utf8_fn = UTF8_FN;                                                                 \
            static constexpr auto ucs_fn = UNICODE_FN;                                                               \
                                                                                                                     \
            template <typename ...Args>                                                                              \
            static auto utf8_call(Args&& ...args) {                                                                  \
				conv_buffer cvt;  																					 \
                return utf8_fn(proxy(std::forward<Args&&>(args), cvt)...);                                           \
            }                                                                                                        \
            template <typename ...Args>                                                                              \
            static auto ucs_call(Args&& ...args) {                                                                   \
                conv_buffer cvt;                                                                                     \
                return ucs_fn(proxy(std::forward<Args&&>(args), cvt)...);                                            \
            }                                                                                                        \
                                                                                                                     \
            template <typename ...Args>                                                                              \
            static auto call(Args&& ...args) {                                                                       \
                if constexpr (is_present<std::wstring, Args...>::value) {                                            \
                    return ucs_call (std::forward<Args&&>(args)...);                                                 \
                } else {                                                                                             \
                    return utf8_call (std::forward<Args&&>(args)...);                                                \
                }                                                                                                    \
            }                                                                                                        \
        }

#define d_TTF_CALL(Name) \
d_TTF_CALLER_PROXY(PPCAT(PPCAT(TTF_, Name), _impl), PPCAT(PPCAT(TTF_, Name), UTF8), PPCAT(PPCAT(TTF_, Name), UNICODE))

		d_TTF_CALL(Size);

		d_TTF_CALL(Measure);
	} // ns detail

	template<typename LikeAString>
	[[nodiscard]] std::optional<ttf::text_size_t> ttf::get_text_size (LikeAString&& str,
																	  ensure_string<LikeAString>) const {
		int w;
		int h;
		if (0 != detail::TTF_Size_impl::call(const_cast<TTF_Font*>(handle ()), std::forward<LikeAString&&>(str), &w, &h)) {
			return std::make_tuple (w, h);
		}
		return std::nullopt;
	}

	template<typename LikeAString>
	[[nodiscard]] ttf::text_size_t ttf::measure_text (LikeAString&& str, int max_width_pixels, ensure_string<LikeAString>) const {
		int extent;
		int count;
		auto* f = const_cast<TTF_Font*>(handle ());
		detail::TTF_Measure_impl::call (f, std::forward<LikeAString&&>(str), max_width_pixels, &extent, &count);
		return std::make_tuple (count, extent);
	}
}

#endif //SDLPP_INCLUDE_SDLPP_TTF_HH_
