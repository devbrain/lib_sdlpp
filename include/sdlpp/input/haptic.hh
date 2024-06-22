//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_HAPTIC_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_HAPTIC_HH_

#include <chrono>
#include <array>
#include <string_view>
#include <type_traits>
#include <strong_type/strong_type.hpp>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/clamp.hh>
#include <sdlpp/detail/ostreamops.hh>

#include <bitflags/bitflags.hpp>

namespace neutrino::sdl {
	using haptic_axis_t = strong::type <std::size_t, struct _haptic_axis_, strong::bicrementable, strong::ordered,
	                                    strong::ostreamable>;
	using haptic_effect_t = strong::type <std::size_t, struct _haptic_effect_, strong::bicrementable, strong::ordered,
	                                      strong::ostreamable>;

	class haptic : public object <SDL_Haptic> {
		public:
			BEGIN_BITFLAGS(features)
				FLAG(CONSTANT_EFFECT)
				FLAG(SINE)
				FLAG(LEFT_RIGHT)
				FLAG(TRIANGLE)
				FLAG(SAWTOOTHUP)
				FLAG(SAWTOOTHDOWN)
				FLAG(RAMP)
				FLAG(SPRING)
				FLAG(DUMPER)
				FLAG(INERTIA)
				FLAG(FRICTION)
				FLAG(CUSTOM)
				FLAG(GAIN)
				FLAG(AUTOCENTER)
				FLAG(STATUS)
				FLAG(PAUSE)
			END_BITFLAGS(features)

		public:
			haptic() = default;
			haptic& operator=(object <SDL_Haptic>&& other) noexcept;
			haptic& operator=(haptic&& other) noexcept;

			haptic& operator=(const haptic& other) = delete;
			haptic(const haptic& other) = delete;

			[[nodiscard]] bool has_rumble() const;
			void rumble_init();
			void rumble_play(float strength, std::chrono::milliseconds duration);
			void rumble_stop();

			void stop();
			void stop(haptic_effect_t effect);
			[[nodiscard]] bool can_pause() const;
			void pause(bool enabled);
			[[nodiscard]] bool can_set_gain() const;
			void set_gain(unsigned value);
			[[nodiscard]] bool can_set_autocenter() const;
			void set_autocenter(unsigned value);

			[[nodiscard]] features get_features() const;

			[[nodiscard]] haptic_axis_t cout_axes() const;

			haptic_effect_t register_effect(SDL_HapticEffect& effect);
			void unregister_effect(haptic_effect_t effect);

			void play(haptic_effect_t effect, uint32_t iterations);
			void play_inf(haptic_effect_t effect);
	};

	namespace detail {
		static inline constexpr std::array <haptic::features::flag_type, 16> s_vals_of_haptic_features = {
			haptic::features::CONSTANT_EFFECT,
			haptic::features::SINE,
			haptic::features::LEFT_RIGHT,
			haptic::features::TRIANGLE,
			haptic::features::SAWTOOTHUP,
			haptic::features::SAWTOOTHDOWN,
			haptic::features::RAMP,
			haptic::features::SPRING,
			haptic::features::DUMPER,
			haptic::features::INERTIA,
			haptic::features::FRICTION,
			haptic::features::CUSTOM,
			haptic::features::GAIN,
			haptic::features::AUTOCENTER,
			haptic::features::STATUS,
			haptic::features::PAUSE,
		};
	}

	template<typename T>
	static constexpr const decltype(detail::s_vals_of_haptic_features)&
	values(std::enable_if_t <std::is_same_v <haptic::features, T>>* = nullptr) {
		return detail::s_vals_of_haptic_features;
	}

	template<typename T>
	static constexpr auto
	begin(std::enable_if_t <std::is_same_v <haptic::features, T>>* = nullptr) {
		return detail::s_vals_of_haptic_features.begin();
	}

	template<typename T>
	static constexpr auto
	end(std::enable_if_t <std::is_same_v <haptic::features, T>>* = nullptr) {
		return detail::s_vals_of_haptic_features.end();
	}

	d_SDLPP_OSTREAM(haptic::features);

	inline
	haptic& haptic::operator=(object <SDL_Haptic>&& other) noexcept {
		object <SDL_Haptic>::operator=(std::move(other));
		return *this;;
	}

	inline
	haptic& haptic::operator=(haptic&& other) noexcept {
		object <SDL_Haptic>::operator=(std::move(other));
		return *this;
	}

