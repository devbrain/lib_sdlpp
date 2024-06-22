//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_

#include <cstdint>
#include <array>
#include <optional>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <type_traits>
#include <variant>
#include <strong_type/strong_type.hpp>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/joystick_id.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <sdlpp/system/sensors.hh>
#include <sdlpp/system/touch.hh>
#include <sdlpp/video/color.hh>

namespace neutrino::sdl {
	using bind_button_t = strong::type <int, struct _bind_button_, strong::ordered, strong::ostreamable>;
	using bind_axis_t = strong::type <int, struct _bind_axis_, strong::ordered, strong::ostreamable>;

	struct bind_hat {
		int hat;
		int hat_mask;
	};

	using bind_type = std::variant <bind_button_t, bind_axis_t, bind_hat>;
	using game_controller_touchpad_t = strong::type <std::size_t,
	                                                 struct _game_controller_touchpad_,
	                                                 strong::bicrementable,
	                                                 strong::ordered,
	                                                 strong::ostreamable>;
	using game_controller_finger_t = strong::type <std::size_t,
	                                               struct _game_controller_finger_,
	                                               strong::bicrementable,
	                                               strong::ordered,
	                                               strong::ostreamable>;

	struct SDLPP_EXPORT game_controller_finger_data : public finger_data {
		uint8_t state;
	};

	class game_controller : public object <SDL_GameController> {
		public:
			enum class axis {
				LEFTX = SDL_CONTROLLER_AXIS_LEFTX,
				LEFTY = SDL_CONTROLLER_AXIS_LEFTY,
				RIGHTX = SDL_CONTROLLER_AXIS_RIGHTX,
				RIGHTY = SDL_CONTROLLER_AXIS_RIGHTY,
				TRIGGERLEFT = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
				TRIGGERRIGHT = SDL_CONTROLLER_AXIS_TRIGGERRIGHT
			};

			enum class button {
				A = SDL_CONTROLLER_BUTTON_A,
				B = SDL_CONTROLLER_BUTTON_B,
				X = SDL_CONTROLLER_BUTTON_X,
				Y = SDL_CONTROLLER_BUTTON_Y,
				BACK = SDL_CONTROLLER_BUTTON_BACK,
				GUIDE = SDL_CONTROLLER_BUTTON_GUIDE,
				START = SDL_CONTROLLER_BUTTON_START,
				LEFTSTICK = SDL_CONTROLLER_BUTTON_LEFTSTICK,
				RIGHTSTICK = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
				LEFTSHOULDER = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
				RIGHTSHOULDER = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
				DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP,
				DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
				DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
				DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
				MISC1 = SDL_CONTROLLER_BUTTON_MISC1,
				PADDLE1 = SDL_CONTROLLER_BUTTON_PADDLE1,
				PADDLE2 = SDL_CONTROLLER_BUTTON_PADDLE2,
				PADDLE3 = SDL_CONTROLLER_BUTTON_PADDLE3,
				PADDLE4 = SDL_CONTROLLER_BUTTON_PADDLE4,
				TOUCHPAD = SDL_CONTROLLER_BUTTON_TOUCHPAD
			};

			enum class type {
				UNKNOWN = SDL_CONTROLLER_TYPE_UNKNOWN,
				XBOX360 = SDL_CONTROLLER_TYPE_XBOX360,
				XBOXONE = SDL_CONTROLLER_TYPE_XBOXONE,
				PS3 = SDL_CONTROLLER_TYPE_PS3,
				PS4 = SDL_CONTROLLER_TYPE_PS4,
				NINTENDO_SWITCH_PRO = SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO,
				VIRTUAL = SDL_CONTROLLER_TYPE_VIRTUAL,
				PS5 = SDL_CONTROLLER_TYPE_PS5,
				AMAZON_LUNA = SDL_CONTROLLER_TYPE_AMAZON_LUNA,
				GOOGLE_STADIA = SDL_CONTROLLER_TYPE_GOOGLE_STADIA,
				NVIDIA_SHIELD = SDL_CONTROLLER_TYPE_NVIDIA_SHIELD,
				NINTENDO_SWITCH_JOYCON_LEFT = SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT,
				NINTENDO_SWITCH_JOYCON_RIGHT = SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT,
				NINTENDO_SWITCH_JOYCON_PAIR = SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR
			};

