//
// Created by igor on 6/12/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_JOYSTICK_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_JOYSTICK_HH_

#include <string>
#include <optional>
#include <tuple>
#include <chrono>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <sdlpp/events/system_events.hh>
#include <sdlpp/video/color.hh>
#include <bsw/mp/primitive.hh>
#include <bsw/uuid.hh>

namespace neutrino::sdl {
	class joystick : public object<SDL_Joystick> {
	 public:
		using id_t = primitives::primitive<SDL_JoystickID>;
		using player_index_t = primitives::primitive<int>;

		enum class power_level_t {
			UNKNOWN = SDL_JOYSTICK_POWER_UNKNOWN,
			EMPTY = SDL_JOYSTICK_POWER_EMPTY,
			LOW = SDL_JOYSTICK_POWER_LOW,
			MEDIUM = SDL_JOYSTICK_POWER_MEDIUM,
			FULL = SDL_JOYSTICK_POWER_FULL,
			WIRED = SDL_JOYSTICK_POWER_WIRED,
			MAX = SDL_JOYSTICK_POWER_MAX
		};

		enum class type_t {
			UNKNOWN = SDL_JOYSTICK_TYPE_UNKNOWN,
			GAMECONTROLLER = SDL_JOYSTICK_TYPE_GAMECONTROLLER,
			WHEEL = SDL_JOYSTICK_TYPE_WHEEL,
			ARCADE_STICK = SDL_JOYSTICK_TYPE_ARCADE_STICK,
			FLIGHT_STICK = SDL_JOYSTICK_TYPE_FLIGHT_STICK,
			DANCE_PAD = SDL_JOYSTICK_TYPE_DANCE_PAD,
			GUITAR = SDL_JOYSTICK_TYPE_GUITAR,
			DRUM_KIT = SDL_JOYSTICK_TYPE_DRUM_KIT,
			ARCADE_PAD = SDL_JOYSTICK_TYPE_ARCADE_PAD,
			THROTTLE = SDL_JOYSTICK_TYPE_THROTTLE
		};

		static std::size_t count() {
			const auto rc = SAFE_SDL_CALL(SDL_NumJoysticks);
			return static_cast<std::size_t >(rc);
		}

		static joystick open(std::size_t idx) {
			return joystick(static_cast<int>(idx));
		}

		static joystick open(id_t jid) {
			return joystick(jid);
		}

		[[nodiscard]] static std::string get_name(std::size_t idx) {
			return SAFE_SDL_CALL(SDL_JoystickNameForIndex, static_cast<int>(idx));
		}

		[[nodiscard]] static std::string get_path(std::size_t idx) {
			return SAFE_SDL_CALL(SDL_JoystickPathForIndex, static_cast<int>(idx));
		}
		[[nodiscard]] static bool is_virtual(std::size_t idx) {
			return SAFE_SDL_CALL(SDL_JoystickIsVirtual, static_cast<int>(idx)) == SDL_TRUE;
		}

		[[nodiscard]] static bsw::uuid get_uuid(std::size_t idx) {
			char buff[128] = {0};
			auto jid = SDL_JoystickGetDeviceGUID (static_cast<int>(idx));
			SDL_GUIDToString (jid, buff, sizeof (buff));
			bsw::uuid uuid(buff);
			if (uuid == bsw::uuid::null()) {
				RAISE_SDL_EX();
			}
			return uuid;
		}

		[[nodiscard]] static id_t get_id(std::size_t idx) {
			auto rc = SAFE_SDL_CALL(SDL_JoystickGetDeviceInstanceID, static_cast<int>(idx));
			return rc;
		}

		[[nodiscard]] static std::optional<player_index_t> get_player_index(std::size_t idx) noexcept {
			auto rc = SDL_JoystickGetDevicePlayerIndex(static_cast<int>(idx));
			if (rc >= 0) {
				return rc;
			}
			return std::nullopt;
		}

		[[nodiscard]] static std::optional<uint16_t> get_product(std::size_t idx) {
			auto rc = SDL_JoystickGetDeviceProduct (static_cast<int>(idx));
			if (rc == 0) {
				return std::nullopt;
			}
			return rc;
		}

		[[nodiscard]] static std::optional<uint16_t> get_vendor(std::size_t idx) {
			auto rc = SDL_JoystickGetDeviceVendor (static_cast<int>(idx));
			if (rc == 0) {
				return std::nullopt;
			}
			return rc;
		}

