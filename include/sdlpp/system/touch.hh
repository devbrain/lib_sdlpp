//
// Created by igor on 6/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_TOUCH_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_TOUCH_HH_

#include <string>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <bsw/mp/primitive.hh>


namespace neutrino::sdl {
	class touch_device {
	 public:
		using id_t = primitives::primitive<SDL_TouchID>;

		enum type_t {
			DIRECT = SDL_TOUCH_DEVICE_DIRECT,            /* touch screen with window-relative coordinates */
			INDIRECT_ABSOLUTE = SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE, /* trackpad with absolute device coordinates */
			INDIRECT_RELATIVE = SDL_TOUCH_DEVICE_INDIRECT_RELATIVE  /* trackpad with screen cursor-relative coordinates */
		};

		static std::size_t count () {
			return static_cast<std::size_t>(SDL_GetNumTouchDevices());
		}

		static id_t get_id(std::size_t idx) {
			return SAFE_SDL_CALL(SDL_GetTouchDevice, static_cast<int>(idx));
		}

		static type_t get_type(id_t touch_id) {
			auto rc = SDL_GetTouchDeviceType (touch_id.get());
			if (rc == SDL_TOUCH_DEVICE_INVALID) {
				RAISE_SDL_EX("Invalid touch id");
			}
			return static_cast<type_t>(rc);
		}

		static std::string get_name(std::size_t idx) {
			return SAFE_SDL_CALL(SDL_GetTouchName, static_cast<int>(idx));
		}

		static std::size_t count_fingers(id_t touch_id) {
			auto rc = SDL_GetNumTouchFingers (touch_id.get());
			if (rc == 0) {
				RAISE_SDL_EX();
			}
			return static_cast<std::size_t >(rc);
		}

	};
}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_TOUCH_HH_
