//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_JOYSTIC_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_JOYSTIC_HH_
#include <string>
#include <optional>
#include <tuple>
#include <chrono>
#include <array>
#include <strong_type/strong_type.hpp>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/detail/joystick_id.hh>
#include <sdlpp/events/system_events.hh>

namespace neutrino::sdl {
	using joystick_axis_t = strong::type <std::size_t,
	                                      struct _joystick_axis_,
	                                      strong::bicrementable,
	                                      strong::ordered,
	                                      strong::ostreamable>;
	using joystick_ball_t = strong::type <std::size_t,
	                                      struct _joystick_ball_,
	                                      strong::bicrementable,
	                                      strong::ordered,
	                                      strong::ostreamable>;
	using joystick_button_t = strong::type <std::size_t,
	                                        struct _joystick_button_,
	                                        strong::bicrementable,
	                                        strong::ordered,
	                                        strong::ostreamable>;
	using joystick_hat_t = strong::type <std::size_t,
	                                     struct _joystick_hat_,
	                                     strong::bicrementable,
	                                     strong::ordered,
	                                     strong::ostreamable>;

	class joystick : public object <SDL_Joystick> {
		public:
			enum class power_level {
				UNKNOWN = SDL_JOYSTICK_POWER_UNKNOWN,
				EMPTY = SDL_JOYSTICK_POWER_EMPTY,
				LOW = SDL_JOYSTICK_POWER_LOW,
				MEDIUM = SDL_JOYSTICK_POWER_MEDIUM,
				FULL = SDL_JOYSTICK_POWER_FULL,
				WIRED = SDL_JOYSTICK_POWER_WIRED,
				MAX = SDL_JOYSTICK_POWER_MAX
			};

			enum class type {
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

		public:
			joystick() = default;
			joystick& operator=(object <SDL_Joystick>&& other) noexcept;
			joystick& operator=(joystick&& other) noexcept;

			joystick& operator=(const joystick& other) = delete;
			joystick(const joystick& other) = delete;

			[[nodiscard]] joystick_id_t get_id() const;

			[[nodiscard]] std::string get_name() const;
			[[nodiscard]] std::string get_path() const;
			[[nodiscard]] std::string get_serial() const;
			[[nodiscard]] std::optional <uint16_t> get_firmware_version() const;
			[[nodiscard]] std::optional <uint16_t> get_product() const;
			[[nodiscard]] std::optional <uint16_t> get_product_version() const;
			[[nodiscard]] std::optional <uint16_t> get_vendor() const;
			[[nodiscard]] bool has_led() const;

			void set_led(uint8_t r, uint8_t g, uint8_t b);
			void set_led(const color& c);

			[[nodiscard]] power_level get_power_level() const noexcept;
			[[nodiscard]] std::optional <joystick_player_index_t> get_player_index() const noexcept;
			void set_player_index(joystick_player_index_t idx);
			void clear_player_index();

			[[nodiscard]] joystick_axis_t count_axes() const;
			[[nodiscard]] int16_t get_axis(joystick_axis_t axis_id) const;
			[[nodiscard]] std::optional <int16_t> get_axis_initial_state(joystick_axis_t axis_id) const;

			[[nodiscard]] joystick_ball_t count_balls() const;
			[[nodiscard]] std::tuple <int, int> get_ball(joystick_ball_t ball_id) const;

			[[nodiscard]] joystick_button_t count_buttons() const;
			[[nodiscard]] bool get_button(joystick_button_t button_id) const;

			[[nodiscard]] joystick_hat_t count_hats() const;
			[[nodiscard]] events::joystick_hat_state get_hat(joystick_hat_t hat_idx) const;

			[[nodiscard]] bool has_rumble() const;
			void rumble_triggers(uint16_t left, uint16_t right, std::chrono::milliseconds duration);
			void send_effect(const void* data, std::size_t size);

			[[nodiscard]] bool is_haptic() const;
			[[nodiscard]] object <SDL_Haptic> as_haptic() const;
			[[nodiscard]] bool is_game_cotroller() const;
			[[nodiscard]] object <SDL_GameController> as_game_cotroller() const;
	};

	namespace detail {
		static inline constexpr std::array <joystick::power_level, 7> s_vals_of_joystick_power_level = {
			joystick::power_level::UNKNOWN,
			joystick::power_level::EMPTY,
			joystick::power_level::LOW,
			joystick::power_level::MEDIUM,
			joystick::power_level::FULL,
			joystick::power_level::WIRED,
			joystick::power_level::MAX,
		};
	}