		[[nodiscard]] static std::optional<uint16_t> get_product_version(std::size_t idx) {
			auto rc = SDL_JoystickGetDeviceProductVersion(static_cast<int>(idx));
			if (rc == 0) {
				return std::nullopt;
			}
			return rc;
		}

		[[nodiscard]] static type_t get_type(std::size_t idx) {
			return static_cast<type_t>(SDL_JoystickGetDeviceType (static_cast<int>(idx)));
		}

		[[nodiscard]] static bool is_game_controller(std::size_t idx) {
			return SDL_TRUE == SDL_IsGameController (static_cast<int>(idx));
		}

		[[nodiscard]] static std::string get_game_controller_name(std::size_t idx) {
			return SAFE_SDL_CALL(SDL_GameControllerNameForIndex, static_cast<int>(idx));
		}

		[[nodiscard]] static std::string get_game_controller_path(std::size_t idx) {
			return SAFE_SDL_CALL(SDL_GameControllerPathForIndex, static_cast<int>(idx));
		}


		static void lock();
		static void unlock();

		[[nodiscard]] id_t get_id() const;

		[[nodiscard]] std::string get_name() const;
		[[nodiscard]] std::string get_path() const;
		[[nodiscard]] std::string get_serial() const;
		[[nodiscard]] std::optional<uint16_t> get_firmware_version() const;
		[[nodiscard]] std::optional<uint16_t> get_product() const;
		[[nodiscard]] std::optional<uint16_t> get_product_version() const;
		[[nodiscard]] std::optional<uint16_t> get_vendor() const;
		[[nodiscard]] bool has_led() const;

		void set_led(uint8_t r, uint8_t g, uint8_t b);
		void set_led(const color& c);



		[[nodiscard]] power_level_t get_power_level() const noexcept;
		[[nodiscard]] std::optional<player_index_t> get_player_index() const noexcept;
		void set_player_index(player_index_t idx);
		void clear_player_index();

		[[nodiscard]] std::size_t count_axes() const;
		[[nodiscard]] int16_t get_axis(std::size_t axis_id) const;
		[[nodiscard]] std::optional<int16_t> get_axis_initial_state(std::size_t axis_id) const;

		[[nodiscard]] std::size_t count_balls() const;
		[[nodiscard]] std::tuple<int, int> get_ball(std::size_t ball_id) const;

		[[nodiscard]] std::size_t count_buttons() const;
		[[nodiscard]] bool get_button(std::size_t button_id) const;

		[[nodiscard]] std::size_t count_hats() const;
		[[nodiscard]] events::joystick_hat_state get_hat(std::size_t hat_idx) const;

		[[nodiscard]] bool has_rumble() const;
		void rumble_triggers(uint16_t left, uint16_t right, std::chrono::milliseconds duration);
		void send_effect(const void* data, std::size_t size);


	 private:
		explicit joystick(int index);
		explicit joystick(id_t jid);
	};

	d_SDLPP_OSTREAM(joystick::power_level_t);
	d_SDLPP_OSTREAM(joystick::type_t);

	inline
	joystick::joystick (int index)
	: object<SDL_Joystick>(SAFE_SDL_CALL(SDL_JoystickOpen, index), true)
	{

	}

	inline
	joystick::joystick (joystick::id_t jid)
		: object<SDL_Joystick>(SAFE_SDL_CALL(SDL_JoystickFromInstanceID, jid.get()), true)
	{
	}

	inline
	joystick::id_t joystick::get_id () const {
		return SAFE_SDL_CALL(SDL_JoystickInstanceID,const_handle());
	}

	inline
	std::string joystick::get_name () const {
		return SAFE_SDL_CALL(SDL_JoystickName, const_handle());
	}

	inline
	std::string joystick::get_path () const {
		return SAFE_SDL_CALL(SDL_JoystickPath, const_handle());
	}

	inline
	joystick::power_level_t joystick::get_power_level () const noexcept {
		return static_cast<power_level_t>(SDL_JoystickCurrentPowerLevel (const_cast<SDL_Joystick *>(const_handle())));
	}