		public:
			using timed_data_t = sensor::timed_data_t;

		public:
			game_controller() = default;
			game_controller& operator=(object <SDL_GameController>&& other) noexcept;
			game_controller& operator=(game_controller&& other) noexcept;

			game_controller& operator=(const game_controller& other) = delete;
			game_controller(const game_controller& other) = delete;

			[[nodiscard]] std::optional <uint16_t> get_firmware() const;
			[[nodiscard]] std::optional <uint16_t> get_product() const;
			[[nodiscard]] std::optional <uint16_t> get_vendor() const;
			[[nodiscard]] std::optional <uint16_t> get_product_version() const;
			[[nodiscard]] std::optional <std::string> get_serial() const;
			[[nodiscard]] std::optional <uint64_t> get_steam_handle() const;
			[[nodiscard]] std::optional <std::string> get_name() const;
			[[nodiscard]] std::optional <std::string> get_path() const;

			[[nodiscard]] type get_type() const;

			[[nodiscard]] static bool add_mapping(const std::string& mapping);
			[[nodiscard]] std::optional <std::string> get_mapping() const;

			[[nodiscard]] bool is_connected() const;

			[[nodiscard]] bool has_axis(axis a) const;
			[[nodiscard]] std::vector <axis> get_supported_axes() const;

			[[nodiscard]] bool has_button(button b) const;
			[[nodiscard]] std::vector <button> get_supported_buttons() const;

			[[nodiscard]] bool has_led() const;
			[[nodiscard]] bool has_rumble() const;
			[[nodiscard]] bool has_rumble_triggers() const;

			[[nodiscard]] bool has_sensor(sensor::type st) const;
			[[nodiscard]] bool is_sensor_enabled(sensor::type st) const;
			void set_sensor_enabled(sensor::type st, bool enabled);
			[[nodiscard]] std::vector <sensor::type> get_supported_sensors() const;
			[[nodiscard]] std::vector <sensor::type> get_enabled_sensors() const;

			[[nodiscard]] int16_t get_axis(axis a) const;
			[[nodiscard]] bind_type get_bind(axis a) const;
			[[nodiscard]] bind_type get_bind(button b) const;
			[[nodiscard]] bool is_pressed(button b) const;

			[[nodiscard]] game_controller_touchpad_t touchpads_count() const;
			[[nodiscard]] game_controller_finger_t fingers_count(game_controller_touchpad_t touchpad) const;
			[[nodiscard]] std::optional <game_controller_finger_data>
			get_finger_data(game_controller_touchpad_t t, game_controller_finger_t f) const;

			[[nodiscard]] joystick_player_index_t get_player_index() const;
			void set_player_index(joystick_player_index_t idx);

			void rumble(uint16_t low_frequency_rumble, uint16_t high_frequency_rumble,
			            std::chrono::milliseconds duration);
			void rumble_triggers(uint16_t left, uint16_t right, std::chrono::milliseconds duration);
			void set_led(uint8_t r, uint8_t g, uint8_t b);
			void set_led(const color& c);

			[[nodiscard]] std::optional <float> get_data_rate(sensor::type st) const;

			void get_data(sensor::type st, uint64_t* timestamp, float* data, std::size_t num_values) const;
			void
			get_data(sensor::type st, std::chrono::microseconds* timestamp, float* data, std::size_t num_values) const;
			void get_data(sensor::type st, float* data, std::size_t num_values) const;

			template<class T>
			std::vector <T> get_data(sensor::type st, std::size_t num_values,
			                         typename std::enable_if <std::is_same_v <T,
			                                                                  float>>::type* = nullptr) const;

