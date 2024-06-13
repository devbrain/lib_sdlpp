//
// Created by igor on 6/11/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_VIDEO_CURSOR_HH_
#define SDLPP_INCLUDE_SDLPP_VIDEO_CURSOR_HH_

#include <cstdint>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <sdlpp/video/geometry.hh>

namespace neutrino::sdl {
	enum class system_cursor {
		ARROW = SDL_SYSTEM_CURSOR_ARROW,
		IBEAM = SDL_SYSTEM_CURSOR_IBEAM,
		WAIT = SDL_SYSTEM_CURSOR_WAIT,
		CROSSHAIR = SDL_SYSTEM_CURSOR_CROSSHAIR,
		WAIT_ARROW = SDL_SYSTEM_CURSOR_WAITARROW,
		SIZE_NW_SE = SDL_SYSTEM_CURSOR_SIZENWSE,
		SIZE_NE_SW = SDL_SYSTEM_CURSOR_SIZENESW,
		SIZE_WE = SDL_SYSTEM_CURSOR_SIZEWE,
		SIZE_NS = SDL_SYSTEM_CURSOR_SIZENS,
		SIZE_ALL = SDL_SYSTEM_CURSOR_SIZEALL,
		NO = SDL_SYSTEM_CURSOR_NO,
		HAND = SDL_SYSTEM_CURSOR_HAND
	};

	d_SDLPP_OSTREAM(system_cursor);

	class cursor : public object<SDL_Cursor> {
	 public:
		cursor ();
		explicit cursor (object<SDL_Cursor >&& other);
		cursor (cursor&& other) noexcept;
		cursor& operator= (object<SDL_Cursor>&& other) noexcept;
		cursor& operator= (cursor&& other) noexcept;

		cursor(const object<SDL_Surface>& s, unsigned hot_x, unsigned hot_y);
		cursor(const object<SDL_Surface>& s, const point& hot_p);
		explicit cursor(system_cursor sc);

		cursor(const uint8_t* data, const uint8_t* mask, std::size_t w, std::size_t h, unsigned hot_x, unsigned hot_y);
		cursor(const uint8_t* data, const uint8_t* mask, const area_type& dims, const point& hot_p);

		static cursor get_default_cursor() {
			return cursor{object<SDL_Cursor>(SAFE_SDL_CALL(SDL_GetDefaultCursor), true) };
		}
	};
}

namespace neutrino::sdl {

	inline
	cursor::cursor () = default;

	// ----------------------------------------------------------------------------------------------
	inline
	cursor::cursor (object<SDL_Cursor>&& other)
		: object<SDL_Cursor> (std::move (other)) {

	}

	// ----------------------------------------------------------------------------------------------
	inline
	cursor::cursor (cursor&& other) noexcept
		: object<SDL_Cursor> (nullptr, false) {
		other.swap (*this);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	cursor& cursor::operator= (cursor&& other) noexcept {
		object<SDL_Cursor>::operator= (std::move (other));
		return *this;
	}

	// ----------------------------------------------------------------------------------------------
	inline
	cursor& cursor::operator= (object<SDL_Cursor>&& other) noexcept {
		object<SDL_Cursor>::operator= (std::move (other));
		return *this;
	}

	inline
	cursor::cursor (const object<SDL_Surface>& s, unsigned hot_x, unsigned hot_y)
	: object<SDL_Cursor>(SAFE_SDL_CALL(SDL_CreateColorCursor, s.const_handle(), static_cast<int>(hot_x), static_cast<int>(hot_y)), true){

	}

	inline
	cursor::cursor (const object<SDL_Surface>& s, const point& hot_p)
		: object<SDL_Cursor>(SAFE_SDL_CALL(SDL_CreateColorCursor, s.const_handle(), hot_p.x, hot_p.y), true){

	}

	inline
	cursor::cursor (system_cursor sc)
		: object<SDL_Cursor>(SAFE_SDL_CALL(SDL_CreateSystemCursor, static_cast<SDL_SystemCursor>(sc)), true) {

	}

	inline
	cursor::cursor (const uint8_t* data, const uint8_t* mask, std::size_t w, std::size_t h, unsigned int hot_x, unsigned int hot_y)
		: object<SDL_Cursor>(SAFE_SDL_CALL(SDL_CreateCursor, data, mask, w, h, hot_x, hot_y), true) {
	}

	inline
	cursor::cursor (const uint8_t* data, const uint8_t* mask, const area_type& dims, const point& hot_p)
		: object<SDL_Cursor>(SAFE_SDL_CALL(SDL_CreateCursor, data, mask, dims.w, dims.h, hot_p.x, hot_p.y), true) {

	}

}

#endif //SDLPP_INCLUDE_SDLPP_VIDEO_CURSOR_HH_
