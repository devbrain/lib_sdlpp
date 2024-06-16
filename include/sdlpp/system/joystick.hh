//
// Created by igor on 6/12/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_JOYSTICK_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_JOYSTICK_HH_

#include <string>

#include <strong_type/strong_type.hpp>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/joystick_id.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <sdlpp/input/joystick.hh>
#include <bsw/uuid.hh>

namespace neutrino::sdl {

	class joystick_device  {
	 public:

		static joystick_device_id_t count() {
			const auto rc = SAFE_SDL_CALL(SDL_NumJoysticks);
			return joystick_device_id_t{static_cast<std::size_t >(rc)};
		}

		static object<SDL_Joystick> open (joystick_device_id_t idx) {
			return {SAFE_SDL_CALL(SDL_JoystickOpen, static_cast<int>(idx.value_of ())), true};
		}

		static object<SDL_Joystick> open (joystick_id_t idx) {
			return {SAFE_SDL_CALL(SDL_JoystickFromInstanceID, static_cast<int>(idx.value_of ())), true};
		}

		static object<SDL_Joystick> open (joystick_player_index_t idx) {
			return {SAFE_SDL_CALL(SDL_JoystickFromPlayerIndex, static_cast<int>(idx.value_of ())), true};
		}

		[[nodiscard]] static std::string get_name(joystick_device_id_t idx) {
			return SAFE_SDL_CALL(SDL_JoystickNameForIndex, static_cast<int>(idx.value_of()));
		}

		[[nodiscard]] static std::string get_path(joystick_device_id_t idx) {
			return SAFE_SDL_CALL(SDL_JoystickPathForIndex, static_cast<int>(idx.value_of()));
		}
		[[nodiscard]] static bool is_virtual(joystick_device_id_t idx) {
			return SAFE_SDL_CALL(SDL_JoystickIsVirtual, static_cast<int>(idx.value_of())) == SDL_TRUE;
		}

		[[nodiscard]] static bsw::uuid get_uuid(joystick_device_id_t idx) {
			char buff[128] = {0};
			auto jid = SDL_JoystickGetDeviceGUID (static_cast<int>(idx.value_of()));
			SDL_GUIDToString (jid, buff, sizeof (buff));
			bsw::uuid uuid(buff);
			if (uuid == bsw::uuid::null()) {
				RAISE_SDL_EX();
			}
			return uuid;
		}

		[[nodiscard]] static std::optional<joystick_player_index_t> get_player_index(joystick_device_id_t idx) noexcept {
			auto rc = SDL_JoystickGetDevicePlayerIndex(static_cast<int>(idx.value_of()));
			if (rc >= 0) {
				return joystick_player_index_t{rc};
			}
			return std::nullopt;
		}

		[[nodiscard]] static std::optional<uint16_t> get_product(joystick_device_id_t idx) {
			auto rc = SDL_JoystickGetDeviceProduct (static_cast<int>(idx.value_of()));
			if (rc == 0) {
				return std::nullopt;
			}
			return rc;
		}

		[[nodiscard]] static std::optional<uint16_t> get_vendor(joystick_device_id_t idx) {
			auto rc = SDL_JoystickGetDeviceVendor (static_cast<int>(idx.value_of()));
			if (rc == 0) {
				return std::nullopt;
			}
			return rc;
		}

		[[nodiscard]] static std::optional<uint16_t> get_product_version(joystick_device_id_t idx) {
			auto rc = SDL_JoystickGetDeviceProductVersion(static_cast<int>(idx.value_of()));
			if (rc == 0) {
				return std::nullopt;
			}
			return rc;
		}

		[[nodiscard]] static joystick::type get_type(joystick_device_id_t idx) {
			return static_cast<joystick::type>(SDL_JoystickGetDeviceType (static_cast<int>(idx.value_of())));
		}

		[[nodiscard]] static bool is_game_controller(joystick_device_id_t idx) {
			return SDL_TRUE == SDL_IsGameController (static_cast<int>(idx.value_of()));
		}

		[[nodiscard]] static std::string get_game_controller_name(joystick_device_id_t idx) {
			return SAFE_SDL_CALL(SDL_GameControllerNameForIndex, static_cast<int>(idx.value_of()));
		}

		[[nodiscard]] static std::string get_game_controller_path(joystick_device_id_t idx) {
			return SAFE_SDL_CALL(SDL_GameControllerPathForIndex, static_cast<int>(idx.value_of()));
		}


		static void lock() {
			SDL_LockJoysticks();
		}

		static void unlock() {
			SDL_UnlockJoysticks();
		}
	};

}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_JOYSTICK_HH_
