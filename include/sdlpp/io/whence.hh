//
// Created by igor on 6/6/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_
#define SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_
#include <array>
#include <type_traits>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	/**
	 * @enum whence
	 *
	 * @brief Enumerator representing different seek positions in a data stream.
	 *
	 * The whence enum class is used to represent various seek positions in a data stream.
	 * It provides three options which are SET, CUR, and END. These options define the position
	 * from where the stream should be seeked, i.e., whether from the beginning of the data,
	 * relative to the current read point, or relative to the end of the data.
	 */
	enum class whence : int {
		SET = RW_SEEK_SET, /**< Seek from the beginning of data */
		CUR = RW_SEEK_CUR, /**< Seek relative to current read point */
		END = RW_SEEK_END /**< Seek relative to the end of data */
	};

	namespace detail {
		static inline constexpr std::array <whence, 3> s_vals_of_whence = {
			whence::SET,
			whence::CUR,
			whence::END,
		};
	}

	template<typename T>
	static constexpr const decltype(detail::s_vals_of_whence)&
	values(std::enable_if_t <std::is_same_v <whence, T>>* = nullptr) {
		return detail::s_vals_of_whence;
	}

	template<typename T>
	static constexpr auto
	begin(std::enable_if_t <std::is_same_v <whence, T>>* = nullptr) {
		return detail::s_vals_of_whence.begin();
	}

	template<typename T>
	static constexpr auto
	end(std::enable_if_t <std::is_same_v <whence, T>>* = nullptr) {
		return detail::s_vals_of_whence.end();
	}

	d_SDLPP_OSTREAM(whence);
}
#endif //SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_
