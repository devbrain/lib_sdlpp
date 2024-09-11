//
// Created by igor on 6/8/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DETAIL_OSTREAMOPS_HH_
#define SDLPP_INCLUDE_SDLPP_DETAIL_OSTREAMOPS_HH_

#include <type_traits>
#include <optional>
#include <string>
#include <iosfwd>
#include <sdlpp/export_defines.h>
#include <bsw/exception.hh>
#include <bsw/macros.hh>
#include <bsw/s11n/detail/s11n_convert.hh>

#define d_SDLPP_OSTREAM(TYPE) 																								\
	SDLPP_EXPORT std::string to_string(TYPE t); 																			\
    template <typename T>																									\
	inline																													\
	std::optional<T> from_string(const std::string& x, typename std::enable_if<std::is_same_v<T, TYPE>>::type* = nullptr) {	\
		for (const auto t : values<T>()) {																					\
			if (to_string (t) == x) {																						\
				return t;																									\
			}																												\
		}																													\
		return std::nullopt;																								\
	}                                     																					\
	SDLPP_EXPORT std::ostream& operator << (std::ostream& os, TYPE t)

#define d_SDLPP_S11N(TYPE)																							\
	namespace bsw::s11n {																							\
		template<>																									\
		struct s11n_converter <neutrino::sdl::TYPE> {																\
			static auto from_string(const std::string& s) {															\
				auto v = neutrino::sdl::from_string <neutrino::sdl::TYPE>(s);										\
				if (!v) {																							\
					RAISE_EX("Failed to deserialize " STRINGIZE(TYPE));												\
				}																									\
				return v.value();																					\
			}																										\
																													\
			static std::string to_string(const neutrino::sdl::TYPE& v) {											\
				return neutrino::sdl::to_string(v);																	\
			}																										\
		};																											\
	}


#define d_SDLPP_OSTREAM_WITHOT_FROM_STRING(TYPE) 															\
	SDLPP_EXPORT std::string to_string(TYPE t); 															\
	SDLPP_EXPORT std::ostream& operator << (std::ostream& os, TYPE t)

#endif //SDLPP_INCLUDE_SDLPP_DETAIL_OSTREAMOPS_HH_