	template<typename T>
	static constexpr const decltype(detail::s_vals_of_joystick_power_level)&
	values(std::enable_if_t <std::is_same_v <joystick::power_level, T>>* = nullptr) {
		return detail::s_vals_of_joystick_power_level;
	}

	template<typename T>
	static constexpr auto
	begin(std::enable_if_t <std::is_same_v <joystick::power_level, T>>* = nullptr) {
		return detail::s_vals_of_joystick_power_level.begin();
	}

	template<typename T>
	static constexpr auto
	end(std::enable_if_t <std::is_same_v <joystick::power_level, T>>* = nullptr) {
		return detail::s_vals_of_joystick_power_level.end();
	}

	namespace detail {
		static inline constexpr std::array <joystick::type, 10> s_vals_of_joystick_type = {
			joystick::type::UNKNOWN,
			joystick::type::GAMECONTROLLER,
			joystick::type::WHEEL,
			joystick::type::ARCADE_STICK,
			joystick::type::FLIGHT_STICK,
			joystick::type::DANCE_PAD,
			joystick::type::GUITAR,
			joystick::type::DRUM_KIT,
			joystick::type::ARCADE_PAD,
			joystick::type::THROTTLE,
		};
	}

	template<typename T>
	static constexpr const decltype(detail::s_vals_of_joystick_type)&
	values(std::enable_if_t <std::is_same_v <joystick::type, T>>* = nullptr) {
		return detail::s_vals_of_joystick_type;
	}

	template<typename T>
	static constexpr auto
	begin(std::enable_if_t <std::is_same_v <joystick::type, T>>* = nullptr) {
		return detail::s_vals_of_joystick_type.begin();
	}

	template<typename T>
	static constexpr auto
	end(std::enable_if_t <std::is_same_v <joystick::type, T>>* = nullptr) {
		return detail::s_vals_of_joystick_type.end();
	}

	d_SDLPP_OSTREAM(joystick::power_level);
	d_SDLPP_OSTREAM(joystick::type);

	inline
	joystick& joystick::operator=(object <SDL_Joystick>&& other) noexcept {
		object <SDL_Joystick>::operator=(std::move(other));
		return *this;;
	}

	inline
	joystick& joystick::operator=(joystick&& other) noexcept {
		object <SDL_Joystick>::operator=(std::move(other));
		return *this;
	}

	inline
	std::string joystick::get_name() const {
		return SAFE_SDL_CALL(SDL_JoystickName, const_handle ());
	}

	inline
	std::string joystick::get_path() const {
		return SAFE_SDL_CALL(SDL_JoystickPath, const_handle ());
	}

	inline
	joystick::power_level joystick::get_power_level() const noexcept {
		return static_cast <power_level>(SDL_JoystickCurrentPowerLevel(const_cast <SDL_Joystick*>(const_handle())));
	}

	inline
	std::optional <joystick_player_index_t> joystick::get_player_index() const noexcept {
		auto rc = SDL_JoystickGetPlayerIndex(const_handle());
		if (rc >= 0) {
			return joystick_player_index_t{rc};
		}
		return std::nullopt;
	}

	inline
	joystick_axis_t joystick::count_axes() const {
		auto rc = SAFE_SDL_CALL(SDL_JoystickNumAxes, const_handle ());
		return joystick_axis_t{static_cast <std::size_t>(rc)};
	}

	inline
	int16_t joystick::get_axis(joystick_axis_t axis_id) const {
		const auto rc = SDL_JoystickGetAxis(const_handle(), static_cast <int>(axis_id.value_of()));
		if (rc == 0) {
			RAISE_SDL_EX();
		}
		return rc;
	}

	inline
	std::optional <int16_t> joystick::get_axis_initial_state(joystick_axis_t axis_id) const {
		Sint16 state;
		if (SDL_TRUE
		    == SDL_JoystickGetAxisInitialState(const_handle(), static_cast <int>(axis_id.value_of()), &state)) {
			return state;
		}
		return std::nullopt;
	}

	inline
	joystick_ball_t joystick::count_balls() const {
		const auto rc = SAFE_SDL_CALL(SDL_JoystickNumBalls, const_handle ());
		return joystick_ball_t{static_cast <std::size_t>(rc)};
	}

