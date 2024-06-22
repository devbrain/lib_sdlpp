//
// Created by igor on 01/06/2020.
//

#ifndef SDLPP_TIMERS_HH
#define SDLPP_TIMERS_HH

#include <chrono>
#include <cstdint>


#include <sdlpp/detail/sdl2.hh>

namespace neutrino::sdl {

	/**
	 * @brief Delays the execution of the program for the specified duration.
	 *
	 * This function pauses the execution of the program for the specified duration
	 * in milliseconds. The actual blocking time can be longer due to operating system
	 * scheduling.
	 *
	 * @param ms The duration to delay in milliseconds.
	 *
	 * @note This function is provided inline for performance reasons.
	 *
	 * @see SDL_Delay
	 *
	 * @since This function is available since SDL 2.0.0.
	 */
	inline void delay (const std::chrono::milliseconds& ms) noexcept {
		SDL_Delay (static_cast<uint32_t>(ms.count ()));
	}

	/**
	 * @brief Waits for a specified number of milliseconds.
	 *
	 * This function pauses the program's execution for the specified number of milliseconds.
	 * It utilizes the SDL_Delay function from the SDL library to perform the delay.
	 * Note that the actual delay might be longer than the specified time due to operating system scheduling.
	 *
	 * @param ms The number of milliseconds to delay.
	 *
	 * @see SDL_Delay
	 *
	 * @since Available since SDL 2.0.0.
	 */
	inline void delay (const uint32_t& ms) noexcept {
		SDL_Delay (ms);
	}

	/**
	 * @brief Gets the current value of the high-resolution counter.
	 *
	 * The function is typically used for profiling. The counter values are only meaningful relative to each other. Differences between values can be converted to times by using SDL_GetPerformanceFrequency().
	 *
	 * @return The current counter value.
	 *
	 * @see SDL_GetPerformanceFrequency()
	 * @since SDL 2.0.0
	 */
	[[nodiscard]] inline uint64_t get_performance_counter () noexcept {
		return SDL_GetPerformanceCounter ();
	}

	/**
	   * \brief Returns the platform-specific count per second of the high resolution counter.
	   *
	   * This function returns the count per second of the high resolution counter. The high resolution counter is used to measure elapsed time with a high resolution.
	   *
	   * \return the platform-specific count per second of the high resolution counter.
	   *
	   * \sa SDL_GetPerformanceCounter
	   */
	[[nodiscard]] inline uint64_t get_performance_frequency () noexcept {
		return SDL_GetPerformanceFrequency ();
	}

	/**
	 * @brief Get the number of milliseconds since application initialization.
	 *
	 * This function returns the number of milliseconds that have passed since the SDL library was initialized.
	 * The return value is an unsigned 32-bit value representing the number of milliseconds.
	 *
	 * It is important to note that the value returned by this function will wrap around if the program
	 * runs continuously for approximately 49 days. Therefore, if the application requires long-term timing,
	 * it is recommended to use SDL_GetTicks64(), introduced in SDL 2.0.18, which does not wrap around.
	 *
	 * @returns The number of milliseconds since the SDL library was initialized as std::chrono::milliseconds.
	 *
	 * @see SDL_GetTicks()
	 */
	[[nodiscard]] inline std::chrono::milliseconds get_ms_since_init () noexcept {
		return std::chrono::milliseconds (SDL_GetTicks ());
	}

	/**
	 * @brief Get the number of milliseconds since SDL library initialization.
	 *
	 * This function returns the number of milliseconds that have elapsed since the SDL library was initialized. It provides a 32-bit unsigned integer value which wraps around if the program runs for more than approximately 49 days.
	 *
	 * @note Starting from SDL version 2.0.18, it is recommended to use SDL_GetTicks64() instead of this function, as it provides a 64-bit value that doesn't suffer from the wrapping issue.
	 *
	 * @returns The number of milliseconds since the SDL library initialization as an unsigned 32-bit integer.
	 *
	 * @see SDL_GetTicks64()
	 * @see SDL_TICKS_PASSED
	 *
	 * @since SDL 2.0.0
	 */
	[[nodiscard]] inline uint32_t get_ticks () noexcept {
		return SDL_GetTicks ();
	}

	/**
	 * @brief Get the number of milliseconds since SDL library initialization.
	 *
	 * This function returns the number of milliseconds that have elapsed since the SDL library was initialized. It is a wrapper around the SDL_GetTicks64 function provided by the SDL library.
	 *
	 * @note It is important to note that the SDL_GetTicks64 function returns a 64-bit value, allowing for accurate time calculations even over long periods. This is in contrast to the original SDL_GetTicks function, which suffered from a 32-bit overflow approximately every 49 days. Therefore, the SDL_GetTicks64 function is safe to compare directly in time calculations.
	 *
	 * @return An unsigned 64-bit value representing the number of milliseconds since the SDL library was initialized.
	 * @since This function is available since SDL 2.0.18.
	 */
	[[nodiscard]] inline uint64_t get_ticks_64 () noexcept {
		return SDL_GetTicks64 ();
	}

	/**
	 * @brief Adds a timer that calls a callback function at a future time.
	 *
	 * @param interval The timer delay in milliseconds
	 * @param callback The function to call when the specified interval elapses
	 * @param param A pointer that is passed to the callback function
	 * @return The ID of the timer that was created
	 * @throws sdl_error if an error occurs while adding the timer
	 *
	 * This function adds a timer that will call the specified callback function after the specified interval elapses.
	 * The callback function is passed the current timer interval and the user-supplied parameter from the SDL_AddTimer() call.
	 * The callback function should return the next timer interval. If the value returned from the callback is 0, the timer is canceled.
	 * The callback function is run on a separate thread.
	 * Timers take into account the amount of time it took to execute the callback.
	 * Timing may be inexact due to OS scheduling, so it's recommended to use SDL_GetTicks() or SDL_GetPerformanceCounter() to adjust for variances.
	 *
	 * Example Usage:
	 * @code{.cpp}
	 * add_timer(std::chrono::milliseconds(1000), timer_callback, nullptr);
	 *
	 * static Uint32 timer_callback(Uint32 interval, void* param) {
	 *     // Handle timer callback
	 *     return interval; // Return the next timer interval in milliseconds
	 * }
	 * @endcode
	 *
	 * @sa remove_timer
	 */
	[[nodiscard]] inline SDL_TimerID
	add_timer (const std::chrono::milliseconds& interval, SDL_TimerCallback callback, void* param) {
		SDL_TimerID id = SDL_AddTimer (static_cast<uint32_t>(interval.count ()), callback, param);

		if (id == 0) {
			RAISE_SDL_EX ();
		}
		return id;
	}

	/**
	 * @brief Removes a timer.
	 *
	 * This function removes a timer that was created with SDL_AddTimer().
	 *
	 * @param timer The ID of the timer to remove.
	 * @return `true` if the timer is successfully removed, `false` if the timer was not found.
	 *
	 * @note This function is available since SDL 2.0.0.
	 * @see add_timer
	 */
	[[nodiscard]] inline bool remove_timer (SDL_TimerID timer) noexcept {
		return SDL_TRUE == SDL_RemoveTimer (timer);
	}
} // ns sdl

#endif //SDLPP_TIMERS_HH
