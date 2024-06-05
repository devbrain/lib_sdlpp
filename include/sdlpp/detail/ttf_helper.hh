//
// Created by igor on 6/5/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DETAIL_TTF_HELPER_HH_
#define SDLPP_INCLUDE_SDLPP_DETAIL_TTF_HELPER_HH_

#include <type_traits>
#include <string>
#include <string_view>
#include <vector>
#include <sdlpp/sdl2.hh>
#include <bsw/macros.hh>

namespace neutrino::sdl {
	namespace detail {
		template <class T>
		struct remove_cvref {
			using type = std::remove_cv_t<std::remove_reference_t<T>>;
		};

		template <class From, class To>
		struct is_compatible {
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

		template <typename Ch>
		struct is_char {
			static constexpr bool value =
				std::is_same_v<Ch, char> || std::is_same_v<Ch, wchar_t> || std::is_same_v<Ch, Uint32>;
		};

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
				m_ptr = str.data ();
				n = str.size ();
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


		inline
		constexpr Uint16 proxy(char arg) {
			return ((Uint16)arg) & 0xFF;
		}

		template <typename Ch>
		inline
		constexpr Uint16 proxy(Ch arg, typename std::enable_if<sizeof(Ch)==2 && std::is_same_v<Ch, wchar_t>>::type* = nullptr) {
			return (Uint16)arg;
		}

		template <typename Ch>
		inline
		constexpr Uint32 proxy(Ch arg, typename std::enable_if<sizeof(Ch)==4 && std::is_same_v<Ch, wchar_t>>::type* = nullptr) {
			return (Uint32)arg;
		}


#define d_TTF_CALLER_PROXY(NAME, UTF8_FN, UNICODE_FN)                                                                \
        struct NAME {                                                                                                \
            static constexpr auto utf8_fn = UTF8_FN;                                                                 \
            static constexpr auto ucs_fn = UNICODE_FN;                                                               \
                                                                                                                     \
            template <typename ...Args>                                                                              \
            static auto utf8_call(Args&& ...args) {                                                                  \
                conv_buffer cvt;                                                                                     \
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

#define d_TTF_CALL2(Name, Suffix)                                                        \
    d_TTF_CALLER_PROXY(PPCAT(PPCAT(PPCAT(TTF_, Name), Suffix), _impl),                    \
                       PPCAT(PPCAT(PPCAT(TTF_, Name), UTF8_), Suffix),                \
                       PPCAT(PPCAT(PPCAT(TTF_, Name), UNICODE_), Suffix))

		d_TTF_CALL(Size);

		d_TTF_CALL(Measure);

		d_TTF_CALL2(Render, Solid);

		d_TTF_CALL2(Render, Solid_Wrapped);

		d_TTF_CALL2(Render, Shaded);

		d_TTF_CALL2(Render, Shaded_Wrapped);

		d_TTF_CALL2(Render, Blended);

		d_TTF_CALL2(Render, Blended_Wrapped);

		d_TTF_CALL2(Render, LCD);

		d_TTF_CALL2(Render, LCD_Wrapped);

	} // ns detail
	template <typename LikeAString>
	using ensure_string = typename std::enable_if<detail::string_traits<LikeAString>::is_string>::type*;

	template <typename LikeAChar>
	using ensure_char = typename std::enable_if<detail::is_char<LikeAChar>::value>::type*;
}

#endif //SDLPP_INCLUDE_SDLPP_DETAIL_TTF_HELPER_HH_
