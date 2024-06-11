//
// Created by igor on 6/11/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DETAIL_SDL_STRING_HH_
#define SDLPP_INCLUDE_SDLPP_DETAIL_SDL_STRING_HH_

#include <string>
#include <memory>
#include <sdlpp/detail/sdl2.hh>

namespace neutrino::sdl::detail {
	class sdl_string {
	 public:
		using text_ptr_t = const char*;
		sdl_string(text_ptr_t&& text)
		: m_text(const_cast<char *>(text)) {}

		explicit operator bool() const noexcept { return m_text != nullptr; }

		const char* relese() noexcept {
			return m_text.release();
		}

		[[nodiscard]] std::string copy() const {
			return {m_text.get()};
		}

		[[nodiscard]] const char* get() const noexcept {
			return m_text.get();
		}
	 private:
		struct sdl_string_deleter {
			void operator()(void* ptr) noexcept { SDL_free(ptr); }
		};
		std::unique_ptr<char, sdl_string_deleter> m_text;
	};
}

#endif //SDLPP_INCLUDE_SDLPP_DETAIL_SDL_STRING_HH_
