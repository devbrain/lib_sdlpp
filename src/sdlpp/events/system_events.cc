//
// Created by igor on 6/7/24.
//

#include <array>
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

	std::string to_string (mousebutton t) {
		static std::array<mousebutton , 5> mask = {
			mousebutton ::LEFT,
			mousebutton ::RIGHT,
			mousebutton ::MIDDLE,
			mousebutton ::X1,
			mousebutton ::X2,
		};

		static std::array<std::string_view, 5> names = {
			mousebutton ::LEFT.name,
			mousebutton ::RIGHT.name,
			mousebutton ::MIDDLE.name,
			mousebutton ::X1.name,
			mousebutton ::X2.name,
		};
		std::string out;
		auto v = (uint16_t)t;
		bool first = true;
		for (auto i = 0u; i < mask.size (); i++) {
			if ((v & mask[i]) == mask[i]) {
				if (first) {
					first = false;
				} else {
					out += '|';
				}
				out += names[i];
			}
		}
		return out;
	}

	std::ostream& operator<< (std::ostream& os, mousebutton t) {
		os << to_string (t);
		return os;
	}

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

	BEGIN_CLASS_OUTPUT(window_moved)
		MEMBER(window_id);
		MEMBER(x);
		MEMBER(y);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(window_resized)
		MEMBER(window_id);
		MEMBER(w);
		MEMBER(h);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(text_editing)
		MEMBER(window_id);
		os << "\ttext:" << ((t.text != nullptr) ? "PRESENT" : "ABSENT") << '\n';
		MEMBER(start);
		MEMBER(length);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(text_input)
		MEMBER(window_id);
		os << "\ttext:" << ((t.text != nullptr) ? "PRESENT" : "ABSENT") << '\n';
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(mouse_motion)
		MEMBER(window_id);
		MEMBER_T(state, mousebutton);
		MEMBER(x);
		MEMBER(y);
		MEMBER(xrel);
		MEMBER(yrel);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(touch_device_motion)
		MEMBER(window_id);
		MEMBER_T(button, mousebutton);
		MEMBER(x);
		MEMBER(y);
		MEMBER(xrel);
		MEMBER(yrel);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(mouse_button)
		MEMBER(window_id);
		MEMBER_T(button, mousebutton);
		MEMBER(x);
		MEMBER(y);
		MEMBER(pressed);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(touch_device_button)
		MEMBER(window_id);
		MEMBER_T(button, mousebutton);
		MEMBER(x);
		MEMBER(y);
		MEMBER(pressed);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(mouse_wheel)
		MEMBER(window_id);
		MEMBER(mouse_id);
		MEMBER(x);
		MEMBER(y);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(touch_device_wheel)
		MEMBER(window_id);
		MEMBER(x);
		MEMBER(y);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(joystick_axis)
		MEMBER(joystick);
		MEMBER(axis);
		MEMBER(value);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(joystick_ball)
		MEMBER(joystick);
		MEMBER_U8(ball);
		MEMBER(xrel);
		MEMBER(yrel);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(joystick_button)
		MEMBER(joystick);
		MEMBER_U8(button);
		MEMBER(pressed);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(joystick_hat)
		MEMBER(joystick);
		MEMBER_U8(value);
		MEMBER(state);
	END_CLASS_OUTPUT

	BEGIN_CLASS_OUTPUT(user)
		MEMBER(code);
		MEMBER(data1);
		MEMBER(data2);
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

	d_DEFINE_NO_MEMBERS_EVENT_OUT(terminating)
	d_DEFINE_NO_MEMBERS_EVENT_OUT(low_memory)
	d_DEFINE_NO_MEMBERS_EVENT_OUT(will_enter_background)
	d_DEFINE_NO_MEMBERS_EVENT_OUT(in_background)
	d_DEFINE_NO_MEMBERS_EVENT_OUT(will_enter_foreground)
	d_DEFINE_NO_MEMBERS_EVENT_OUT(in_foreground)
	d_DEFINE_NO_MEMBERS_EVENT_OUT(quit)

	std::string to_string(const event_t& t) {
		return std::visit ([](const auto& x) { return to_string (x); }, t );
	}

	std::ostream& operator << (std::ostream& os, const event_t& t) {
		std::visit ([&os](const auto& x) { os << x; }, t );
		return os;
	}
}
