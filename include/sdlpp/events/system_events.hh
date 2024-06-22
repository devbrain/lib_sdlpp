//
// Created by igor on 04/06/2020.
//

#ifndef SDLPP_SYSTEM_EVENTS_HH
#define SDLPP_SYSTEM_EVENTS_HH

#include <string>
#include <iosfwd>
#include <chrono>

#include <bitflags/bitflags.hpp>
#include <sdlpp/events/event_types.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <sdlpp/detail/window_id.hh>
#include <sdlpp/detail/joystick_id.hh>

namespace neutrino::sdl::events {
	using system_event = SDL_Event;

	namespace detail {
		class common_event {
			public:
				const std::chrono::milliseconds timestamp;

			public:
				common_event()
					: timestamp(SDL_GetTicks()) {
				}

				explicit common_event(uint32_t ms)
					: timestamp(ms) {
				}

				explicit common_event(std::chrono::milliseconds timestamp_)
					: timestamp(timestamp_) {
				}

				explicit common_event(const SDL_Event& ev)
					: timestamp(ev.common.timestamp) {
				}
		};

		class window_event : public common_event {
			public:
				const window_id_t window_id;

			protected:
				explicit window_event(const SDL_Event& ev)
					: common_event(ev), window_id(ev.window.windowID) {
				}
		};
	} // ns detail

	/**
	 * @brief The keyboard class represents a keyboard event.
	 *
	 * This class inherits from the window_event class and contains information
	 * about a keyboard event, such as whether a key was pressed or released, the
	 * scancode and keycode of the pressed/released key, and any modifiers that
	 * were pressed along with the key.
	 */
	class keyboard : public detail::window_event {
		public:
			const bool pressed;
			const bool repeat;
			const scancode scan_code;
			const keycode key_code;
			const uint16_t key_mod;

			explicit keyboard(const SDL_Event& e)
				: window_event(e),
				  pressed(e.key.state == SDL_PRESSED),
				  repeat(e.key.repeat > 0),
				  scan_code(static_cast <scancode>(e.key.keysym.scancode)),
				  key_code(static_cast <keycode>(e.key.keysym.sym)),
				  key_mod(e.key.keysym.mod) {
			}
	};

	d_SDLPP_OSTREAM(const keyboard&);

#define d_DEFINE_NO_MEMBERS_EVENT_WIN(NAME)                         \
  class NAME : public detail::window_event                          \
  {                                                                 \
    public: explicit NAME (const SDL_Event& e)                		\
      : window_event (e) {}             							\
  };                                                               	\
  d_SDLPP_OSTREAM(const NAME&)

	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_shown);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_hidden);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_exposed);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_minimized);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_maximized);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_restored);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_mouse_entered);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_mouse_leaved);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_focus_gained);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_focus_lost);
	d_DEFINE_NO_MEMBERS_EVENT_WIN(window_close);

