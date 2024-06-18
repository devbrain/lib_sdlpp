//
// Created by igor on 04/06/2020.
//

#ifndef NEUTRINO_SDL_EVENTS_EVENT_TYPES_HH
#define NEUTRINO_SDL_EVENTS_EVENT_TYPES_HH

#include <cstdint>
#include <string>
#include <array>
#include <type_traits>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp//detail/ostreamops.hh>

namespace neutrino::sdl {

	/**
	 * @enum event_type
	 * @brief Enumeration for different types of events
	 *
	 * This enumeration defines the different types of events that can be
	 * generated by the application or the underlying system.
	 */
	enum class event_type : uint32_t {

		QUIT = SDL_QUIT, /**< User-requested quit */
		/* These application events have special meaning on iOS, see README-ios.md for details */
		APP_TERMINATING = SDL_APP_TERMINATING,        /**< The application is being terminated by the OS
                                 Called on iOS in applicationWillTerminate()
                                 Called on Android in onDestroy()
                              */
		APP_LOWMEMORY = SDL_APP_LOWMEMORY,          /**< The application is low on memory, free memory if possible.
                               Called on iOS in applicationDidReceiveMemoryWarning()
                               Called on Android in onLowMemory()
                            */
		APP_WILLENTERBACKGROUND = SDL_APP_WILLENTERBACKGROUND, /**< The application is about to enter the background
                                  Called on iOS in applicationWillResignActive()
                                  Called on Android in onPause()
                                   */
		APP_DIDENTERBACKGROUND = SDL_APP_DIDENTERBACKGROUND, /**< The application did enter the background and may not get CPU for some time
                                    Called on iOS in applicationDidEnterBackground()
                                    Called on Android in onPause()
                                 */
		APP_WILLENTERFOREGROUND = SDL_APP_WILLENTERFOREGROUND, /**< The application is about to enter the foreground
                                  Called on iOS in applicationWillEnterForeground()
                                  Called on Android in onResume()
                                   */
		APP_DIDENTERFOREGROUND = SDL_APP_DIDENTERFOREGROUND, /**< The application is now interactive
                                    Called on iOS in applicationDidBecomeActive()
                                    Called on Android in onResume()
                                 */

		/* Window events */
		WINDOWEVENT = SDL_WINDOWEVENT, /**< Window state change */
		SYSWMEVENT = SDL_SYSWMEVENT,             /**< System specific event */
		/* Keyboard events */
		KEYDOWN = SDL_KEYDOWN,                /**< Key pressed */
		KEYUP = SDL_KEYUP,                  /**< Key released */
		TEXTEDITING = SDL_TEXTEDITING,            /**< Keyboard text editing (composition) */
		TEXTINPUT = SDL_TEXTINPUT,              /**< Keyboard text input */
		KEYMAPCHANGED = SDL_KEYMAPCHANGED,          /**< Keymap changed due to a system event such as an
                                                      input language or keyboard layout change.
                                                   */

		/* Mouse events */
		MOUSEMOTION = SDL_MOUSEMOTION, /**< Mouse moved */
		MOUSEBUTTONDOWN = SDL_MOUSEBUTTONDOWN,        /**< Mouse button pressed */
		MOUSEBUTTONUP = SDL_MOUSEBUTTONUP,          /**< Mouse button released */
		MOUSEWHEEL = SDL_MOUSEWHEEL,             /**< Mouse wheel motion */
		/* Joystick events */
		JOYAXISMOTION = SDL_JOYAXISMOTION, /**< Joystick axis motion */
		JOYBALLMOTION = SDL_JOYBALLMOTION,          /**< Joystick trackball motion */
		JOYHATMOTION = SDL_JOYHATMOTION,           /**< Joystick hat position change */
		JOYBUTTONDOWN = SDL_JOYBUTTONDOWN,          /**< Joystick button pressed */
		JOYBUTTONUP = SDL_JOYBUTTONUP,            /**< Joystick button released */
		JOYDEVICEADDED = SDL_JOYDEVICEADDED,         /**< A new joystick has been inserted into the system */
		JOYDEVICEREMOVED = SDL_JOYDEVICEREMOVED,       /**< An opened joystick has been removed */
		/* Game controller events */
		CONTROLLERAXISMOTION = SDL_CONTROLLERAXISMOTION, /**< Game controller axis motion */
		CONTROLLERBUTTONDOWN = SDL_CONTROLLERBUTTONDOWN,          /**< Game controller button pressed */
		CONTROLLERBUTTONUP = SDL_CONTROLLERBUTTONUP,            /**< Game controller button released */
		CONTROLLERDEVICEADDED = SDL_CONTROLLERDEVICEADDED,         /**< A new Game controller has been inserted into the system */
		CONTROLLERDEVICEREMOVED = SDL_CONTROLLERDEVICEREMOVED,       /**< An opened Game controller has been removed */
		CONTROLLERDEVICEREMAPPED = SDL_CONTROLLERDEVICEREMAPPED,      /**< The controller mapping was updated */
		/* Touch events */
		FINGERDOWN = SDL_FINGERDOWN,
		FINGERUP = SDL_FINGERUP,
		FINGERMOTION = SDL_FINGERMOTION,
		/* Gesture events */
		DOLLARGESTURE = SDL_DOLLARGESTURE,
		DOLLARRECORD = SDL_DOLLARRECORD,
		MULTIGESTURE = SDL_MULTIGESTURE,
		/* Clipboard events */
		CLIPBOARDUPDATE = SDL_CLIPBOARDUPDATE, /**< The clipboard changed */
		/* Drag and drop events */
		DROPFILE = SDL_DROPFILE, /**< The system requests a file open */

		DROPTEXT = SDL_DROPTEXT,            /**< text/plain drag-and-drop event */
		DROPBEGIN = SDL_DROPBEGIN,           /**< A new set of drops is beginning (NULL filename) */
		DROPCOMPLETE = SDL_DROPCOMPLETE,        /**< Current set of drops is now complete (NULL filename) */

		/* Audio hotplug events */
		AUDIODEVICEADDED = SDL_AUDIODEVICEADDED, /**< A new audio device is available */
		AUDIODEVICEREMOVED = SDL_AUDIODEVICEREMOVED,        /**< An audio device has been removed. */
		/* Render events */
		RENDER_TARGETS_RESET = SDL_RENDER_TARGETS_RESET, /**< The render targets have been reset and their contents need to be updated */
		RENDER_DEVICE_RESET = SDL_RENDER_DEVICE_RESET, /**< The device has been reset and all textures need to be recreated */
		USEREVENT = SDL_USEREVENT
	};

	/**
	 * @brief This represents actions associated with SDL event operations.
	 *
	 * The event_action class is an enumerated type that defines the different actions that can be performed
	 * with SDL event operations. It provides the following actions:
	 *   - ADD: Indicates the action of adding an event to the event queue.
	 *   - PEEK: Indicates the action of peeking at the first event in the event queue.
	 *   - GET: Indicates the action of retrieving the first event from the event queue.
	 *
	 * The event_action class is used as an input parameter in functions that deal with SDL event operations
	 * and helps specify the desired action to be performed.
	 */
	enum class event_action {
		ADD = SDL_ADDEVENT,
		PEEK = SDL_PEEKEVENT,
		GET = SDL_GETEVENT
	};

