//
// Created by igor on 6/6/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_
#define SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_

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
		SET = RW_SEEK_SET,  /**< Seek from the beginning of data */
		CUR = RW_SEEK_CUR, /**< Seek relative to current read point */
		END = RW_SEEK_END  /**< Seek relative to the end of data */
	};

	d_SDLPP_OSTREAM(whence);
}
#endif //SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_