#define d_DEFINE_NO_MEMBERS_EVENT(NAME)         							\
  class NAME : public detail::common_event        							\
  {                                             							\
    public: explicit NAME (const SDL_Event &e)         						\
	: common_event(e) {}          											\
  };                          						                        \
  d_SDLPP_OSTREAM(const NAME&)

	// urgent events
	d_DEFINE_NO_MEMBERS_EVENT(terminating);
	d_DEFINE_NO_MEMBERS_EVENT(low_memory);
	d_DEFINE_NO_MEMBERS_EVENT(will_enter_background);
	d_DEFINE_NO_MEMBERS_EVENT(in_background);
	d_DEFINE_NO_MEMBERS_EVENT(will_enter_foreground);
	d_DEFINE_NO_MEMBERS_EVENT(in_foreground);
	d_DEFINE_NO_MEMBERS_EVENT(quit);

	/**
	 * \class window_moved
	 *
	 * \brief A class representing the window moved event.
	 *
	 * The window_moved class is a derived class of detail::window_event class,
	 * representing the event when a window is moved.
	 *
	 * The class provides access to the x and y coordinates of the new position
	 * of the window.
	 *
	 * Usage:
	 * - Create an instance of window_moved class by passing a SDL_WindowEvent as a parameter
	 * - Access the x and y coordinates using the x and y member variables
	 *
	 * Example:
	 * \code
	 * SDL_WindowEvent event;
	 * // fill the event structure with actual event data
	 * window_moved windowMoved(event);
	 * unsigned x = windowMoved.x;
	 * unsigned y = windowMoved.y;
	 * \endcode
	 */
	class window_moved : public detail::window_event {
		public:
			unsigned x;
			unsigned y;

			explicit window_moved(const SDL_Event& e)
				: window_event(e),
				  x(static_cast <unsigned>(e.window.data1)),
				  y(static_cast <unsigned>(e.window.data2)) {
			}
	};

	d_SDLPP_OSTREAM(const window_moved&);
	/**
	 * @class window_resized
	 * @brief Represents a window resized event.
	 *
	 * The window_resized class is a subclass of the window_event class and represents
	 * a window resized event in the SDL library. It contains the width and height
	 * of the resized window.
	 */
	class window_resized : public detail::window_event {
		public:
			unsigned w;
			unsigned h;

			explicit window_resized(const SDL_Event& e)
				: window_event(e),
				  w(static_cast <unsigned>(e.window.data1)),
				  h(static_cast <unsigned>(e.window.data2)) {
			}
	};

	d_SDLPP_OSTREAM(const window_resized&);

	/**
	 *  \class text_editing
	 *
	 *  \brief Represents a text editing event.
	 *
	 *  The text_editing class is a subclass of the window_event class, representing a text editing event.
	 *  It contains information about the edited text, the start cursor position, and the length of the selected text.
	 *  It is designed to be used with SDL_TextEditingEvent to handle text editing events.
	 *
	 *  \see window_event
	 */
	class text_editing : public detail::window_event {
		public:
			static constexpr size_t MAX_TEXT_LENGTH = SDL_TEXTEDITINGEVENT_TEXT_SIZE;

			const char* text;
			unsigned start;
			unsigned length;

			explicit text_editing(const SDL_Event& e)
				: detail::window_event(e),
				  text(e.edit.text),
				  start(static_cast <unsigned>(e.edit.start)),
				  length(static_cast <unsigned>(e.edit.length)) {
			}
	};

	d_SDLPP_OSTREAM(const text_editing&);

	/**
	 * @class text_input
	 * @brief A class that represents a text input event in a window.
	 * @details The text_input class is a derived class of the window_event class.
	 * It inherits the window_id member variable from the base class, which represents
	 * the ID of the window that has the keyboard focus. Also, it has a text member variable
	 * that represents the input text.
	 *
	 * The maximum length of the text input is defined as MAX_TEXT_LENGTH, which is set to
	 * the value of SDL_TEXTEDITINGEVENT_TEXT_SIZE.
	 *
	 * This class is designed to be instantiated with an SDL_TextInputEvent object, which is
	 * a structure that holds text input event information.
	 */
	class text_input : public detail::window_event {
		public:
			static constexpr size_t MAX_TEXT_LENGTH = SDL_TEXTEDITINGEVENT_TEXT_SIZE;

			const char* text;

			explicit text_input(const SDL_Event& e)
				: window_event(e),
				  text(e.text.text) {
			}
	};

	d_SDLPP_OSTREAM(const text_input&);

	/**
	 * @enum mousebutton
	 * @brief Enum class representing the state of a mouse button.
	 */
	BEGIN_BITFLAGS(mousebutton)
		FLAG(LEFT)
		FLAG(RIGHT)
		FLAG(MIDDLE)
		FLAG(X1)
		FLAG(X2)
	END_BITFLAGS(mousebutton)

	d_SDLPP_OSTREAM(mousebutton);

	namespace detail {
		inline mousebutton map_mousebutton_from_bitflags(uint32_t b) {
			mousebutton out;
			if ((b & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK) {
				out |= mousebutton::LEFT;
			}
			if ((b & SDL_BUTTON_RMASK) == SDL_BUTTON_RMASK) {
				out |= mousebutton::RIGHT;
			}
			if ((b & SDL_BUTTON_MMASK) == SDL_BUTTON_MMASK) {
				out |= mousebutton::MIDDLE;
			}
			if ((b & SDL_BUTTON_X1MASK) == SDL_BUTTON_X1MASK) {
				out |= mousebutton::X1;
			}
			if ((b & SDL_BUTTON_X2MASK) == SDL_BUTTON_X2MASK) {
				out |= mousebutton::X2;
			}
			return out;
		}
	}

	typedef Uint32 mouse_id_t;

	/**
	 * @brief The mouse_motion struct represents a mouse motion event.
	 *
	 * It inherits from the window_event class and contains information about the mouse motion event, such as the mouse ID, button state, coordinates, and relative motion.
	 */
	class mouse_motion : public detail::window_event {
		public:
			mouse_id_t mouse_id;
			mousebutton state;
			int x;
			int y;
			int xrel;
			int yrel;

			explicit mouse_motion(const SDL_Event& e)
				: window_event(e),
				  mouse_id(static_cast <mouse_id_t>(e.motion.which)),
				  state(detail::map_mousebutton_from_bitflags(e.motion.state)),
				  x(e.motion.x),
				  y(e.motion.y),
				  xrel(e.motion.xrel),
				  yrel(e.motion.yrel) {
			}
	};

	d_SDLPP_OSTREAM(const mouse_motion&);

	/**
	 * @brief A struct representing a touch device motion event.
	 *
	 * touch_device_motion is a struct that inherits from the window_event struct.
	 * It provides information about a touch device motion event, such as the button state, coordinates, and motion deltas.
	 */
	class touch_device_motion : public detail::window_event {
		public:
			uint32_t button;
			int x;
			int y;
			int xrel;
			int yrel;

			explicit touch_device_motion(const SDL_Event& e)
				: window_event(e),
				  button(static_cast <uint32_t>(e.motion.state)),
				  x(e.motion.x),
				  y(e.motion.y),
				  xrel(e.motion.xrel),
				  yrel(e.motion.yrel) {
			}
	};

	d_SDLPP_OSTREAM(const touch_device_motion&);

	/**
	 * @class mouse_button
	 * @brief Represents a mouse button event.
	 *
	 * This class inherits from window_event class to provide information about the mouse button event,
	 * such as the ID of the mouse, button index, coordinates and whether the button was pressed or released.
	 */
	class mouse_button : public detail::window_event {
		public:
			mouse_id_t mouse_id;
			mousebutton button;
			int x;
			int y;
			bool pressed;

			explicit mouse_button(const SDL_Event& e)
				: window_event(e),
				  mouse_id(static_cast <mouse_id_t>(e.button.which)),
				  button(detail::map_mousebutton_from_bitflags(e.button.button)),
				  x(e.button.x),
				  y(e.button.y),
				  pressed(e.button.state == SDL_PRESSED) {
			}
	};

	d_SDLPP_OSTREAM(const mouse_button&);

	/**
	 *  @class touch_device_button
	 *  @brief Represents a button event from a touch device.
	 *
	 *  The touch_device_button class is a subclass of the window_event class and represents a button event
	 *  generated by a touch device. It stores information about the button, its position, and whether it was
	 *  pressed or released.
	 */
	class touch_device_button : public detail::window_event {
		public:
			uint32_t button;
			int x;
			int y;
			bool pressed;

			explicit touch_device_button(const SDL_Event& e)
				: window_event(e),
				  button(static_cast <uint32_t>(e.button.button)),
				  x(e.button.x),
				  y(e.button.y),
				  pressed(e.button.state == SDL_PRESSED) {
			}
	};

	d_SDLPP_OSTREAM(const touch_device_button&);

	/**
	 * @class mouse_wheel
	 * @brief Represents a mouse wheel event.
	 *
	 * The mouse_wheel class is a subclass of window_event and represents a mouse wheel event
	 * in which the user rotates the mouse wheel in either the horizontal or vertical direction.
	 */
	class mouse_wheel : public detail::window_event {
		public:
			mouse_id_t mouse_id;
			int x;
			int y;

			explicit mouse_wheel(const SDL_Event& e)
				: window_event(e),
				  mouse_id(static_cast <mouse_id_t>(e.wheel.which)),
				  x(e.wheel.x),
				  y(e.wheel.y) {
			}
	};

	d_SDLPP_OSTREAM(const mouse_wheel&);

	/**
	  * \class touch_device_wheel
	  *
	  * \brief A class representing a touch device wheel event.
	  *
	  * This class inherits from the window_event class and represents a touch device
	  * wheel event in a graphical window system. It provides information such as the
	  * position of the wheel event.
	  *
	  * It is constructed using an SDL_MouseWheelEvent object, which contains the necessary
	  * information about the wheel event.
	  */
	class touch_device_wheel : public detail::window_event {
		public:
			int x;
			int y;

			explicit touch_device_wheel(const SDL_Event& e)
				: window_event(e),
				  x(e.wheel.x),
				  y(e.wheel.y) {
			}
	};

	d_SDLPP_OSTREAM(const touch_device_wheel&);

	/**
	 * @class joystick_axis
	 * @brief Represents an axis event of a joystick
	 *
	 * This class holds information about an axis event of a joystick, such as the joystick ID, the axis index,
	 * and the value of the axis.
	 */
	class joystick_axis {
		public:
			joystick_id_t joystick;
			uint8_t axis;
			signed short value;

			explicit joystick_axis(const SDL_Event& e)
				: joystick(e.jaxis.which),
				  axis(e.jaxis.axis),
				  value(e.jaxis.value) {
			}
	};

	d_SDLPP_OSTREAM(const joystick_axis&);

	/**
	 * @class joystick_ball
	 * @brief Represents a joystick trackball event
	 */
	class joystick_ball {
		public:
			joystick_id_t joystick;
			uint8_t ball;
			signed short xrel;
			signed short yrel;

			explicit joystick_ball(const SDL_Event& e)
				: joystick(e.jball.which),
				  ball(e.jball.ball),
				  xrel(e.jball.xrel),
				  yrel(e.jball.yrel) {
			}
	};

	d_SDLPP_OSTREAM(const joystick_ball&);

	/**
	 * @class joystick_button
	 * Represents a joystick button event.
	 */
	class joystick_button {
		public:
			joystick_id_t joystick;
			Uint8 button;
			bool pressed;

			explicit joystick_button(const SDL_Event& e)
				: joystick(e.jbutton.which),
				  button(e.jbutton.button),
				  pressed(e.jbutton.state == SDL_PRESSED) {
			}
	};

	d_SDLPP_OSTREAM(const joystick_button&);

	/**
	 * @enum joystick_hat_state
	 * @brief Enumerations representing the states of a joystick hat
	 *
	 * This enumeration represents the possible states of a joystick hat. Each state
	 * corresponds to a specific direction or position of the joystick hat. The
	 * enumeration values are based on the SDL hat states.
	 */
	enum class joystick_hat_state {
		LEFTUP = SDL_HAT_LEFTUP,
		HAT_UP = SDL_HAT_UP,
		HAT_RIGHTUP = SDL_HAT_RIGHTUP,
		HAT_LEFT = SDL_HAT_LEFT,
		HAT_CENTERED = SDL_HAT_CENTERED,
		HAT_RIGHT = SDL_HAT_RIGHT,
		HAT_LEFTDOWN = SDL_HAT_LEFTDOWN,
		HAT_DOWN = SDL_HAT_DOWN,
		HAT_RIGHTDOWN = SDL_HAT_RIGHTDOWN
	};

	d_SDLPP_OSTREAM(joystick_hat_state);

	/**
	 * @class joystick_hat
	 * @brief Represents the state of a joystick hat
	 *
	 * The joystick_hat class represents the state of a joystick hat. It stores the ID of the joystick,
	 * the value of the hat, and the state of the hat. The state corresponds to a specific direction or
	 * position of the hat.
	 */
	class joystick_hat {
		public:
			joystick_id_t joystick;
			Uint8 value;
			joystick_hat_state state;

			explicit joystick_hat(const SDL_Event& e)
				: joystick(e.jhat.which),
				  value(e.jhat.value),
				  state(static_cast <joystick_hat_state>(e.jhat.value)) {
			}
	};

	d_SDLPP_OSTREAM(const joystick_hat&);

	/**
	 * @struct user
	 * @brief A class representing a user-defined event.
	 *
	 * The user class provides a convenient way to create, store, and manipulate
	 * user-defined events. It encapsulates the SDL_UserEvent struct and provides
	 * constructors that accept different combinations of parameters to initialize
	 * the event code and data pointers.
	 */
	struct user {
		public:
			explicit user(const SDL_Event& u)
				: code(u.user.code),
				  data1(u.user.data1),
				  data2(u.user.data2) {
			}

			explicit user(int32_t code_)
				: code(code_),
				  data1(nullptr),
				  data2(nullptr) {
			}

			user(int32_t code_, void* d1, void* d2 = nullptr)
				: code(code_),
				  data1(d1),
				  data2(d2) {
			}

			int32_t code;
			void* data1;
			void* data2;
	};

	d_SDLPP_OSTREAM(const user&);

	using event_t = std::variant <
		std::monostate,
		keyboard,
		window_shown,
		window_hidden,
		window_exposed,
		window_minimized,
		window_maximized,
		window_restored,
		window_mouse_entered,
		window_mouse_leaved,
		window_focus_gained,
		window_focus_lost,
		window_close,
		terminating,
		low_memory,
		will_enter_background,
		in_background,
		will_enter_foreground,
		in_foreground,
		quit,
		window_moved,
		window_resized,
		text_editing,
		text_input,
		mouse_motion,
		touch_device_motion,
		mouse_button,
		touch_device_button,
		mouse_wheel,
		touch_device_wheel,
		joystick_axis,
		joystick_ball,
		joystick_button,
		joystick_hat,
		user>;
	d_SDLPP_OSTREAM(const event_t&);
}
#endif //SDLPP_SYSTEM_EVENTS_HH
