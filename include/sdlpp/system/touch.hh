//
// Created by igor on 6/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_TOUCH_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_TOUCH_HH_

#include <array>
#include <type_traits>
#include <string>
#include <optional>
#include <strong_type/strong_type.hpp>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/ostreamops.hh>


namespace neutrino::sdl {
	using touch_device_t = strong::type<std::size_t, struct _touch_device_, strong::bicrementable, strong::ordered, strong::ostreamable>;
	using touch_id_t = strong::type<SDL_TouchID, struct _touch_id_, strong::bicrementable, strong::ordered, strong::ostreamable>;
	using touch_finger_t = strong::type<std::size_t, struct _touch_finger_, strong::bicrementable, strong::ordered, strong::ostreamable>;

	struct SDLPP_EXPORT finger_data {
		float x;
		float y;
		float pressure;
	};

	class touch_device {
	 public:
		enum type {
			DIRECT = SDL_TOUCH_DEVICE_DIRECT,            /* touch screen with window-relative coordinates */
			INDIRECT_ABSOLUTE = SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE, /* trackpad with absolute device coordinates */
			INDIRECT_RELATIVE = SDL_TOUCH_DEVICE_INDIRECT_RELATIVE  /* trackpad with screen cursor-relative coordinates */
		};

		static touch_device_t count () {
			return touch_device_t {static_cast<std::size_t>(SDL_GetNumTouchDevices())};
		}

		static touch_id_t get_id(touch_device_t idx) {
			return touch_id_t{SAFE_SDL_CALL(SDL_GetTouchDevice, static_cast<int>(idx.value_of()))};
		}

		static type get_type (touch_id_t touch_id) {
			auto rc = SDL_GetTouchDeviceType (touch_id.value_of());
			if (rc == SDL_TOUCH_DEVICE_INVALID) {
				RAISE_SDL_EX("Invalid touch id");
			}
			return static_cast<type>(rc);
		}

		static std::string get_name(touch_device_t idx) {
			return SAFE_SDL_CALL(SDL_GetTouchName, static_cast<int>(idx.value_of()));
		}

		static touch_finger_t count_fingers(touch_id_t touch_id) {
			auto rc = SDL_GetNumTouchFingers (touch_id.value_of());
			if (rc == 0) {
				RAISE_SDL_EX("Failed to get number of fingers");
			}
			return touch_finger_t {static_cast<std::size_t >(rc)};
		}

		static std::optional<finger_data> get_finger(touch_id_t touch_id, touch_finger_t idx) {
			auto* rc = SDL_GetTouchFinger (touch_id.value_of(), static_cast<int>(idx.value_of()));
			if (rc) {
				return finger_data{rc->x, rc->y, rc->pressure};
			}
			return std::nullopt;
		}
	};



	namespace detail {
		static inline constexpr std::array<touch_device::type, 3> s_vals_of_touch_device_type = {
			touch_device::type::DIRECT,
			touch_device::type::INDIRECT_ABSOLUTE,
			touch_device::type::INDIRECT_RELATIVE,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_touch_device_type)&
	values(typename std::enable_if<std::is_same_v<touch_device::type, T>>::type* = nullptr) {
		return detail::s_vals_of_touch_device_type;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<touch_device::type, T>>::type* = nullptr) {
		return detail::s_vals_of_touch_device_type.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<touch_device::type, T>>::type* = nullptr) {
		return detail::s_vals_of_touch_device_type.end();
	}

	d_SDLPP_OSTREAM(touch_device::type);

}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_TOUCH_HH_