			template<class T>
			std::vector <T> get_data(sensor::type st, std::size_t num_values,
			                         typename std::enable_if <std::is_same_v <T,
			                                                                  timed_data_t>>::type* = nullptr) const;

			[[nodiscard]] object <SDL_Joystick> as_joystick() const;
	};

	namespace detail {
		static inline constexpr std::array <game_controller::axis, 6> s_vals_of_game_controller_axis = {
			game_controller::axis::LEFTX,
			game_controller::axis::LEFTY,
			game_controller::axis::RIGHTX,
			game_controller::axis::RIGHTY,
			game_controller::axis::TRIGGERLEFT,
			game_controller::axis::TRIGGERRIGHT,
		};
	}

	template<typename T>
	static inline constexpr const decltype(detail::s_vals_of_game_controller_axis)&
	values(typename std::enable_if <std::is_same_v <game_controller::axis, T>>::type* = nullptr) {
		return detail::s_vals_of_game_controller_axis;
	}

	template<typename T>
	static inline constexpr auto
	begin(typename std::enable_if <std::is_same_v <game_controller::axis, T>>::type* = nullptr) {
		return detail::s_vals_of_game_controller_axis.begin();
	}

	template<typename T>
	static inline constexpr auto
	end(typename std::enable_if <std::is_same_v <game_controller::axis, T>>::type* = nullptr) {
		return detail::s_vals_of_game_controller_axis.end();
	}

	namespace detail {
		static inline constexpr std::array <game_controller::button, 21> s_vals_of_game_controller_button = {
			game_controller::button::A,
			game_controller::button::B,
			game_controller::button::X,
			game_controller::button::Y,
			game_controller::button::BACK,
			game_controller::button::GUIDE,
			game_controller::button::START,
			game_controller::button::LEFTSTICK,
			game_controller::button::RIGHTSTICK,
			game_controller::button::LEFTSHOULDER,
			game_controller::button::RIGHTSHOULDER,
			game_controller::button::DPAD_UP,
			game_controller::button::DPAD_DOWN,
			game_controller::button::DPAD_LEFT,
			game_controller::button::DPAD_RIGHT,
			game_controller::button::MISC1,
			game_controller::button::PADDLE1,
			game_controller::button::PADDLE2,
			game_controller::button::PADDLE3,
			game_controller::button::PADDLE4,
			game_controller::button::TOUCHPAD,
		};
	}

	template<typename T>
	static inline constexpr const decltype(detail::s_vals_of_game_controller_button)&
	values(std::enable_if_t <std::is_same_v <game_controller::button, T>>* = nullptr) {
		return detail::s_vals_of_game_controller_button;
	}

	template<typename T>
	static constexpr auto
	begin(std::enable_if_t <std::is_same_v <game_controller::button, T>>* = nullptr) {
		return detail::s_vals_of_game_controller_button.begin();
	}

	template<typename T>
	static constexpr auto
	end(std::enable_if_t <std::is_same_v <game_controller::button, T>>* = nullptr) {
		return detail::s_vals_of_game_controller_button.end();
	}

	namespace detail {
		static inline constexpr std::array <game_controller::type, 14> s_vals_of_game_controller_type = {
			game_controller::type::UNKNOWN,
			game_controller::type::XBOX360,
			game_controller::type::XBOXONE,
			game_controller::type::PS3,
			game_controller::type::PS4,
			game_controller::type::NINTENDO_SWITCH_PRO,
			game_controller::type::VIRTUAL,
			game_controller::type::PS5,
			game_controller::type::AMAZON_LUNA,
			game_controller::type::GOOGLE_STADIA,
			game_controller::type::NVIDIA_SHIELD,
			game_controller::type::NINTENDO_SWITCH_JOYCON_LEFT,
			game_controller::type::NINTENDO_SWITCH_JOYCON_RIGHT,
			game_controller::type::NINTENDO_SWITCH_JOYCON_PAIR,
		};
	}

	template<typename T>
	static inline constexpr const decltype(detail::s_vals_of_game_controller_type)&
	values(typename std::enable_if <std::is_same_v <game_controller::type, T>>::type* = nullptr) {
		return detail::s_vals_of_game_controller_type;
	}

