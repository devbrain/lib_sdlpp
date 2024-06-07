//
// Created by igor on 6/7/24.
//

#include <sdlpp/events/system_events.hh>
#include "utils/switch_ostream.hh"

#define d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(NAME) \
	BEGIN_CLASS_OUTPUT(NAME)					\
		MEMBER(window_id);						\
	END_CLASS_OUTPUT

#define d_DEFINE_NO_MEMBERS_EVENT_OUT(NAME)		\
	BEGIN_CLASS_OUTPUT(NAME)					\
	END_CLASS_OUTPUT

namespace neutrino::sdl::events {
	BEGIN_IMPL_OUTPUT(mousebutton)
			case mousebutton::LEFT: return "LEFT";
			case mousebutton::RIGHT: return "RIGHT";
			case mousebutton::MIDDLE: return "MIDDLE";
			case mousebutton::X1: return "X1";
			case mousebutton::X2: return "X2";
	END_IMPL_OUTPUT(mousebutton)

	BEGIN_IMPL_OUTPUT(joystick_hat_state)
			case joystick_hat_state::LEFTUP: return "LEFTUP";
			case joystick_hat_state::HAT_UP: return "HAT_UP";
			case joystick_hat_state::HAT_RIGHTUP: return "HAT_RIGHTUP";
			case joystick_hat_state::HAT_LEFT: return "HAT_LEFT";
			case joystick_hat_state::HAT_CENTERED: return "HAT_CENTERED";
			case joystick_hat_state::HAT_RIGHT: return "HAT_RIGHT";
			case joystick_hat_state::HAT_LEFTDOWN: return "HAT_LEFTDOWN";
			case joystick_hat_state::HAT_DOWN: return "HAT_DOWN";
			case joystick_hat_state::HAT_RIGHTDOWN: return "HAT_RIGHTDOWN";
	END_IMPL_OUTPUT(joystick_hat_state)

	BEGIN_CLASS_OUTPUT(keyboard)
		MEMBER(window_id);
		MEMBER(pressed);
		MEMBER(repeat);
		MEMBER(scan_code);
		MEMBER(key_code);
		MEMBER_T(key_mod, keymod);
	END_CLASS_OUTPUT

	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_shown)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_hidden)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_exposed)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_minimized)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_maximized)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_restored)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_mouse_entered)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_mouse_leaved)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_focus_gained)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_focus_lost)
	d_DEFINE_NO_MEMBERS_EVENT_WIN_OUT(window_close)

	d_DEFINE_NO_MEMBERS_EVENT_OUT(terminating);
	d_DEFINE_NO_MEMBERS_EVENT_OUT(low_memory);
	d_DEFINE_NO_MEMBERS_EVENT_OUT(will_enter_background);
	d_DEFINE_NO_MEMBERS_EVENT_OUT(in_background);
	d_DEFINE_NO_MEMBERS_EVENT_OUT(will_enter_foreground);
	d_DEFINE_NO_MEMBERS_EVENT_OUT(in_foreground);
	d_DEFINE_NO_MEMBERS_EVENT_OUT(quit);

}