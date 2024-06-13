//
// Created by igor on 6/11/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_KEYBOARD_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_KEYBOARD_HH_

#include <cstdint>
#include <optional>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/events/event_types.hh>
#include <sdlpp/video/window.hh>

namespace neutrino::sdl {
	class keyboard;

	class keyboard_state {
		friend class keyboard;
	 public:
		bool operator [](scancode sc) const {
			return m_kbd_state[static_cast<int>(sc)] != 0;
		}
		bool operator ()(scancode sc) const {
			return m_kbd_state[static_cast<int>(sc)] != 0;
		}

		template <typename ... Args,
			typename std::enable_if_t<std::conjunction_v<std::is_same<Args, keymod>...>, void*> = nullptr>
		bool operator() (Args... flags) const {
			uint16_t f = (static_cast<std::uint16_t>(flags) | ... | 0u);
			return (SDL_GetModState() & f) == f;
		}

	 private:
		keyboard_state() : m_num_keys(0),
						   m_kbd_state(SDL_GetKeyboardState (&m_num_keys)) {
		}
	 private:
		int m_num_keys;
		const uint8_t* m_kbd_state;
	};

	class keyboard {
	 public:
		static const keyboard_state& get_keyboard_state() noexcept{
			static keyboard_state ks;
			return ks;
		}

		static void reset() {
			SDL_ResetKeyboard();
		}

		template <typename ... Args,
			typename std::enable_if_t<std::conjunction_v<std::is_same<Args, keymod>...>, void*> = nullptr>
		void set_keymod(Args... flags) const {
			uint16_t f = (static_cast<std::uint16_t>(flags) | ... | 0u);
			SDL_SetModState (static_cast<SDL_Keymod>(f));
		}

		static std::optional<window> focus()  noexcept {
			SDL_Window* w = SDL_GetKeyboardFocus();
			if (w) {
				return window{object<SDL_Window>(w, false) };
			}
			return std::nullopt;
		}

		static bool has_screen_keyboard()  noexcept {
			return SDL_HasScreenKeyboardSupport() == SDL_TRUE;
		}

		static bool is_screen_keyboard_shown(const object<SDL_Window>& w)  noexcept {
			return SDL_IsScreenKeyboardShown (w.const_handle()) == SDL_TRUE;
		}

		static bool is_text_input_active()  noexcept {
			return SDL_IsTextInputActive() == SDL_TRUE;
		}

		static bool is_text_input_shown() noexcept {
			return SDL_IsTextInputShown() == SDL_TRUE;
		}

		static void set_text_input_rec(const rect& r) noexcept{
			SDL_SetTextInputRect (&r);
		}

		static void start_text_input() noexcept{
			SDL_StartTextInput();
		}

		static void stop_text_input() noexcept{
			SDL_StopTextInput();
		}
	};
}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_KEYBOARD_HH_
