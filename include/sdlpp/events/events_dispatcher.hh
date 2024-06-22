//
// Created by igor on 05/06/2020.
//

#ifndef NEUTRINO_SDL_EVENTS_DISPATCHER_HH
#define NEUTRINO_SDL_EVENTS_DISPATCHER_HH

#include <variant>

#include <sdlpp/events/system_events.hh>

namespace neutrino::sdl {
	inline
	events::event_t map_event(const SDL_Event& e) {
		switch (e.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				return events::keyboard(e);

			case SDL_WINDOWEVENT:
				switch (e.window.event) {
					case SDL_WINDOWEVENT_SHOWN: return events::window_shown(e);
					case SDL_WINDOWEVENT_HIDDEN: return events::window_hidden(e);
					case SDL_WINDOWEVENT_EXPOSED: return events::window_exposed(e);
					case SDL_WINDOWEVENT_MOVED: return events::window_moved(e);
					case SDL_WINDOWEVENT_RESIZED: return events::window_resized(e);
					case SDL_WINDOWEVENT_MINIMIZED: return events::window_minimized(e);
					case SDL_WINDOWEVENT_MAXIMIZED: return events::window_maximized(e);
					case SDL_WINDOWEVENT_RESTORED: return events::window_restored(e);
					case SDL_WINDOWEVENT_ENTER: return events::window_mouse_entered(e);
					case SDL_WINDOWEVENT_LEAVE: return events::window_mouse_leaved(e);
					case SDL_WINDOWEVENT_FOCUS_GAINED: return events::window_focus_gained(e);
					case SDL_WINDOWEVENT_FOCUS_LOST: return events::window_focus_lost(e);
					case SDL_WINDOWEVENT_CLOSE: return events::window_close(e);
					default:
						return {};
				}
			case SDL_TEXTEDITING: return events::text_editing(e);
			case SDL_TEXTINPUT: return events::text_input(e);

			case SDL_MOUSEMOTION:
				if (e.motion.which != SDL_TOUCH_MOUSEID) {
					return events::mouse_motion(e);
				}
				return events::touch_device_motion(e);

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (e.button.which != SDL_TOUCH_MOUSEID) {
					return events::mouse_button(e);
				}
				return events::touch_device_button(e);
			case SDL_MOUSEWHEEL:
				if (e.wheel.which != SDL_TOUCH_MOUSEID) {
					return events::mouse_wheel(e);
				}
				return events::touch_device_wheel(e);

			case SDL_JOYAXISMOTION: return events::joystick_axis(e);
			case SDL_JOYBALLMOTION: return events::joystick_ball(e);
			case SDL_JOYHATMOTION: return events::joystick_hat(e);
			case SDL_USEREVENT: return events::user(e);

			case SDL_JOYBUTTONUP:
			case SDL_JOYBUTTONDOWN:
				return events::joystick_button(e);

			case SDL_APP_TERMINATING: return events::terminating(e);
			case SDL_APP_LOWMEMORY: return events::low_memory(e);
			case SDL_APP_WILLENTERBACKGROUND: return events::will_enter_background(e);
			case SDL_APP_DIDENTERBACKGROUND: return events::in_background(e);
			case SDL_APP_WILLENTERFOREGROUND: return events::will_enter_foreground(e);
			case SDL_APP_DIDENTERFOREGROUND: return events::in_foreground(e);
			case SDL_QUIT: return events::quit(e);

			default:
				return {};
		}

		//return {};
	}
}

#endif
