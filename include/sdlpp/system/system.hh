//
// Created by igor on 31/05/2020.
//

#ifndef NEUTRINO_SDL_SYSTEM_HH
#define NEUTRINO_SDL_SYSTEM_HH

#include <cstdint>
#include <array>
#include <type_traits>

#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/sdl2.hh>

namespace neutrino::sdl {
	enum class init_flags : uint32_t {
		TIMER = SDL_INIT_TIMER,
		AUDIO = SDL_INIT_AUDIO,
		VIDEO = SDL_INIT_VIDEO,
		JOYSTICK = SDL_INIT_JOYSTICK,
		HAPTIC = SDL_INIT_HAPTIC,
		GAMECONTROLLER = SDL_INIT_GAMECONTROLLER,
		EVENTS = SDL_INIT_EVENTS,
		SENSOR = SDL_INIT_SENSOR,
		NOPARACHUTE = SDL_INIT_NOPARACHUTE
	};

	class system {
	 public:
		template <typename ... Args,
		          std::enable_if_t<std::conjunction_v<std::is_same<Args, init_flags>...>, void*> = nullptr>
		explicit system (Args... flags);

		~system () noexcept;

		static std::size_t ram_in_mb() {
			return static_cast<std::size_t>(SDL_GetSystemRAM());
		}
	};

	namespace detail {
		static inline constexpr std::array<init_flags, 9> s_vals_of_init_flags = {
			init_flags::TIMER,
			init_flags::AUDIO,
			init_flags::VIDEO,
			init_flags::JOYSTICK,
			init_flags::HAPTIC,
			init_flags::GAMECONTROLLER,
			init_flags::EVENTS,
			init_flags::SENSOR,
			init_flags::NOPARACHUTE,
		};
	}
	template <typename T>
	static constexpr const decltype(detail::s_vals_of_init_flags)&
	values(std::enable_if_t<std::is_same_v<init_flags, T>>* = nullptr) {
		return detail::s_vals_of_init_flags;
	}
	template <typename T>
	static constexpr auto
	begin(std::enable_if_t<std::is_same_v<init_flags, T>>* = nullptr) {
		return detail::s_vals_of_init_flags.begin();
	}
	template <typename T>
	static constexpr auto
	end(std::enable_if_t<std::is_same_v<init_flags, T>>* = nullptr) {
		return detail::s_vals_of_init_flags.end();
	}

} // ns sdl

// ====================================================================
// Implementation
// ====================================================================

namespace neutrino::sdl {

	template <typename ... Args,
		typename std::enable_if_t<std::conjunction_v<std::is_same<Args, init_flags>...>, void*>>
	inline
	system::system (Args... flags) {
		uint32_t f = (static_cast<std::uint32_t>(flags) | ... | 0u);
		if (f == 0) {
			f = SDL_INIT_EVERYTHING;
		}
		SAFE_SDL_CALL(SDL_Init, f);
		TTF_Init();
		IMG_Init (IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP |  IMG_INIT_JXL | IMG_INIT_AVIF);
	}

	// -----------------------------------------------------------------------------------------------
	inline
	system::~system () noexcept {
		IMG_Quit();
		TTF_Quit();
		SDL_Quit ();
	}
}

#endif