	inline
	std::optional<joystick::player_index_t> joystick::get_player_index () const noexcept {
		auto rc = SDL_JoystickGetPlayerIndex(const_handle());
		if (rc >= 0) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::size_t joystick::count_axes () const {
		auto rc = SAFE_SDL_CALL(SDL_JoystickNumAxes, const_handle());
		return static_cast<std::size_t>(rc);
	}

	inline
	int16_t joystick::get_axis (std::size_t axis_id) const {
		auto rc = SDL_JoystickGetAxis (const_handle(), static_cast<int>(axis_id));
		if (rc == 0) {
			RAISE_SDL_EX();
		}
		return rc;
	}

	inline
	std::optional<int16_t> joystick::get_axis_initial_state (std::size_t axis_id) const {
		Sint16 state;
		if (SDL_TRUE == SDL_JoystickGetAxisInitialState (const_handle(), static_cast<int>(axis_id), &state)) {
			return state;
		}
		return std::nullopt;
	}

	inline
	std::size_t joystick::count_balls () const {
		auto rc = SAFE_SDL_CALL(SDL_JoystickNumBalls, const_handle());
		return static_cast<std::size_t>(rc);
	}

	inline
	std::tuple<int, int> joystick::get_ball (std::size_t ball_id) const {
		int dx;
		int dy;
		SAFE_SDL_CALL(SDL_JoystickGetBall, const_handle(), static_cast<int>(ball_id), &dx, &dy);
		return {dx, dy};
	}

	inline
	std::size_t joystick::count_buttons () const {
		auto rc = SAFE_SDL_CALL(SDL_JoystickNumButtons, const_handle());
		return static_cast<std::size_t>(rc);
	}

	inline
	bool joystick::get_button (std::size_t button_id) const {
		return SDL_JoystickGetButton (const_handle(), static_cast<int>(button_id));
	}

	inline
	std::optional<uint16_t> joystick::get_firmware_version () const {
		auto rc = SDL_JoystickGetFirmwareVersion (const_handle());
		if (rc != 0) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::size_t joystick::count_hats () const {
		auto rc = SAFE_SDL_CALL(SDL_JoystickNumHats, const_handle());
		return static_cast<std::size_t>(rc);
	}

	inline
	events::joystick_hat_state joystick::get_hat (std::size_t hat_idx) const {
		return static_cast<events::joystick_hat_state>(SDL_JoystickGetHat(const_handle(), static_cast<int>(hat_idx)));
	}

	inline
	std::optional<uint16_t> joystick::get_product() const{
		auto rc = SDL_JoystickGetProduct (const_handle());
		if (rc == 0) {
			return std::nullopt;
		}
		return rc;
	}

	inline
	std::optional<uint16_t> joystick::get_product_version() const {
		auto rc = SDL_JoystickGetProductVersion(const_handle());
		if (rc == 0) {
			return std::nullopt;
		}
		return rc;
	}

	inline
	std::optional<uint16_t> joystick::get_vendor() const {
		auto rc = SDL_JoystickGetVendor (const_handle());
		if (rc == 0) {
			return std::nullopt;
		}
		return rc;
	}

	inline
	std::string joystick::get_serial () const {
		const char * rc = SDL_JoystickGetSerial(const_handle());
		if (rc) {
			return rc;
		}
		return {};
	}

	inline
	bool joystick::has_led () const {
		return SDL_TRUE == SDL_JoystickHasLED (const_handle());
	}

	inline
	void joystick::set_led (uint8_t r, uint8_t g, uint8_t b) {
		SAFE_SDL_CALL(SDL_JoystickSetLED, handle(), r, g, b);
	}

	inline
	void joystick::set_led (const color& c) {
		set_led (c.r, c.g, c.b);
	}

	inline
	bool joystick::has_rumble() const {
		return SDL_TRUE == SDL_JoystickHasRumble (const_handle());
	}

	inline
	void joystick::rumble_triggers (uint16_t left, uint16_t right, std::chrono::milliseconds duration) {
		SAFE_SDL_CALL(SDL_JoystickRumbleTriggers, handle(), left, right, duration.count());
	}

	inline
	void joystick::send_effect (const void* data, std::size_t size) {
		SAFE_SDL_CALL(SDL_JoystickSendEffect, handle(), data, static_cast<int>(size));
	}

	inline
	void joystick::set_player_index (player_index_t idx) {
		SDL_JoystickSetPlayerIndex (handle(), idx.get());
	}

	inline
	void joystick::clear_player_index () {
		SDL_JoystickSetPlayerIndex (handle(), -1);
	}

	inline
	void joystick::lock () {
		SDL_LockJoysticks();
	}

	inline
	void joystick::unlock ()  {
		SDL_UnlockJoysticks();
	}
}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_JOYSTICK_HH_
