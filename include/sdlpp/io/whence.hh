//
// Created by igor on 6/6/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_
#define SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_

#include <sdlpp/detail/sdl2.hh>

namespace neutrino::sdl {
	enum class whence : int {
		SET = RW_SEEK_SET,  /**< Seek from the beginning of data */
		CUR = RW_SEEK_CUR, /**< Seek relative to current read point */
		END = RW_SEEK_END  /**< Seek relative to the end of data */
	};
}
#endif //SDLPP_INCLUDE_SDLPP_IO_WHENCE_HH_