	/**
	 * @enum scancode
	 * @brief Enumerated type for keyboard scancodes
	 *
	 * This represents the scancodes for keyboard keys used in SDL. Each scancode value
	 * corresponds to a specific key on the keyboard. The scancodes are based on the SDL_SCANCODE_
	 * constants provided by the SDL library.
	 */
	enum scancode {
		UNKNOWN = SDL_SCANCODE_UNKNOWN,
		A = SDL_SCANCODE_A,
		B = SDL_SCANCODE_B,
		C = SDL_SCANCODE_C,
		D = SDL_SCANCODE_D,
		E = SDL_SCANCODE_E,
		F = SDL_SCANCODE_F,
		G = SDL_SCANCODE_G,
		H = SDL_SCANCODE_H,
		I = SDL_SCANCODE_I,
		J = SDL_SCANCODE_J,
		K = SDL_SCANCODE_K,
		L = SDL_SCANCODE_L,
		M = SDL_SCANCODE_M,
		N = SDL_SCANCODE_N,
		O = SDL_SCANCODE_O,
		P = SDL_SCANCODE_P,
		Q = SDL_SCANCODE_Q,
		R = SDL_SCANCODE_R,
		S = SDL_SCANCODE_S,
		T = SDL_SCANCODE_T,
		U = SDL_SCANCODE_U,
		V = SDL_SCANCODE_V,
		W = SDL_SCANCODE_W,
		X = SDL_SCANCODE_X,
		Y = SDL_SCANCODE_Y,
		Z = SDL_SCANCODE_Z,
		_1 = SDL_SCANCODE_1,
		_2 = SDL_SCANCODE_2,
		_3 = SDL_SCANCODE_3,
		_4 = SDL_SCANCODE_4,
		_5 = SDL_SCANCODE_5,
		_6 = SDL_SCANCODE_6,
		_7 = SDL_SCANCODE_7,
		_8 = SDL_SCANCODE_8,
		_9 = SDL_SCANCODE_9,
		_0 = SDL_SCANCODE_0,
		RETURN = SDL_SCANCODE_RETURN,
		ESCAPE = SDL_SCANCODE_ESCAPE,
		BACKSPACE = SDL_SCANCODE_BACKSPACE,
		TAB = SDL_SCANCODE_TAB,
		SPACE = SDL_SCANCODE_SPACE,
		MINUS = SDL_SCANCODE_MINUS,
		EQUALS = SDL_SCANCODE_EQUALS,
		LEFTBRACKET = SDL_SCANCODE_LEFTBRACKET,
		RIGHTBRACKET = SDL_SCANCODE_RIGHTBRACKET,
		BACKSLASH = SDL_SCANCODE_BACKSLASH,
		NONUSHASH = SDL_SCANCODE_NONUSHASH,
		SEMICOLON = SDL_SCANCODE_SEMICOLON,
		APOSTROPHE = SDL_SCANCODE_APOSTROPHE,
		GRAVE = SDL_SCANCODE_GRAVE,
		COMMA = SDL_SCANCODE_COMMA,
		PERIOD = SDL_SCANCODE_PERIOD,
		SLASH = SDL_SCANCODE_SLASH,
		CAPSLOCK = SDL_SCANCODE_CAPSLOCK,
		F1 = SDL_SCANCODE_F1,
		F2 = SDL_SCANCODE_F2,
		F3 = SDL_SCANCODE_F3,
		F4 = SDL_SCANCODE_F4,
		F5 = SDL_SCANCODE_F5,
		F6 = SDL_SCANCODE_F6,
		F7 = SDL_SCANCODE_F7,
		F8 = SDL_SCANCODE_F8,
		F9 = SDL_SCANCODE_F9,
		F10 = SDL_SCANCODE_F10,
		F11 = SDL_SCANCODE_F11,
		F12 = SDL_SCANCODE_F12,
		PRINTSCREEN = SDL_SCANCODE_PRINTSCREEN,
		SCROLLLOCK = SDL_SCANCODE_SCROLLLOCK,
		PAUSE = SDL_SCANCODE_PAUSE,
		INSERT = SDL_SCANCODE_INSERT,
		HOME = SDL_SCANCODE_HOME,
		PAGEUP = SDL_SCANCODE_PAGEUP,
		DEL = SDL_SCANCODE_DELETE,
		END = SDL_SCANCODE_END,
		PAGEDOWN = SDL_SCANCODE_PAGEDOWN,
		RIGHT = SDL_SCANCODE_RIGHT,
		LEFT = SDL_SCANCODE_LEFT,
		DOWN = SDL_SCANCODE_DOWN,
		UP = SDL_SCANCODE_UP,
		NUMLOCKCLEAR = SDL_SCANCODE_NUMLOCKCLEAR,
		KP_DIVIDE = SDL_SCANCODE_KP_DIVIDE,
		KP_MULTIPLY = SDL_SCANCODE_KP_MULTIPLY,
		KP_MINUS = SDL_SCANCODE_KP_MINUS,
		KP_PLUS = SDL_SCANCODE_KP_PLUS,
		KP_ENTER = SDL_SCANCODE_KP_ENTER,
		KP_1 = SDL_SCANCODE_KP_1,
		KP_2 = SDL_SCANCODE_KP_2,
		KP_3 = SDL_SCANCODE_KP_3,
		KP_4 = SDL_SCANCODE_KP_4,
		KP_5 = SDL_SCANCODE_KP_5,
		KP_6 = SDL_SCANCODE_KP_6,
		KP_7 = SDL_SCANCODE_KP_7,
		KP_8 = SDL_SCANCODE_KP_8,
		KP_9 = SDL_SCANCODE_KP_9,
		KP_0 = SDL_SCANCODE_KP_0,
		KP_PERIOD = SDL_SCANCODE_KP_PERIOD,
		NONUSBACKSLASH = SDL_SCANCODE_NONUSBACKSLASH,
		APPLICATION = SDL_SCANCODE_APPLICATION,
		POWER = SDL_SCANCODE_POWER,
		KP_EQUALS = SDL_SCANCODE_KP_EQUALS,
		F13 = SDL_SCANCODE_F13,
		F14 = SDL_SCANCODE_F14,
		F15 = SDL_SCANCODE_F15,
		F16 = SDL_SCANCODE_F16,
		F17 = SDL_SCANCODE_F17,
		F18 = SDL_SCANCODE_F18,
		F19 = SDL_SCANCODE_F19,
		F20 = SDL_SCANCODE_F20,
		F21 = SDL_SCANCODE_F21,
		F22 = SDL_SCANCODE_F22,
		F23 = SDL_SCANCODE_F23,
		F24 = SDL_SCANCODE_F24,
		EXECUTE = SDL_SCANCODE_EXECUTE,
		HELP = SDL_SCANCODE_HELP,
		MENU = SDL_SCANCODE_MENU,
		SELECT = SDL_SCANCODE_SELECT,
		STOP = SDL_SCANCODE_STOP,
		AGAIN = SDL_SCANCODE_AGAIN,
		UNDO = SDL_SCANCODE_UNDO,
		CUT = SDL_SCANCODE_CUT,
		COPY = SDL_SCANCODE_COPY,
		PASTE = SDL_SCANCODE_PASTE,
		FIND = SDL_SCANCODE_FIND,
		MUTE = SDL_SCANCODE_MUTE,
		VOLUMEUP = SDL_SCANCODE_VOLUMEUP,
		VOLUMEDOWN = SDL_SCANCODE_VOLUMEDOWN,
		KP_COMMA = SDL_SCANCODE_KP_COMMA,
		KP_EQUALSAS400 = SDL_SCANCODE_KP_EQUALSAS400,
		INTERNATIONAL1 = SDL_SCANCODE_INTERNATIONAL1,
		INTERNATIONAL2 = SDL_SCANCODE_INTERNATIONAL2,
		INTERNATIONAL3 = SDL_SCANCODE_INTERNATIONAL3,
		INTERNATIONAL4 = SDL_SCANCODE_INTERNATIONAL4,
		INTERNATIONAL5 = SDL_SCANCODE_INTERNATIONAL5,
		INTERNATIONAL6 = SDL_SCANCODE_INTERNATIONAL6,
		INTERNATIONAL7 = SDL_SCANCODE_INTERNATIONAL7,
		INTERNATIONAL8 = SDL_SCANCODE_INTERNATIONAL8,
		INTERNATIONAL9 = SDL_SCANCODE_INTERNATIONAL9,
		LANG1 = SDL_SCANCODE_LANG1,
		LANG2 = SDL_SCANCODE_LANG2,
		LANG3 = SDL_SCANCODE_LANG3,
		LANG4 = SDL_SCANCODE_LANG4,
		LANG5 = SDL_SCANCODE_LANG5,
		LANG6 = SDL_SCANCODE_LANG6,
		LANG7 = SDL_SCANCODE_LANG7,
		LANG8 = SDL_SCANCODE_LANG8,
		LANG9 = SDL_SCANCODE_LANG9,
		ALTERASE = SDL_SCANCODE_ALTERASE,
		SYSREQ = SDL_SCANCODE_SYSREQ,
		CANCEL = SDL_SCANCODE_CANCEL,
		CLEAR = SDL_SCANCODE_CLEAR,
		PRIOR = SDL_SCANCODE_PRIOR,
		RETURN2 = SDL_SCANCODE_RETURN2,
		SEPARATOR = SDL_SCANCODE_SEPARATOR,
		KBD_OUT = SDL_SCANCODE_OUT,
		OPER = SDL_SCANCODE_OPER,
		CLEARAGAIN = SDL_SCANCODE_CLEARAGAIN,
		CRSEL = SDL_SCANCODE_CRSEL,
		EXSEL = SDL_SCANCODE_EXSEL,
		KP_00 = SDL_SCANCODE_KP_00,
		KP_000 = SDL_SCANCODE_KP_000,
		THOUSANDSSEPARATOR = SDL_SCANCODE_THOUSANDSSEPARATOR,
		DECIMALSEPARATOR = SDL_SCANCODE_DECIMALSEPARATOR,
		CURRENCYUNIT = SDL_SCANCODE_CURRENCYUNIT,
		CURRENCYSUBUNIT = SDL_SCANCODE_CURRENCYSUBUNIT,
		KP_LEFTPAREN = SDL_SCANCODE_KP_LEFTPAREN,
		KP_RIGHTPAREN = SDL_SCANCODE_KP_RIGHTPAREN,
		KP_LEFTBRACE = SDL_SCANCODE_KP_LEFTBRACE,
		KP_RIGHTBRACE = SDL_SCANCODE_KP_RIGHTBRACE,
		KP_TAB = SDL_SCANCODE_KP_TAB,
		KP_BACKSPACE = SDL_SCANCODE_KP_BACKSPACE,
		KP_A = SDL_SCANCODE_KP_A,
		KP_B = SDL_SCANCODE_KP_B,
		KP_C = SDL_SCANCODE_KP_C,
		KP_D = SDL_SCANCODE_KP_D,
		KP_E = SDL_SCANCODE_KP_E,
		KP_F = SDL_SCANCODE_KP_F,
		KP_XOR = SDL_SCANCODE_KP_XOR,
		KP_POWER = SDL_SCANCODE_KP_POWER,
		KP_PERCENT = SDL_SCANCODE_KP_PERCENT,
		KP_LESS = SDL_SCANCODE_KP_LESS,
		KP_GREATER = SDL_SCANCODE_KP_GREATER,
		KP_AMPERSAND = SDL_SCANCODE_KP_AMPERSAND,
		KP_DBLAMPERSAND = SDL_SCANCODE_KP_DBLAMPERSAND,
		KP_VERTICALBAR = SDL_SCANCODE_KP_VERTICALBAR,
		KP_DBLVERTICALBAR = SDL_SCANCODE_KP_DBLVERTICALBAR,
		KP_COLON = SDL_SCANCODE_KP_COLON,
		KP_HASH = SDL_SCANCODE_KP_HASH,
		KP_SPACE = SDL_SCANCODE_KP_SPACE,
		KP_AT = SDL_SCANCODE_KP_AT,
		KP_EXCLAM = SDL_SCANCODE_KP_EXCLAM,
		KP_MEMSTORE = SDL_SCANCODE_KP_MEMSTORE,
		KP_MEMRECALL = SDL_SCANCODE_KP_MEMRECALL,
		KP_MEMCLEAR = SDL_SCANCODE_KP_MEMCLEAR,
		KP_MEMADD = SDL_SCANCODE_KP_MEMADD,
		KP_MEMSUBTRACT = SDL_SCANCODE_KP_MEMSUBTRACT,
		KP_MEMMULTIPLY = SDL_SCANCODE_KP_MEMMULTIPLY,
		KP_MEMDIVIDE = SDL_SCANCODE_KP_MEMDIVIDE,
		KP_PLUSMINUS = SDL_SCANCODE_KP_PLUSMINUS,
		KP_CLEAR = SDL_SCANCODE_KP_CLEAR,
		KP_CLEARENTRY = SDL_SCANCODE_KP_CLEARENTRY,
		KP_BINARY = SDL_SCANCODE_KP_BINARY,
		KP_OCTAL = SDL_SCANCODE_KP_OCTAL,
		KP_DECIMAL = SDL_SCANCODE_KP_DECIMAL,
		KP_HEXADECIMAL = SDL_SCANCODE_KP_HEXADECIMAL,
		LCTRL = SDL_SCANCODE_LCTRL,
		LSHIFT = SDL_SCANCODE_LSHIFT,
		LALT = SDL_SCANCODE_LALT,
		LGUI = SDL_SCANCODE_LGUI,
		RCTRL = SDL_SCANCODE_RCTRL,
		RSHIFT = SDL_SCANCODE_RSHIFT,
		RALT = SDL_SCANCODE_RALT,
		RGUI = SDL_SCANCODE_RGUI,
		MODE = SDL_SCANCODE_MODE,
		AUDIONEXT = SDL_SCANCODE_AUDIONEXT,
		AUDIOPREV = SDL_SCANCODE_AUDIOPREV,
		AUDIOSTOP = SDL_SCANCODE_AUDIOSTOP,
		AUDIOPLAY = SDL_SCANCODE_AUDIOPLAY,
		AUDIOMUTE = SDL_SCANCODE_AUDIOMUTE,
		MEDIASELECT = SDL_SCANCODE_MEDIASELECT,
		WWW = SDL_SCANCODE_WWW,
		MAIL = SDL_SCANCODE_MAIL,
		CALCULATOR = SDL_SCANCODE_CALCULATOR,
		COMPUTER = SDL_SCANCODE_COMPUTER,
		AC_SEARCH = SDL_SCANCODE_AC_SEARCH,
		AC_HOME = SDL_SCANCODE_AC_HOME,
		AC_BACK = SDL_SCANCODE_AC_BACK,
		AC_FORWARD = SDL_SCANCODE_AC_FORWARD,
		AC_STOP = SDL_SCANCODE_AC_STOP,
		AC_REFRESH = SDL_SCANCODE_AC_REFRESH,
		AC_BOOKMARKS = SDL_SCANCODE_AC_BOOKMARKS,
		BRIGHTNESSDOWN = SDL_SCANCODE_BRIGHTNESSDOWN,
		BRIGHTNESSUP = SDL_SCANCODE_BRIGHTNESSUP,
		DISPLAYSWITCH = SDL_SCANCODE_DISPLAYSWITCH,
		KBDILLUMTOGGLE = SDL_SCANCODE_KBDILLUMTOGGLE,
		KBDILLUMDOWN = SDL_SCANCODE_KBDILLUMDOWN,
		KBDILLUMUP = SDL_SCANCODE_KBDILLUMUP,
		EJECT = SDL_SCANCODE_EJECT,
		SLEEP = SDL_SCANCODE_SLEEP,
		APP1 = SDL_SCANCODE_APP1,
		APP2 = SDL_SCANCODE_APP2,
		NUM_SCANCODES = SDL_NUM_SCANCODES
	};

