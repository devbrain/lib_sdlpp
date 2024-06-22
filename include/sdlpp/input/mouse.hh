//
// Created by igor on 6/11/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_MOUSE_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_MOUSE_HH_

#include <optional>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/geometry.hh>
#include <sdlpp/events/system_events.hh>

namespace neutrino::sdl {
	class mouse {
		public:
			static std::optional <window> get_focus() noexcept {
				if (SDL_Window* w = SDL_GetMouseFocus()) {
					return window{object <SDL_Window>(w, false)};
				}
				return std::nullopt;
			}

			static void capture(bool enable) {
				SAFE_SDL_CALL(SDL_CaptureMouse, (enable ? SDL_TRUE : SDL_FALSE));
			}

			static bool is_relative_mode_enable() noexcept {
				return SDL_GetRelativeMouseMode() == SDL_TRUE;
			}

			static void relative_mode(bool enable) {
				SAFE_SDL_CALL(SDL_SetRelativeMouseMode, (enable ? SDL_TRUE : SDL_FALSE));
			}

			static void move(unsigned x, unsigned y) {
				SAFE_SDL_CALL(SDL_WarpMouseGlobal, static_cast<int>(x), static_cast<int>(y));
			}

			static void move(const point& p) {
				SAFE_SDL_CALL(SDL_WarpMouseGlobal, p.x, p.y);
			}

			static void move(const object <SDL_Window>& w, unsigned x, unsigned y) {
				SAFE_SDL_CALL(SDL_WarpMouseInWindow, w.const_handle(), static_cast<int>(x), static_cast<int>(y));
			}

			static void move(const object <SDL_Window>& w, const point& p) {
				SAFE_SDL_CALL(SDL_WarpMouseInWindow, w.const_handle(), p.x, p.y);
			}

			static void show_cursor(bool enable) {
				auto rc = SDL_ShowCursor(enable ? SDL_ENABLE : SDL_DISABLE);
				ENFORCE(rc >= 0);
			}

			static bool is_cursor_hidden() {
				const auto rc = SDL_ShowCursor(SDL_QUERY);
				ENFORCE(rc >= 0);
				return rc == SDL_DISABLE;
			}

			static void set_cursor(const object <SDL_Cursor>& cur) {
				SDL_SetCursor(cur.const_handle());
			}

			static void redraw_cursor() {
				SDL_SetCursor(nullptr);
			}

			static object <SDL_Cursor> get_active_cursor() {
				return {SAFE_SDL_CALL(SDL_GetCursor), false};
			}

			static events::mouse_button get_state(bool global = false) {
				SDL_Event ev;
				ev.common.timestamp = SDL_GetTicks();
				ev.button.button = global
					                   ? SDL_GetGlobalMouseState(&ev.button.x, &ev.button.y)
					                   : SDL_GetMouseState(&ev.button.x, &ev.button.y);
				ev.button.state == (ev.button.button != 0 ? SDL_PRESSED : SDL_RELEASED);
				return events::mouse_button(ev);
			}

			static bool is_haptic() {
				return SDL_MouseIsHaptic() == SDL_TRUE;
			}

			static object <SDL_Haptic> as_haptic() {
				return {SAFE_SDL_CALL(SDL_HapticOpenFromMouse), true};
			}
	};
}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_MOUSE_HH_