	template<typename T>
	static inline constexpr auto
	begin(typename std::enable_if <std::is_same_v <game_controller::type, T>>::type* = nullptr) {
		return detail::s_vals_of_game_controller_type.begin();
	}

	template<typename T>
	static inline constexpr auto
	end(typename std::enable_if <std::is_same_v <game_controller::type, T>>::type* = nullptr) {
		return detail::s_vals_of_game_controller_type.end();
	}

	inline
	game_controller& game_controller::operator=(object <SDL_GameController>&& other) noexcept {
		object <SDL_GameController>::operator=(std::move(other));
		return *this;;
	}

	inline
	game_controller& game_controller::operator=(game_controller&& other) noexcept {
		object <SDL_GameController>::operator=(std::move(other));
		return *this;
	}

	inline
	bool game_controller::add_mapping(const std::string& mapping) {
		auto rc = SDL_GameControllerAddMapping(mapping.c_str());
		if (rc == -1) {
			RAISE_SDL_EX("Failed to add mapping");
		}

		return rc == 1;
	}

	inline
	bool game_controller::is_connected() const {
		return SDL_GameControllerGetAttached(const_handle()) == SDL_TRUE;
	}

	inline
	int16_t game_controller::get_axis(game_controller::axis a) const {
		return SDL_GameControllerGetAxis(const_handle(), static_cast <SDL_GameControllerAxis>(a));
	}

	namespace detail {
		inline
		bind_type map_bind(const SDL_GameControllerButtonBind& x, const char* tag) {
			if (x.bindType == SDL_CONTROLLER_BINDTYPE_NONE) {
				RAISE_SDL_EX("Failed to get bind for ", tag);
			}
			switch (x.bindType) {
				case SDL_CONTROLLER_BINDTYPE_BUTTON: return bind_button_t{x.value.button};
				case SDL_CONTROLLER_BINDTYPE_AXIS: return bind_axis_t{x.value.axis};
				case SDL_CONTROLLER_BINDTYPE_HAT: return bind_hat{x.value.hat.hat, x.value.hat.hat_mask};
				default: ENFORCE(false);
			}
		}
	}

	inline
	bind_type game_controller::get_bind(game_controller::axis a) const {
		auto rc = SDL_GameControllerGetBindForAxis(const_handle(), static_cast <SDL_GameControllerAxis>(a));
		return detail::map_bind(rc, "axis");
	}

	inline
	bind_type game_controller::get_bind(game_controller::button b) const {
		auto rc = SDL_GameControllerGetBindForButton(const_handle(), static_cast <SDL_GameControllerButton>(b));
		return detail::map_bind(rc, "button");
	}

	inline
	bool game_controller::is_pressed(button b) const {
		return 1
		       == SAFE_SDL_CALL(SDL_GameControllerGetButton, const_handle (), static_cast<SDL_GameControllerButton>(b));
	}