	/**
	 * @brief Enum representing key codes.
	 *
	 * This defines key codes for different keys on a keyboard.
	 */
	enum class keycode {
		UNKNOWN = SDLK_UNKNOWN,
		RETURN = SDLK_RETURN,
		ESCAPE = SDLK_ESCAPE,
		BACKSPACE = SDLK_BACKSPACE,
		TAB = SDLK_TAB,
		SPACE = SDLK_SPACE,
		EXCLAIM = SDLK_EXCLAIM,
		QUOTEDBL = SDLK_QUOTEDBL,
		PERCENT = SDLK_PERCENT,
		DOLLAR = SDLK_DOLLAR,
		AMPERSAND = SDLK_AMPERSAND,
		QUOTE = SDLK_QUOTE,
		LEFTPAREN = SDLK_LEFTPAREN,
		RIGHTPAREN = SDLK_RIGHTPAREN,
		ASTERISK = SDLK_ASTERISK,
		PLUS = SDLK_PLUS,
		COMMA = SDLK_COMMA,
		MINUS = SDLK_MINUS,
		PERIOD = SDLK_PERIOD,
		SLASH = SDLK_SLASH,
		_0 = SDLK_0,
		_1 = SDLK_1,
		_2 = SDLK_2,
		_3 = SDLK_3,
		_4 = SDLK_4,
		_5 = SDLK_5,
		_6 = SDLK_6,
		_7 = SDLK_7,
		_8 = SDLK_8,
		_9 = SDLK_9,
		COLON = SDLK_COLON,
		SEMICOLON = SDLK_SEMICOLON,
		LESS = SDLK_LESS,
		EQUALS = SDLK_EQUALS,
		GREATER = SDLK_GREATER,
		QUESTION = SDLK_QUESTION,
		AT = SDLK_AT,
		LEFTBRACKET = SDLK_LEFTBRACKET,
		BACKSLASH = SDLK_BACKSLASH,
		RIGHTBRACKET = SDLK_RIGHTBRACKET,
		CARET = SDLK_CARET,
		UNDERSCORE = SDLK_UNDERSCORE,
		BACKQUOTE = SDLK_BACKQUOTE,
		a = SDLK_a,
		b = SDLK_b,
		c = SDLK_c,
		d = SDLK_d,
		e = SDLK_e,
		f = SDLK_f,
		g = SDLK_g,
		h = SDLK_h,
		i = SDLK_i,
		j = SDLK_j,
		k = SDLK_k,
		l = SDLK_l,
		m = SDLK_m,
		n = SDLK_n,
		o = SDLK_o,
		p = SDLK_p,
		q = SDLK_q,
		r = SDLK_r,
		s = SDLK_s,
		t = SDLK_t,
		u = SDLK_u,
		v = SDLK_v,
		w = SDLK_w,
		x = SDLK_x,
		y = SDLK_y,
		z = SDLK_z,
		CAPSLOCK = SDLK_CAPSLOCK,
		F1 = SDLK_F1,
		F2 = SDLK_F2,
		F3 = SDLK_F3,
		F4 = SDLK_F4,
		F5 = SDLK_F5,
		F6 = SDLK_F6,
		F7 = SDLK_F7,
		F8 = SDLK_F8,
		F9 = SDLK_F9,
		F10 = SDLK_F10,
		F11 = SDLK_F11,
		F12 = SDLK_F12,
		PRINTSCREEN = SDLK_PRINTSCREEN,
		SCROLLLOCK = SDLK_SCROLLLOCK,
		PAUSE = SDLK_PAUSE,
		INSERT = SDLK_INSERT,
		HOME = SDLK_HOME,
		PAGEUP = SDLK_PAGEUP,
		DEL = SDLK_DELETE,
		END = SDLK_END,
		PAGEDOWN = SDLK_PAGEDOWN,
		RIGHT = SDLK_RIGHT,
		LEFT = SDLK_LEFT,
		DOWN = SDLK_DOWN,
		UP = SDLK_UP,
		NUMLOCKCLEAR = SDLK_NUMLOCKCLEAR,
		KP_DIVIDE = SDLK_KP_DIVIDE,
		KP_MULTIPLY = SDLK_KP_MULTIPLY,
		KP_MINUS = SDLK_KP_MINUS,
		KP_PLUS = SDLK_KP_PLUS,
		KP_ENTER = SDLK_KP_ENTER,
		KP_1 = SDLK_KP_1,
		KP_2 = SDLK_KP_2,
		KP_3 = SDLK_KP_3,
		KP_4 = SDLK_KP_4,
		KP_5 = SDLK_KP_5,
		KP_6 = SDLK_KP_6,
		KP_7 = SDLK_KP_7,
		KP_8 = SDLK_KP_8,
		KP_9 = SDLK_KP_9,
		KP_0 = SDLK_KP_0,
		KP_PERIOD = SDLK_KP_PERIOD,
		APPLICATION = SDLK_APPLICATION,
		POWER = SDLK_POWER,
		KP_EQUALS = SDLK_KP_EQUALS,
		F13 = SDLK_F13,
		F14 = SDLK_F14,
		F15 = SDLK_F15,
		F16 = SDLK_F16,
		F17 = SDLK_F17,
		F18 = SDLK_F18,
		F19 = SDLK_F19,
		F20 = SDLK_F20,
		F21 = SDLK_F21,
		F22 = SDLK_F22,
		F23 = SDLK_F23,
		F24 = SDLK_F24,
		EXECUTE = SDLK_EXECUTE,
		HELP = SDLK_HELP,
		MENU = SDLK_MENU,
		SELECT = SDLK_SELECT,
		STOP = SDLK_STOP,
		AGAIN = SDLK_AGAIN,
		UNDO = SDLK_UNDO,
		CUT = SDLK_CUT,
		COPY = SDLK_COPY,
		PASTE = SDLK_PASTE,
		FIND = SDLK_FIND,
		MUTE = SDLK_MUTE,
		VOLUMEUP = SDLK_VOLUMEUP,
		VOLUMEDOWN = SDLK_VOLUMEDOWN,
		KP_COMMA = SDLK_KP_COMMA,
		KP_EQUALSAS400 = SDLK_KP_EQUALSAS400,
		ALTERASE = SDLK_ALTERASE,
		SYSREQ = SDLK_SYSREQ,
		CANCEL = SDLK_CANCEL,
		CLEAR = SDLK_CLEAR,
		PRIOR = SDLK_PRIOR,
		RETURN2 = SDLK_RETURN2,
		SEPARATOR = SDLK_SEPARATOR,
		KBD_OUT = SDLK_OUT,
		OPER = SDLK_OPER,
		CLEARAGAIN = SDLK_CLEARAGAIN,
		CRSEL = SDLK_CRSEL,
		EXSEL = SDLK_EXSEL,
		KP_00 = SDLK_KP_00,
		KP_000 = SDLK_KP_000,
		THOUSANDSSEPARATOR = SDLK_THOUSANDSSEPARATOR,
		DECIMALSEPARATOR = SDLK_DECIMALSEPARATOR,
		CURRENCYUNIT = SDLK_CURRENCYUNIT,
		CURRENCYSUBUNIT = SDLK_CURRENCYSUBUNIT,
		KP_LEFTPAREN = SDLK_KP_LEFTPAREN,
		KP_RIGHTPAREN = SDLK_KP_RIGHTPAREN,
		KP_LEFTBRACE = SDLK_KP_LEFTBRACE,
		KP_RIGHTBRACE = SDLK_KP_RIGHTBRACE,
		KP_TAB = SDLK_KP_TAB,
		KP_BACKSPACE = SDLK_KP_BACKSPACE,
		KP_A = SDLK_KP_A,
		KP_B = SDLK_KP_B,
		KP_C = SDLK_KP_C,
		KP_D = SDLK_KP_D,
		KP_E = SDLK_KP_E,
		KP_F = SDLK_KP_F,
		KP_XOR = SDLK_KP_XOR,
		KP_POWER = SDLK_KP_POWER,
		KP_PERCENT = SDLK_KP_PERCENT,
		KP_LESS = SDLK_KP_LESS,
		KP_GREATER = SDLK_KP_GREATER,
		KP_AMPERSAND = SDLK_KP_AMPERSAND,
		KP_DBLAMPERSAND = SDLK_KP_DBLAMPERSAND,
		KP_VERTICALBAR = SDLK_KP_VERTICALBAR,
		KP_DBLVERTICALBAR = SDLK_KP_DBLVERTICALBAR,
		KP_COLON = SDLK_KP_COLON,
		KP_HASH = SDLK_KP_HASH,
		KP_SPACE = SDLK_KP_SPACE,
		KP_AT = SDLK_KP_AT,
		KP_EXCLAM = SDLK_KP_EXCLAM,
		KP_MEMSTORE = SDLK_KP_MEMSTORE,
		KP_MEMRECALL = SDLK_KP_MEMRECALL,
		KP_MEMCLEAR = SDLK_KP_MEMCLEAR,
		KP_MEMADD = SDLK_KP_MEMADD,
		KP_MEMSUBTRACT = SDLK_KP_MEMSUBTRACT,
		KP_MEMMULTIPLY = SDLK_KP_MEMMULTIPLY,
		KP_MEMDIVIDE = SDLK_KP_MEMDIVIDE,
		KP_PLUSMINUS = SDLK_KP_PLUSMINUS,
		KP_CLEAR = SDLK_KP_CLEAR,
		KP_CLEARENTRY = SDLK_KP_CLEARENTRY,
		KP_BINARY = SDLK_KP_BINARY,
		KP_OCTAL = SDLK_KP_OCTAL,
		KP_DECIMAL = SDLK_KP_DECIMAL,
		KP_HEXADECIMAL = SDLK_KP_HEXADECIMAL,
		LCTRL = SDLK_LCTRL,
		LSHIFT = SDLK_LSHIFT,
		LALT = SDLK_LALT,
		LGUI = SDLK_LGUI,
		RCTRL = SDLK_RCTRL,
		RSHIFT = SDLK_RSHIFT,
		RALT = SDLK_RALT,
		RGUI = SDLK_RGUI,
		MODE = SDLK_MODE,
		AUDIONEXT = SDLK_AUDIONEXT,
		AUDIOPREV = SDLK_AUDIOPREV,
		AUDIOSTOP = SDLK_AUDIOSTOP,
		AUDIOPLAY = SDLK_AUDIOPLAY,
		AUDIOMUTE = SDLK_AUDIOMUTE,
		MEDIASELECT = SDLK_MEDIASELECT,
		WWW = SDLK_WWW,
		MAIL = SDLK_MAIL,
		CALCULATOR = SDLK_CALCULATOR,
		COMPUTER = SDLK_COMPUTER,
		AC_SEARCH = SDLK_AC_SEARCH,
		AC_HOME = SDLK_AC_HOME,
		AC_BACK = SDLK_AC_BACK,
		AC_FORWARD = SDLK_AC_FORWARD,
		AC_STOP = SDLK_AC_STOP,
		AC_REFRESH = SDLK_AC_REFRESH,
		AC_BOOKMARKS = SDLK_AC_BOOKMARKS,
		BRIGHTNESSDOWN = SDLK_BRIGHTNESSDOWN,
		BRIGHTNESSUP = SDLK_BRIGHTNESSUP,
		DISPLAYSWITCH = SDLK_DISPLAYSWITCH,
		KBDILLUMTOGGLE = SDLK_KBDILLUMTOGGLE,
		KBDILLUMDOWN = SDLK_KBDILLUMDOWN,
		KBDILLUMUP = SDLK_KBDILLUMUP,
		EJECT = SDLK_EJECT,
		SLEEP = SDLK_SLEEP,
	};