	inline
	std::tuple <int, int> joystick::get_ball(joystick_ball_t ball_id) const {
		int dx;
		int dy;
		SAFE_SDL_CALL(SDL_JoystickGetBall, const_handle (), static_cast<int>(ball_id.value_of ()), &dx, &dy);
		return {dx, dy};
	}

	inline
	joystick_button_t joystick::count_buttons() const {
		const auto rc = SAFE_SDL_CALL(SDL_JoystickNumButtons, const_handle ());
		return joystick_button_t{static_cast <std::size_t>(rc)};
	}

	inline
	bool joystick::get_button(joystick_button_t button_id) const {
		return SDL_JoystickGetButton(const_handle(), static_cast <int>(button_id.value_of()));
	}

	inline
	std::optional <uint16_t> joystick::get_firmware_version() const {
		if (auto rc = SDL_JoystickGetFirmwareVersion(const_handle()); rc != 0) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	joystick_hat_t joystick::count_hats() const {
		const auto rc = SAFE_SDL_CALL(SDL_JoystickNumHats, const_handle ());
		return joystick_hat_t{static_cast <std::size_t>(rc)};
	}

	inline
	events::joystick_hat_state joystick::get_hat(joystick_hat_t hat_idx) const {
		return static_cast <events::joystick_hat_state>(SDL_JoystickGetHat(const_handle(), static_cast <int>(hat_idx
			                                                                   .value_of())));
	}

	inline
	std::optional <uint16_t> joystick::get_product() const {
		auto rc = SDL_JoystickGetProduct(const_handle());
		if (rc == 0) {
			return std::nullopt;
		}
		return rc;
	}

	inline
	std::optional <uint16_t> joystick::get_product_version() const {
		auto rc = SDL_JoystickGetProductVersion(const_handle());
		if (rc == 0) {
			return std::nullopt;
		}
		return rc;
	}

	inline
	std::optional <uint16_t> joystick::get_vendor() const {
		auto rc = SDL_JoystickGetVendor(const_handle());
		if (rc == 0) {
			return std::nullopt;
		}
		return rc;
	}

	inline
	std::string joystick::get_serial() const {
		if (const char* rc = SDL_JoystickGetSerial(const_handle())) {
			return rc;
		}
		return {};
	}

	inline
	bool joystick::has_led() const {
		return SDL_TRUE == SDL_JoystickHasLED(const_handle());
	}

	inline
	void joystick::set_led(uint8_t r, uint8_t g, uint8_t b) {
		SAFE_SDL_CALL(SDL_JoystickSetLED, handle (), r, g, b);
	}

	inline
	void joystick::set_led(const color& c) {
		set_led(c.r, c.g, c.b);
	}

	inline
	bool joystick::has_rumble() const {
		return SDL_TRUE == SDL_JoystickHasRumble(const_handle());
	}

	inline
	void joystick::rumble_triggers(uint16_t left, uint16_t right, std::chrono::milliseconds duration) {
		SAFE_SDL_CALL(SDL_JoystickRumbleTriggers, handle (), left, right, duration.count ());
	}

	inline
	void joystick::send_effect(const void* data, std::size_t size) {
		SAFE_SDL_CALL(SDL_JoystickSendEffect, handle (), data, static_cast<int>(size));
	}

	inline
	void joystick::set_player_index(joystick_player_index_t idx) {
		SDL_JoystickSetPlayerIndex(handle(), idx.value_of());
	}

	inline
	void joystick::clear_player_index() {
		SDL_JoystickSetPlayerIndex(handle(), -1);
	}

	inline
	object <SDL_Haptic> joystick::as_haptic() const {
		return {SAFE_SDL_CALL(SDL_HapticOpenFromJoystick, const_handle()), true};
	}

	inline
	bool joystick::is_haptic() const {
		return SDL_JoystickIsHaptic(const_handle()) == SDL_TRUE;
	}

	inline
	joystick_id_t joystick::get_id() const {
		return joystick_id_t(SDL_JoystickInstanceID(const_handle()));
	}

	inline object <SDL_GameController> joystick::as_game_cotroller() const {
		return {SAFE_SDL_CALL(SDL_GameControllerFromInstanceID, get_id().value_of()), true};
	}

	inline bool joystick::is_game_cotroller() const {
		object <SDL_GameController> o(SDL_GameControllerFromInstanceID(get_id().value_of()), true);
		return !o.is_null();
	}
}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_JOYSTIC_HH_