	inline
	std::optional <uint16_t> game_controller::get_firmware() const {
		auto rc = SDL_GameControllerGetFirmwareVersion(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	game_controller_touchpad_t game_controller::touchpads_count() const {
		return game_controller_touchpad_t{
			static_cast <std::size_t>(SDL_GameControllerGetNumTouchpads(const_handle()))
		};
	}

	inline
	game_controller_finger_t game_controller::fingers_count(game_controller_touchpad_t touchpad) const {
		return game_controller_finger_t{
			static_cast <std::size_t>(SDL_GameControllerGetNumTouchpadFingers(const_handle(), static_cast <int>(touchpad
				                                                                  .value_of())))
		};
	}

	inline
	joystick_player_index_t game_controller::get_player_index() const {
		return joystick_player_index_t{SDL_GameControllerGetPlayerIndex(const_handle())};
	}

	inline
	std::optional <uint16_t> game_controller::get_product() const {
		auto rc = SDL_GameControllerGetProduct(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::optional <uint16_t> game_controller::get_vendor() const {
		auto rc = SDL_GameControllerGetVendor(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::optional <uint16_t> game_controller::get_product_version() const {
		auto rc = SDL_GameControllerGetProductVersion(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	void game_controller::get_data(sensor::type st, float* data, std::size_t num_values) const {
		SAFE_SDL_CALL(SDL_GameControllerGetSensorData, const_handle (), static_cast<SDL_SensorType>(st), data,
		              num_values);
	}

	inline
	void game_controller::get_data(sensor::type st, uint64_t* timestamp, float* data, std::size_t num_values) const {
		SAFE_SDL_CALL(SDL_GameControllerGetSensorDataWithTimestamp, const_handle (), static_cast<SDL_SensorType>(st),
		              timestamp, data, num_values);
	}

	void
	inline
	game_controller::get_data(sensor::type st, std::chrono::microseconds* timestamp, float* data,
	                          std::size_t num_values) const {
		std::vector <uint64_t> t(num_values);
		get_data(st, t.data(), data, num_values);
		std::transform(t.begin(), t.end(), timestamp, [](auto v) { return std::chrono::microseconds(v); });
	}

	inline
	std::optional <float> game_controller::get_data_rate(sensor::type st) const {
		auto rc = SDL_GameControllerGetSensorDataRate(const_handle(), static_cast <SDL_SensorType>(st));
		if (rc != 0.0f) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::optional <std::string> game_controller::get_serial() const {
		auto rc = SDL_GameControllerGetSerial(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::optional <uint64_t> game_controller::get_steam_handle() const {
		auto rc = SDL_GameControllerGetSteamHandle(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::optional <game_controller_finger_data>
	game_controller::get_finger_data(game_controller_touchpad_t t, game_controller_finger_t f) const {
		game_controller_finger_data out{};
		auto rc = SDL_GameControllerGetTouchpadFinger(const_handle(),
		                                              static_cast <int>(t.value_of()),
		                                              static_cast <int>(f.value_of()),
		                                              &out.state, &out.x, &out.y, &out.pressure);
		if (rc != -1) {
			return out;
		}
		return std::nullopt;
	}

	inline
	game_controller::type game_controller::get_type() const {
		return static_cast <type>(SDL_GameControllerGetType(const_handle()));
	}

	inline
	bool game_controller::has_axis(axis a) const {
		return SDL_TRUE == SDL_GameControllerHasAxis(const_handle(), static_cast <SDL_GameControllerAxis>(a));
	}

	inline
	std::vector <game_controller::axis> game_controller::get_supported_axes() const {
		std::vector <axis> out;
		std::copy_if(begin <axis>(), end <axis>(), std::back_inserter(out), [this](auto a) { return has_axis(a); });
		return out;
	}

	inline
	bool game_controller::has_button(button b) const {
		return SDL_TRUE == SDL_GameControllerHasButton(const_handle(), static_cast <SDL_GameControllerButton>(b));
	}

	inline
	std::vector <game_controller::button> game_controller::get_supported_buttons() const {
		std::vector <button> out;
		std::copy_if(begin <button>(), end <button>(), std::back_inserter(out),
		             [this](auto a) { return has_button(a); });
		return out;
	}

	inline
	bool game_controller::has_led() const {
		return SDL_TRUE == SDL_GameControllerHasLED(const_handle());
	}

	inline
	bool game_controller::has_rumble() const {
		return SDL_TRUE == SDL_GameControllerHasRumble(const_handle());
	}

	inline
	bool game_controller::has_rumble_triggers() const {
		return SDL_TRUE == SDL_GameControllerHasRumbleTriggers(const_handle());
	}

	inline
	bool game_controller::has_sensor(sensor::type st) const {
		return SDL_TRUE == SDL_GameControllerHasSensor(const_handle(), static_cast <SDL_SensorType>(st));
	}

	inline
	std::vector <sensor::type> game_controller::get_supported_sensors() const {
		std::vector <sensor::type> out;
		std::copy_if(begin <sensor::type>(), end <sensor::type>(), std::back_inserter(out),
		             [this](auto a) { return has_sensor(a); });
		return out;
	}

	inline
	bool game_controller::is_sensor_enabled(sensor::type st) const {
		return SDL_TRUE == SDL_GameControllerIsSensorEnabled(const_handle(), static_cast <SDL_SensorType>(st));
	}

	inline
	std::vector <sensor::type> game_controller::get_enabled_sensors() const {
		static std::array <sensor::type, 9> names = {
			sensor::type::UNKNOWN,
			sensor::type::ACCEL,
			sensor::type::GYRO,
			sensor::type::ACCEL_L,
			sensor::type::GYRO_L,
			sensor::type::ACCEL_R,
			sensor::type::GYRO_R
		};
		std::vector <sensor::type> out;
		for (const auto a : names) {
			if (is_sensor_enabled(a)) {
				out.emplace_back(a);
			}
		}
		return out;
	}

	inline
	std::optional <std::string> game_controller::get_mapping() const {
		auto rc = SDL_GameControllerMapping(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::optional <std::string> game_controller::get_name() const {
		auto rc = SDL_GameControllerName(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	std::optional <std::string> game_controller::get_path() const {
		auto rc = SDL_GameControllerPath(const_handle());
		if (rc) {
			return rc;
		}
		return std::nullopt;
	}

	inline
	void
	game_controller::rumble(uint16_t low_frequency_rumble, uint16_t high_frequency_rumble,
	                        std::chrono::milliseconds duration) {
		SAFE_SDL_CALL(SDL_GameControllerRumble, handle (), low_frequency_rumble, high_frequency_rumble,
		              static_cast<int>(duration
			              .count ()));
	}

	inline
	void game_controller::set_led(uint8_t r, uint8_t g, uint8_t b) {
		SAFE_SDL_CALL(SDL_GameControllerSetLED, handle (), r, g, b);
	}

	inline
	void game_controller::set_led(const color& c) {
		set_led(c.r, c.g, c.b);
	}

	inline
	void game_controller::set_player_index(joystick_player_index_t idx) {
		SDL_GameControllerSetPlayerIndex(handle(), static_cast <int>(idx.value_of()));
	}

	inline
	void game_controller::set_sensor_enabled(sensor::type st, bool enabled) {
		SAFE_SDL_CALL(SDL_GameControllerSetSensorEnabled, handle (), static_cast<SDL_SensorType>(st),
		              enabled ? SDL_TRUE : SDL_FALSE);
	}

	inline
	void game_controller::rumble_triggers(uint16_t left, uint16_t right, std::chrono::milliseconds duration) {
		SAFE_SDL_CALL(SDL_GameControllerRumbleTriggers, handle (), left, right, static_cast<int>(duration.count ()));
	}

	inline
	object <SDL_Joystick> game_controller::as_joystick() const {
		return {SDL_GameControllerGetJoystick(const_handle()), false};
	}

	template<class T>
	std::vector <T>
	game_controller::get_data(sensor::type st, std::size_t num_values, std::enable_if_t <std::is_same_v <T,
		                          float>>*) const {
		std::vector <T> t(num_values);
		get_data(st, (float*)t.data(), num_values);
		return t;
	}

	template<class T>
	std::vector <T>
	game_controller::get_data(sensor::type st, std::size_t num_values, std::enable_if_t <std::is_same_v <T,
		                          timed_data_t>>*) const {
		std::vector <uint64_t> times(num_values);
		std::vector <float> values(num_values);
		std::vector <T> out;
		std::transform(times.begin(), times.end(), values.begin(),
		               std::back_inserter(out),
		               [](auto t, auto v) {
			               return timed_data_t(std::chrono::microseconds(t), v);
		               });
		return out;
	}

	d_SDLPP_OSTREAM(game_controller::axis);
	d_SDLPP_OSTREAM(game_controller::type);
	d_SDLPP_OSTREAM(game_controller::button);
}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_GAME_CONTROLLER_HH_