	/**
	 * @enum keymod
	 *
	 * @brief Enumeration representing the keyboard modifiers.
	 *
	 * This enumeration represents the different keyboard modifiers, such as shift, control, alt, etc.
	 */
	enum class keymod : uint16_t {
		LSHIFT = KMOD_LSHIFT,
		RSHIFT = KMOD_RSHIFT,
		LCTRL = KMOD_LCTRL,
		RCTRL = KMOD_RCTRL,
		LALT = KMOD_LALT,
		RALT = KMOD_RALT,
		LGUI = KMOD_LGUI,
		RGUI = KMOD_RGUI,
		NUM = KMOD_NUM,
		CAPS = KMOD_CAPS,
		MODE = KMOD_MODE,

		CTRL = LCTRL | RCTRL,
		ALT = LALT | RALT,
		SHIFT = LSHIFT | RSHIFT,
		GUI = LGUI | RGUI
	};

	namespace detail {
		static inline constexpr std::array<event_type, 47> s_vals_of_event_type = {
			event_type::QUIT,
			event_type::APP_TERMINATING,
			event_type::APP_LOWMEMORY,
			event_type::APP_WILLENTERBACKGROUND,
			event_type::APP_DIDENTERBACKGROUND,
			event_type::APP_WILLENTERFOREGROUND,
			event_type::APP_DIDENTERFOREGROUND,
			event_type::WINDOWEVENT,
			event_type::SYSWMEVENT,
			event_type::KEYDOWN,
			event_type::KEYUP,
			event_type::TEXTEDITING,
			event_type::TEXTINPUT,
			event_type::KEYMAPCHANGED,
			event_type::MOUSEMOTION,
			event_type::MOUSEBUTTONDOWN,
			event_type::MOUSEBUTTONUP,
			event_type::MOUSEWHEEL,
			event_type::JOYAXISMOTION,
			event_type::JOYBALLMOTION,
			event_type::JOYHATMOTION,
			event_type::JOYBUTTONDOWN,
			event_type::JOYBUTTONUP,
			event_type::JOYDEVICEADDED,
			event_type::JOYDEVICEREMOVED,
			event_type::CONTROLLERAXISMOTION,
			event_type::CONTROLLERBUTTONDOWN,
			event_type::CONTROLLERBUTTONUP,
			event_type::CONTROLLERDEVICEADDED,
			event_type::CONTROLLERDEVICEREMOVED,
			event_type::CONTROLLERDEVICEREMAPPED,
			event_type::FINGERDOWN,
			event_type::FINGERUP,
			event_type::FINGERMOTION,
			event_type::DOLLARGESTURE,
			event_type::DOLLARRECORD,
			event_type::MULTIGESTURE,
			event_type::CLIPBOARDUPDATE,
			event_type::DROPFILE,
			event_type::DROPTEXT,
			event_type::DROPBEGIN,
			event_type::DROPCOMPLETE,
			event_type::AUDIODEVICEADDED,
			event_type::AUDIODEVICEREMOVED,
			event_type::RENDER_TARGETS_RESET,
			event_type::RENDER_DEVICE_RESET,
			event_type::USEREVENT,
		};
	}

