//
// Created by igor on 6/11/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_CLIPBOARD_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_CLIPBOARD_HH_

#include <string>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/sdl_string.hh>

namespace neutrino::sdl {
	inline
	void set_clipboard(const char* text) {
		ENFORCE(text);
		SAFE_SDL_CALL(SDL_SetClipboardText, text);
	}

	inline
	void set_clipboard(const std::string& text) {
		SAFE_SDL_CALL(SDL_SetClipboardText, text.c_str());
	}

	inline
	bool has_clipboard() {
		return SDL_HasClipboardText() == SDL_TRUE;
	}

	inline
	std::string get_clipboard() {
		const char* text = SDL_GetClipboardText();
		ENFORCE(text);
		const detail::sdl_string s(std::move(text));
		return s.copy();
	}
}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_CLIPBOARD_HH_