	inline
	bool haptic::has_rumble() const {
		return SDL_HapticRumbleSupported(const_handle()) == SDL_TRUE;
	}

	inline
	void haptic::rumble_init() {
		SAFE_SDL_CALL(SDL_HapticRumbleInit, handle());
	}

	inline
	void haptic::rumble_play(float strength, std::chrono::milliseconds duration) {
		SAFE_SDL_CALL(SDL_HapticRumblePlay, handle(), strength, static_cast<uint32_t>(duration.count()));
	}

	inline
	void haptic::rumble_stop() {
		SDL_HapticRumbleStop(handle());
	}

	inline
	void haptic::stop() {
		SDL_HapticStopAll(handle());
	}

	inline
	void haptic::pause(bool enabled) {
		if (enabled) {
			SDL_HapticPause(handle());
		} else {
			SDL_HapticUnpause(handle());
		}
	}

	inline
	void haptic::set_gain(unsigned int value) {
		SDL_HapticSetGain(handle(), clamp(static_cast <int>(value), 0, 100));
	}

	inline
	void haptic::set_autocenter(unsigned int value) {
		SDL_HapticSetAutocenter(handle(), clamp(static_cast <int>(value), 0, 100));
	}

	inline
	bool haptic::can_pause() const {
		return (SDL_HapticQuery(const_handle()) & SDL_HAPTIC_PAUSE) == SDL_HAPTIC_PAUSE;
	}

	inline
	bool haptic::can_set_gain() const {
		return (SDL_HapticQuery(const_handle()) & SDL_HAPTIC_GAIN) == SDL_HAPTIC_GAIN;
	}

	inline
	bool haptic::can_set_autocenter() const {
		return (SDL_HapticQuery(const_handle()) & SDL_HAPTIC_AUTOCENTER) == SDL_HAPTIC_AUTOCENTER;
	}

	inline
	haptic::features haptic::get_features() const {
		static std::array <uint16_t, 16> sdl_features{
			SDL_HAPTIC_CONSTANT,
			SDL_HAPTIC_SINE,
			SDL_HAPTIC_LEFTRIGHT,
			SDL_HAPTIC_TRIANGLE,
			SDL_HAPTIC_SAWTOOTHUP,
			SDL_HAPTIC_SAWTOOTHDOWN,
			SDL_HAPTIC_RAMP,
			SDL_HAPTIC_SPRING,
			SDL_HAPTIC_DAMPER,
			SDL_HAPTIC_INERTIA,
			SDL_HAPTIC_FRICTION,
			SDL_HAPTIC_CUSTOM,
			SDL_HAPTIC_GAIN,
			SDL_HAPTIC_AUTOCENTER,
			SDL_HAPTIC_STATUS,
			SDL_HAPTIC_PAUSE
		};
		static auto my_features = values <features>();

		auto f = SDL_HapticQuery(const_handle());
		static_assert(sdl_features.size() == my_features.size());
		features out{};
		for (int i = 0u; i < sdl_features.size(); i++) {
			if ((f & sdl_features[i]) == sdl_features[i]) {
				out.set(my_features[i]);
			}
		}
		return out;
	}

	inline
	haptic_axis_t haptic::cout_axes() const {
		auto rc = SDL_HapticNumAxes(const_handle());
		if (rc >= 0) {
			return haptic_axis_t{static_cast <std::size_t>(rc)};
		}
		return haptic_axis_t{0};
	}

	inline
	void haptic::play(haptic_effect_t effect, uint32_t iterations) {
		SAFE_SDL_CALL(SDL_HapticRunEffect, handle(), static_cast<int>(effect.value_of()), iterations);
	}

	inline
	void haptic::play_inf(haptic_effect_t effect) {
		SAFE_SDL_CALL(SDL_HapticRunEffect, handle(), static_cast<int>(effect.value_of()), SDL_HAPTIC_INFINITY);
	}

	inline
	haptic_effect_t haptic::register_effect(SDL_HapticEffect& effect) {
		return haptic_effect_t{static_cast <std::size_t>(SAFE_SDL_CALL(SDL_HapticNewEffect, handle(), &effect))};
	}

	inline
	void haptic::unregister_effect(haptic_effect_t effect) {
		SDL_HapticDestroyEffect(handle(), static_cast <int>(effect.value_of()));
	}

	inline
	void haptic::stop(haptic_effect_t effect) {
		SDL_HapticStopEffect(handle(), static_cast <int>(effect.value_of()));
	}
}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_HAPTIC_HH_