	template <typename T>
	static inline constexpr const decltype (detail::s_vals_of_event_type)&
	values (typename std::enable_if<std::is_same_v<event_type, T>>::type* = nullptr) {
		return detail::s_vals_of_event_type;
	}

	template <typename T>
	static inline constexpr auto
	begin (typename std::enable_if<std::is_same_v<event_type, T>>::type* = nullptr) {
		return detail::s_vals_of_event_type.begin ();
	}

	template <typename T>
	static inline constexpr auto
	end (typename std::enable_if<std::is_same_v<event_type, T>>::type* = nullptr) {
		return detail::s_vals_of_event_type.end ();
	}

	namespace detail {
		static inline constexpr std::array<scancode, 242> s_vals_of_scancode = {
			scancode::UNKNOWN,
			scancode::A,
			scancode::B,
			scancode::C,
			scancode::D,
			scancode::E,
			scancode::F,
			scancode::G,
			scancode::H,
			scancode::I,
			scancode::J,
			scancode::K,
			scancode::L,
			scancode::M,
			scancode::N,
			scancode::O,
			scancode::P,
			scancode::Q,
			scancode::R,
			scancode::S,
			scancode::T,
			scancode::U,
			scancode::V,
			scancode::W,
			scancode::X,
			scancode::Y,
			scancode::Z,
			scancode::_1,
			scancode::_2,
			scancode::_3,
			scancode::_4,
			scancode::_5,
			scancode::_6,
			scancode::_7,
			scancode::_8,
			scancode::_9,
			scancode::_0,
			scancode::RETURN,
			scancode::ESCAPE,
			scancode::BACKSPACE,
			scancode::TAB,
			scancode::SPACE,
			scancode::MINUS,
			scancode::EQUALS,
			scancode::LEFTBRACKET,
			scancode::RIGHTBRACKET,
			scancode::BACKSLASH,
			scancode::NONUSHASH,
			scancode::SEMICOLON,
			scancode::APOSTROPHE,
			scancode::GRAVE,
			scancode::COMMA,
			scancode::PERIOD,
			scancode::SLASH,
			scancode::CAPSLOCK,
			scancode::F1,
			scancode::F2,
			scancode::F3,
			scancode::F4,
			scancode::F5,
			scancode::F6,
			scancode::F7,
			scancode::F8,
			scancode::F9,
			scancode::F10,
			scancode::F11,
			scancode::F12,
			scancode::PRINTSCREEN,
			scancode::SCROLLLOCK,
			scancode::PAUSE,
			scancode::INSERT,
			scancode::HOME,
			scancode::PAGEUP,
			scancode::DEL,
			scancode::END,
			scancode::PAGEDOWN,
			scancode::RIGHT,
			scancode::LEFT,
			scancode::DOWN,
			scancode::UP,
			scancode::NUMLOCKCLEAR,
			scancode::KP_DIVIDE,
			scancode::KP_MULTIPLY,
			scancode::KP_MINUS,
			scancode::KP_PLUS,
			scancode::KP_ENTER,
			scancode::KP_1,
			scancode::KP_2,
			scancode::KP_3,
			scancode::KP_4,
			scancode::KP_5,
			scancode::KP_6,
			scancode::KP_7,
			scancode::KP_8,
			scancode::KP_9,
			scancode::KP_0,
			scancode::KP_PERIOD,
			scancode::NONUSBACKSLASH,
			scancode::APPLICATION,
			scancode::POWER,
			scancode::KP_EQUALS,
			scancode::F13,
			scancode::F14,
			scancode::F15,
			scancode::F16,
			scancode::F17,
			scancode::F18,
			scancode::F19,
			scancode::F20,
			scancode::F21,
			scancode::F22,
			scancode::F23,
			scancode::F24,
			scancode::EXECUTE,
			scancode::HELP,
			scancode::MENU,
			scancode::SELECT,
			scancode::STOP,
			scancode::AGAIN,
			scancode::UNDO,
			scancode::CUT,
			scancode::COPY,
			scancode::PASTE,
			scancode::FIND,
			scancode::MUTE,
			scancode::VOLUMEUP,
			scancode::VOLUMEDOWN,
			scancode::KP_COMMA,
			scancode::KP_EQUALSAS400,
			scancode::INTERNATIONAL1,
			scancode::INTERNATIONAL2,
			scancode::INTERNATIONAL3,
			scancode::INTERNATIONAL4,
			scancode::INTERNATIONAL5,
			scancode::INTERNATIONAL6,
			scancode::INTERNATIONAL7,
			scancode::INTERNATIONAL8,
			scancode::INTERNATIONAL9,
			scancode::LANG1,
			scancode::LANG2,
			scancode::LANG3,
			scancode::LANG4,
			scancode::LANG5,
			scancode::LANG6,
			scancode::LANG7,
			scancode::LANG8,
			scancode::LANG9,
			scancode::ALTERASE,
			scancode::SYSREQ,
			scancode::CANCEL,
			scancode::CLEAR,
			scancode::PRIOR,
			scancode::RETURN2,
			scancode::SEPARATOR,
			scancode::KBD_OUT,
			scancode::OPER,
			scancode::CLEARAGAIN,
			scancode::CRSEL,
			scancode::EXSEL,
			scancode::KP_00,
			scancode::KP_000,
			scancode::THOUSANDSSEPARATOR,
			scancode::DECIMALSEPARATOR,
			scancode::CURRENCYUNIT,
			scancode::CURRENCYSUBUNIT,
			scancode::KP_LEFTPAREN,
			scancode::KP_RIGHTPAREN,
			scancode::KP_LEFTBRACE,
			scancode::KP_RIGHTBRACE,
			scancode::KP_TAB,
			scancode::KP_BACKSPACE,
			scancode::KP_A,
			scancode::KP_B,
			scancode::KP_C,
			scancode::KP_D,
			scancode::KP_E,
			scancode::KP_F,
			scancode::KP_XOR,
			scancode::KP_POWER,
			scancode::KP_PERCENT,
			scancode::KP_LESS,
			scancode::KP_GREATER,
			scancode::KP_AMPERSAND,
			scancode::KP_DBLAMPERSAND,
			scancode::KP_VERTICALBAR,
			scancode::KP_DBLVERTICALBAR,
			scancode::KP_COLON,
			scancode::KP_HASH,
			scancode::KP_SPACE,
			scancode::KP_AT,
			scancode::KP_EXCLAM,
			scancode::KP_MEMSTORE,
			scancode::KP_MEMRECALL,
			scancode::KP_MEMCLEAR,
			scancode::KP_MEMADD,
			scancode::KP_MEMSUBTRACT,
			scancode::KP_MEMMULTIPLY,
			scancode::KP_MEMDIVIDE,
			scancode::KP_PLUSMINUS,
			scancode::KP_CLEAR,
			scancode::KP_CLEARENTRY,
			scancode::KP_BINARY,
			scancode::KP_OCTAL,
			scancode::KP_DECIMAL,
			scancode::KP_HEXADECIMAL,
			scancode::LCTRL,
			scancode::LSHIFT,
			scancode::LALT,
			scancode::LGUI,
			scancode::RCTRL,
			scancode::RSHIFT,
			scancode::RALT,
			scancode::RGUI,
			scancode::MODE,
			scancode::AUDIONEXT,
			scancode::AUDIOPREV,
			scancode::AUDIOSTOP,
			scancode::AUDIOPLAY,
			scancode::AUDIOMUTE,
			scancode::MEDIASELECT,
			scancode::WWW,
			scancode::MAIL,
			scancode::CALCULATOR,
			scancode::COMPUTER,
			scancode::AC_SEARCH,
			scancode::AC_HOME,
			scancode::AC_BACK,
			scancode::AC_FORWARD,
			scancode::AC_STOP,
			scancode::AC_REFRESH,
			scancode::AC_BOOKMARKS,
			scancode::BRIGHTNESSDOWN,
			scancode::BRIGHTNESSUP,
			scancode::DISPLAYSWITCH,
			scancode::KBDILLUMTOGGLE,
			scancode::KBDILLUMDOWN,
			scancode::KBDILLUMUP,
			scancode::EJECT,
			scancode::SLEEP,
			scancode::APP1,
			scancode::APP2,
			scancode::NUM_SCANCODES,
		};
	}

