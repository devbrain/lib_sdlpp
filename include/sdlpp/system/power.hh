//
// Created by igor on 6/11/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_POWER_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_POWER_HH_
#include <array>
#include <type_traits>
#include <optional>
#include <chrono>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	enum class power_state {
		UNKNOWN = SDL_POWERSTATE_UNKNOWN,        ///< The status is unknown.
		ON_BATTERY = SDL_POWERSTATE_ON_BATTERY,  ///< Not plugged in and running on battery.
		NO_BATTERY = SDL_POWERSTATE_NO_BATTERY,  ///< No battery available.
		CHARGING = SDL_POWERSTATE_CHARGING,      ///< Charging the battery.
		CHARGED = SDL_POWERSTATE_CHARGED         ///< Plugged in and charged.
	};

	namespace detail {
		static inline constexpr std::array<power_state, 5> s_vals_of_power_state = {
			power_state::UNKNOWN,
			power_state::ON_BATTERY,
			power_state::NO_BATTERY,
			power_state::CHARGING,
			power_state::CHARGED,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_power_state)&
	values(typename std::enable_if<std::is_same_v<power_state, T>>::type* = nullptr) {
		return detail::s_vals_of_power_state;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<power_state, T>>::type* = nullptr) {
		return detail::s_vals_of_power_state.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<power_state, T>>::type* = nullptr) {
		return detail::s_vals_of_power_state.end();
	}


	d_SDLPP_OSTREAM(power_state);

	[[nodiscard]] inline
	power_state get_power_state() noexcept {
		return static_cast<power_state>(SDL_GetPowerInfo (nullptr, nullptr));
	}

	[[nodiscard]] inline
	bool is_battery_available() noexcept {
		const auto state = get_power_state();
		return state != power_state::NO_BATTERY && state != power_state::UNKNOWN;
	}

	[[nodiscard]] inline
	std::optional<int> get_battery_percentage() noexcept {
		int percentage = -1;
		SDL_GetPowerInfo(nullptr, &percentage);
		if (percentage != -1) {
			return percentage;
		}
		else {
			return std::nullopt;
		}
	}

	[[nodiscard]] inline
	std::optional<std::chrono::seconds> get_battery_seconds_left() noexcept {
		int secondsLeft = -1;
		SDL_GetPowerInfo(&secondsLeft, nullptr);
		if (secondsLeft != -1) {
			return std::chrono::seconds {secondsLeft};
		}
		else {
			return std::nullopt;
		}
	}

}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_POWER_HH_
