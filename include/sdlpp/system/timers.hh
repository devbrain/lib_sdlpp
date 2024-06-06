//
// Created by igor on 01/06/2020.
//

#ifndef SDLPP_TIMERS_HH
#define SDLPP_TIMERS_HH

#include <chrono>
#include <cstdint>

#include "sdlpp/detail/sdl2.hh"
#include "sdlpp/detail/call.hh"
#include "bsw/exception.hh"

namespace neutrino::sdl {
  inline void delay (const std::chrono::milliseconds& ms) noexcept {
    SDL_Delay (static_cast<uint32_t>(ms.count ()));
  }

  inline void delay (const uint32_t& ms) noexcept {
    SDL_Delay (ms);
  }

  [[nodiscard]] inline uint64_t get_performance_counter () noexcept {
    return SDL_GetPerformanceCounter ();
  }

  [[nodiscard]] inline uint64_t get_performance_frequency () noexcept {
    return SDL_GetPerformanceFrequency ();
  }

  [[nodiscard]] inline std::chrono::milliseconds get_ms_since_init () noexcept {
    return std::chrono::milliseconds (SDL_GetTicks ());
  }

  [[nodiscard]] inline uint32_t get_ticks() noexcept {
    return SDL_GetTicks();
  }

  [[nodiscard]] inline uint64_t get_ticks_64() noexcept {
    return SDL_GetTicks64();
  }

  [[nodiscard]] inline SDL_TimerID
  add_timer (const std::chrono::milliseconds& interval, SDL_TimerCallback callback, void* param) {
    SDL_TimerID id = SDL_AddTimer (static_cast<uint32_t>(interval.count ()), callback, param);

    if (id == 0) {
      RAISE_SDL_EX ();
    }
    return id;
  }

  [[nodiscard]] inline bool remove_timer (SDL_TimerID timer) noexcept {
    return SDL_TRUE == SDL_RemoveTimer (timer);
  }
} // ns sdl

#endif //SDLPP_TIMERS_HH