	template <typename T>
	static inline constexpr const decltype (detail::s_vals_of_scancode)&
	values (typename std::enable_if<std::is_same_v<scancode, T>>::type* = nullptr) {
		return detail::s_vals_of_scancode;
	}

	template <typename T>
	static inline constexpr auto
	begin (typename std::enable_if<std::is_same_v<scancode, T>>::type* = nullptr) {
		return detail::s_vals_of_scancode.begin ();
	}

	template <typename T>
	static inline constexpr auto
	end (typename std::enable_if<std::is_same_v<scancode, T>>::type* = nullptr) {
		return detail::s_vals_of_scancode.end ();
	}

	namespace detail {
		static inline constexpr std::array<keycode, 235> s_vals_of_keycode = {
			keycode::UNKNOWN,
			keycode::RETURN,
			keycode::ESCAPE,
			keycode::BACKSPACE,
			keycode::TAB,
			keycode::SPACE,
			keycode::EXCLAIM,
			keycode::QUOTEDBL,
			keycode::PERCENT,
			keycode::DOLLAR,
			keycode::AMPERSAND,
			keycode::QUOTE,
			keycode::LEFTPAREN,
			keycode::RIGHTPAREN,
			keycode::ASTERISK,
			keycode::PLUS,
			keycode::COMMA,
			keycode::MINUS,
			keycode::PERIOD,
			keycode::SLASH,
			keycode::_0,
			keycode::_1,
			keycode::_2,
			keycode::_3,
			keycode::_4,
			keycode::_5,
			keycode::_6,
			keycode::_7,
			keycode::_8,
			keycode::_9,
			keycode::COLON,
			keycode::SEMICOLON,
			keycode::LESS,
			keycode::EQUALS,
			keycode::GREATER,
			keycode::QUESTION,
			keycode::AT,
			keycode::LEFTBRACKET,
			keycode::BACKSLASH,
			keycode::RIGHTBRACKET,
			keycode::CARET,
			keycode::UNDERSCORE,
			keycode::BACKQUOTE,
			keycode::a,
			keycode::b,
			keycode::c,
			keycode::d,
			keycode::e,
			keycode::f,
			keycode::g,
			keycode::h,
			keycode::i,
			keycode::j,
			keycode::k,
			keycode::l,
			keycode::m,
			keycode::n,
			keycode::o,
			keycode::p,
			keycode::q,
			keycode::r,
			keycode::s,
			keycode::t,
			keycode::u,
			keycode::v,
			keycode::w,
			keycode::x,
			keycode::y,
			keycode::z,
			keycode::CAPSLOCK,
			keycode::F1,
			keycode::F2,
			keycode::F3,
			keycode::F4,
			keycode::F5,
			keycode::F6,
			keycode::F7,
			keycode::F8,
			keycode::F9,
			keycode::F10,
			keycode::F11,
			keycode::F12,
			keycode::PRINTSCREEN,
			keycode::SCROLLLOCK,
			keycode::PAUSE,
			keycode::INSERT,
			keycode::HOME,
			keycode::PAGEUP,
			keycode::DEL,
			keycode::END,
			keycode::PAGEDOWN,
			keycode::RIGHT,
			keycode::LEFT,
			keycode::DOWN,
			keycode::UP,
			keycode::NUMLOCKCLEAR,
			keycode::KP_DIVIDE,
			keycode::KP_MULTIPLY,
			keycode::KP_MINUS,
			keycode::KP_PLUS,
			keycode::KP_ENTER,
			keycode::KP_1,
			keycode::KP_2,
			keycode::KP_3,
			keycode::KP_4,
			keycode::KP_5,
			keycode::KP_6,
			keycode::KP_7,
			keycode::KP_8,
			keycode::KP_9,
			keycode::KP_0,
			keycode::KP_PERIOD,
			keycode::APPLICATION,
			keycode::POWER,
			keycode::KP_EQUALS,
			keycode::F13,
			keycode::F14,
			keycode::F15,
			keycode::F16,
			keycode::F17,
			keycode::F18,
			keycode::F19,
			keycode::F20,
			keycode::F21,
			keycode::F22,
			keycode::F23,
			keycode::F24,
			keycode::EXECUTE,
			keycode::HELP,
			keycode::MENU,
			keycode::SELECT,
			keycode::STOP,
			keycode::AGAIN,
			keycode::UNDO,
			keycode::CUT,
			keycode::COPY,
			keycode::PASTE,
			keycode::FIND,
			keycode::MUTE,
			keycode::VOLUMEUP,
			keycode::VOLUMEDOWN,
			keycode::KP_COMMA,
			keycode::KP_EQUALSAS400,
			keycode::ALTERASE,
			keycode::SYSREQ,
			keycode::CANCEL,
			keycode::CLEAR,
			keycode::PRIOR,
			keycode::RETURN2,
			keycode::SEPARATOR,
			keycode::KBD_OUT,
			keycode::OPER,
			keycode::CLEARAGAIN,
			keycode::CRSEL,
			keycode::EXSEL,
			keycode::KP_00,
			keycode::KP_000,
			keycode::THOUSANDSSEPARATOR,
			keycode::DECIMALSEPARATOR,
			keycode::CURRENCYUNIT,
			keycode::CURRENCYSUBUNIT,
			keycode::KP_LEFTPAREN,
			keycode::KP_RIGHTPAREN,
			keycode::KP_LEFTBRACE,
			keycode::KP_RIGHTBRACE,
			keycode::KP_TAB,
			keycode::KP_BACKSPACE,
			keycode::KP_A,
			keycode::KP_B,
			keycode::KP_C,
			keycode::KP_D,
			keycode::KP_E,
			keycode::KP_F,
			keycode::KP_XOR,
			keycode::KP_POWER,
			keycode::KP_PERCENT,
			keycode::KP_LESS,
			keycode::KP_GREATER,
			keycode::KP_AMPERSAND,
			keycode::KP_DBLAMPERSAND,
			keycode::KP_VERTICALBAR,
			keycode::KP_DBLVERTICALBAR,
			keycode::KP_COLON,
			keycode::KP_HASH,
			keycode::KP_SPACE,
			keycode::KP_AT,
			keycode::KP_EXCLAM,
			keycode::KP_MEMSTORE,
			keycode::KP_MEMRECALL,
			keycode::KP_MEMCLEAR,
			keycode::KP_MEMADD,
			keycode::KP_MEMSUBTRACT,
			keycode::KP_MEMMULTIPLY,
			keycode::KP_MEMDIVIDE,
			keycode::KP_PLUSMINUS,
			keycode::KP_CLEAR,
			keycode::KP_CLEARENTRY,
			keycode::KP_BINARY,
			keycode::KP_OCTAL,
			keycode::KP_DECIMAL,
			keycode::KP_HEXADECIMAL,
			keycode::LCTRL,
			keycode::LSHIFT,
			keycode::LALT,
			keycode::LGUI,
			keycode::RCTRL,
			keycode::RSHIFT,
			keycode::RALT,
			keycode::RGUI,
			keycode::MODE,
			keycode::AUDIONEXT,
			keycode::AUDIOPREV,
			keycode::AUDIOSTOP,
			keycode::AUDIOPLAY,
			keycode::AUDIOMUTE,
			keycode::MEDIASELECT,
			keycode::WWW,
			keycode::MAIL,
			keycode::CALCULATOR,
			keycode::COMPUTER,
			keycode::AC_SEARCH,
			keycode::AC_HOME,
			keycode::AC_BACK,
			keycode::AC_FORWARD,
			keycode::AC_STOP,
			keycode::AC_REFRESH,
			keycode::AC_BOOKMARKS,
			keycode::BRIGHTNESSDOWN,
			keycode::BRIGHTNESSUP,
			keycode::DISPLAYSWITCH,
			keycode::KBDILLUMTOGGLE,
			keycode::KBDILLUMDOWN,
			keycode::KBDILLUMUP,
			keycode::EJECT,
			keycode::SLEEP,
		};
	}

	template <typename T>
	static inline constexpr const decltype (detail::s_vals_of_keycode)&
	values (typename std::enable_if<std::is_same_v<keycode, T>>::type* = nullptr) {
		return detail::s_vals_of_keycode;
	}

	template <typename T>
	static inline constexpr auto
	begin (typename std::enable_if<std::is_same_v<keycode, T>>::type* = nullptr) {
		return detail::s_vals_of_keycode.begin ();
	}

	template <typename T>
	static inline constexpr auto
	end (typename std::enable_if<std::is_same_v<keycode, T>>::type* = nullptr) {
		return detail::s_vals_of_keycode.end ();
	}

	namespace detail {
		static inline constexpr std::array<keymod, 15> s_vals_of_keymod = {
			keymod::LSHIFT,
			keymod::RSHIFT,
			keymod::LCTRL,
			keymod::RCTRL,
			keymod::LALT,
			keymod::RALT,
			keymod::LGUI,
			keymod::RGUI,
			keymod::NUM,
			keymod::CAPS,
			keymod::MODE,
			keymod::CTRL,
			keymod::ALT,
			keymod::SHIFT,
			keymod::GUI,
		};
	}

	template <typename T>
	static inline constexpr const decltype (detail::s_vals_of_keymod)&
	values (typename std::enable_if<std::is_same_v<keymod, T>>::type* = nullptr) {
		return detail::s_vals_of_keymod;
	}

	template <typename T>
	static inline constexpr auto
	begin (typename std::enable_if<std::is_same_v<keymod, T>>::type* = nullptr) {
		return detail::s_vals_of_keymod.begin ();
	}

	template <typename T>
	static inline constexpr auto
	end (typename std::enable_if<std::is_same_v<keymod, T>>::type* = nullptr) {
		return detail::s_vals_of_keymod.end ();
	}

	d_SDLPP_OSTREAM(event_action);
	d_SDLPP_OSTREAM(event_type);
	d_SDLPP_OSTREAM(scancode);
	d_SDLPP_OSTREAM(keycode);
	d_SDLPP_OSTREAM(keymod);
}

#endif //NEUTRINO_SDL_EVENTS_EVENT_TYPES_HH
