//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file timer.hh
 * @brief Modern C++ wrapper for SDL3 timer functionality
 * 
 * This header provides a type-safe, std::chrono-based interface to SDL's timer system,
 * including high-resolution timing, delays, and timer callbacks.
 */

#include <sdlpp/core/sdl.hh>
#include <SDL3/SDL_timer.h>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <thread>

namespace sdlpp {
    /**
     * @brief Timer callback function type
     * @param interval The timer interval
     * @return New interval in milliseconds (0 to cancel timer)
     */
    using timer_callback = std::function <std::chrono::milliseconds(std::chrono::milliseconds interval)>;

    /**
     * @brief High-precision timer callback function type
     * @param interval The timer interval
     * @return New interval in nanoseconds (0 to cancel timer)
     */
    using timer_callback_ns = std::function <std::chrono::nanoseconds(std::chrono::nanoseconds interval)>;

    /**
     * @brief Timer and timing utilities
     *
     * This class provides static methods for time measurement, delays, and timer management
     * using std::chrono types for type safety and clarity.
     */
    class timer {
        public:
            /**
             * @brief Get elapsed time since SDL initialization
             * @return Time elapsed as chrono duration
             */
            [[nodiscard]] static std::chrono::milliseconds elapsed() {
                return std::chrono::milliseconds(SDL_GetTicks());
            }

            /**
             * @brief Get elapsed time since SDL initialization with nanosecond precision
             * @return Time elapsed as chrono duration
             * @note Available since SDL 3.0.0
             */
            [[nodiscard]] static std::chrono::nanoseconds elapsed_ns() {
                return std::chrono::nanoseconds(SDL_GetTicksNS());
            }

            /**
             * @brief Get elapsed time since a specific time point
             * @param since Time point to measure from
             * @return Time elapsed since the given point
             */
            [[nodiscard]] static std::chrono::milliseconds elapsed_since(std::chrono::milliseconds since) {
                return elapsed() - since;
            }

            /**
             * @brief Get elapsed time since a specific time point with nanosecond precision
             * @param since Time point to measure from
             * @return Time elapsed since the given point
             */
            [[nodiscard]] static std::chrono::nanoseconds elapsed_since_ns(std::chrono::nanoseconds since) {
                return elapsed_ns() - since;
            }

            /**
             * @brief Clock type for SDL timing
             *
             * This clock uses SDL's internal timer as its time source.
             */
            struct clock {
                using duration = std::chrono::milliseconds;
                using rep = duration::rep;
                using period = duration::period;
                using time_point = std::chrono::time_point <clock>;

                static constexpr bool is_steady = true;

                [[nodiscard]] static time_point now() noexcept {
                    return time_point(elapsed());
                }
            };

            /**
             * @brief High-resolution performance counter
             *
             * Provides access to SDL's high-resolution timer for precise measurements.
             */
            class performance_counter {
                private:
                    Uint64 start_count;
                    static Uint64 frequency;

                public:
                    /**
                     * @brief Default constructor - captures current counter value
                     */
                    performance_counter()
                        : start_count(SDL_GetPerformanceCounter()) {
                    }

                    /**
                     * @brief Get elapsed time since construction
                     * @return Elapsed time as chrono duration
                     */
                    template<typename Duration = std::chrono::nanoseconds>
                    [[nodiscard]] Duration elapsed() const {
                        Uint64 current = SDL_GetPerformanceCounter();
                        Uint64 elapsed_counts = current - start_count;

                        // Convert to nanoseconds first for precision
                        auto nanos = (elapsed_counts * 1'000'000'000) / get_frequency();
                        return std::chrono::duration_cast <Duration>(std::chrono::nanoseconds(nanos));
                    }

                    /**
                     * @brief Reset the counter to current time
                     */
                    void reset() {
                        start_count = SDL_GetPerformanceCounter();
                    }

                    /**
                     * @brief Get the performance counter frequency
                     * @return Counts per second
                     */
                    [[nodiscard]] static Uint64 get_frequency() {
                        if (frequency == 0) {
                            frequency = SDL_GetPerformanceFrequency();
                        }
                        return frequency;
                    }

                    /**
                     * @brief Get current raw counter value
                     * @return Current performance counter value
                     */
                    [[nodiscard]] static Uint64 get_counter() {
                        return SDL_GetPerformanceCounter();
                    }
            };

            /**
             * @brief High-resolution clock using performance counter
             *
             * This clock provides nanosecond precision using SDL's performance counter.
             */
            struct high_resolution_clock {
                using duration = std::chrono::nanoseconds;
                using rep = duration::rep;
                using period = duration::period;
                using time_point = std::chrono::time_point <high_resolution_clock>;

                static constexpr bool is_steady = true;

                [[nodiscard]] static time_point now() noexcept {
                    static const Uint64 frequency = performance_counter::get_frequency();
                    Uint64 count = performance_counter::get_counter();
                    auto nanos = (count * 1'000'000'000) / frequency;
                    return time_point(duration(nanos));
                }
            };

            /**
             * @brief Delay execution for a specified duration
             * @tparam Rep Numeric type of the duration
             * @tparam Period Period type of the duration
             * @param duration Time to delay
             */
            template<typename Rep, typename Period>
            static void delay(std::chrono::duration <Rep, Period> duration) {
                auto ms = std::chrono::duration_cast <std::chrono::milliseconds>(duration);
                if (ms.count() > 0) {
                    SDL_Delay(static_cast <Uint32>(ms.count()));
                }
            }

            /**
             * @brief Delay execution with nanosecond precision
             * @tparam Rep Numeric type of the duration
             * @tparam Period Period type of the duration
             * @param duration Time to delay
             */
            template<typename Rep, typename Period>
            static void delay_precise(std::chrono::duration <Rep, Period> duration) {
                auto ns = std::chrono::duration_cast <std::chrono::nanoseconds>(duration);
                if (ns.count() > 0) {
#ifdef SDL_DelayPrecise
                SDL_DelayPrecise(static_cast<Uint64>(ns.count()));
#else
                    SDL_DelayNS(static_cast <Uint64>(ns.count()));
#endif
                }
            }

            /**
             * @brief Sleep the current thread for a specified duration
             *
             * This is an alias for delay() to match std::this_thread::sleep_for
             *
             * @tparam Rep Numeric type of the duration
             * @tparam Period Period type of the duration
             * @param duration Time to sleep
             */
            template<typename Rep, typename Period>
            static void sleep_for(std::chrono::duration <Rep, Period> duration) {
                delay(duration);
            }

            /**
             * @brief Sleep until a specific time point
             * @tparam Clock Clock type
             * @tparam Duration Duration type
             * @param time_point Time to sleep until
             */
            template<typename Clock, typename Duration>
            static void sleep_until(std::chrono::time_point <Clock, Duration> time_point) {
                auto now = Clock::now();
                if (time_point > now) {
                    sleep_for(time_point - now);
                }
            }
    };

    // Initialize static member
    inline Uint64 timer::performance_counter::frequency = 0;

    /**
     * @brief RAII wrapper for SDL timer callbacks
     *
     * This class provides automatic management of SDL timer callbacks,
     * ensuring proper cleanup when the timer goes out of scope.
     */
    class timer_handle {
        private:
            SDL_TimerID id;

            // Wrapper to handle the C-style callback
            struct callback_data {
                timer_callback func;
                std::atomic <bool> active{true};
                std::atomic <bool> timer_active{true}; // Track if timer is still scheduled

                static Uint32 sdl_callback(void* userdata, [[maybe_unused]] SDL_TimerID timer_id, Uint32 interval) {
                    auto* data = static_cast <callback_data*>(userdata);
                    if (!data || !data->active.load()) {
                        return 0; // Cancel timer
                    }

                    auto new_interval = data->func(std::chrono::milliseconds(interval));
                    if (new_interval.count() == 0) {
                        // Timer is being cancelled by callback
                        data->active.store(false);
                        data->timer_active.store(false);
                    }
                    return static_cast <Uint32>(new_interval.count());
                }
            };

            std::unique_ptr <callback_data> data;

        public:
            /**
             * @brief Default constructor - creates null timer
             */
            timer_handle() noexcept
                : id(0) {
            }

            /**
             * @brief Construct from timer ID and callback data
             * @param timer_id SDL timer ID
             * @param cb_data Callback data
             */
            timer_handle(SDL_TimerID timer_id, std::unique_ptr <callback_data> cb_data) noexcept
                : id(timer_id), data(std::move(cb_data)) {
            }

            /**
             * @brief Move constructor
             */
            timer_handle(timer_handle&& other) noexcept
                : id(other.id), data(std::move(other.data)) {
                other.id = 0;
            }

            /**
             * @brief Move assignment
             */
            timer_handle& operator=(timer_handle&& other) noexcept {
                if (this != &other) {
                    cancel();
                    id = other.id;
                    data = std::move(other.data);
                    other.id = 0;
                }
                return *this;
            }

            /**
             * @brief Destructor - cancels timer
             */
            ~timer_handle() {
                cancel();
            }

            // Delete copy operations
            timer_handle(const timer_handle&) = delete;
            timer_handle& operator=(const timer_handle&) = delete;

            /**
             * @brief Check if timer is active
             */
            [[nodiscard]] bool is_active() const noexcept {
                return id != 0 && data && data->timer_active.load();
            }

            /**
             * @brief Cancel the timer
             * @return true if timer was cancelled, false if it wasn't active
             */
            bool cancel() {
                if (id != 0) {
                    if (data) {
                        data->active.store(false);
                        data->timer_active.store(false);
                    }
                    [[maybe_unused]] bool removed = SDL_RemoveTimer(id);
                    id = 0;
                    data.reset();
                    return removed;
                }
                return false;
            }

            /**
             * @brief Create a timer with millisecond precision
             * @param interval Initial interval
             * @param callback Callback function
             * @return Expected containing timer handle or error message
             */
            [[nodiscard]] static expected <timer_handle, std::string> create(
                std::chrono::milliseconds interval,
                timer_callback callback) {
                auto data = std::make_unique <callback_data>();
                data->func = std::move(callback);

                SDL_TimerID id = SDL_AddTimer(
                    static_cast <Uint32>(interval.count()),
                    callback_data::sdl_callback,
                    data.get()
                );

                if (id == 0) {
                    return make_unexpected(get_error());
                }

                return timer_handle(id, std::move(data));
            }

            /**
             * @brief Create a one-shot timer
             * @param delay Delay before callback
             * @param callback Callback function (called once)
             * @return Expected containing timer handle or error message
             */
            template<typename Rep, typename Period>
            [[nodiscard]] static expected <timer_handle, std::string> create_oneshot(
                std::chrono::duration <Rep, Period> delay,
                std::function <void()> callback) {
                return create(
                    std::chrono::duration_cast <std::chrono::milliseconds>(delay),
                    [cb = std::move(callback)](std::chrono::milliseconds) {
                        cb();
                        return std::chrono::milliseconds(0); // Cancel after one execution
                    }
                );
            }

            /**
             * @brief Create a repeating timer
             * @param interval Interval between callbacks
             * @param callback Callback function (called repeatedly)
             * @return Expected containing timer handle or error message
             */
            template<typename Rep, typename Period>
            [[nodiscard]] static expected <timer_handle, std::string> create_repeating(
                std::chrono::duration <Rep, Period> interval,
                std::function <void()> callback) {
                auto interval_ms = std::chrono::duration_cast <std::chrono::milliseconds>(interval);
                return create(
                    interval_ms,
                    [cb = std::move(callback), interval_ms](std::chrono::milliseconds) {
                        cb();
                        return interval_ms; // Keep the same interval
                    }
                );
            }
    };

    /**
     * @brief High-precision timer handle for nanosecond timers
     */
    class timer_handle_ns {
        private:
            SDL_TimerID id;

            struct callback_data {
                timer_callback_ns func;
                std::atomic <bool> active{true};
                std::atomic <bool> timer_active{true};

                static Uint64 sdl_callback(void* userdata, SDL_TimerID, Uint64 interval) {
                    auto* data = static_cast <callback_data*>(userdata);
                    if (!data || !data->active.load()) {
                        return 0;
                    }

                    auto new_interval = data->func(std::chrono::nanoseconds(interval));
                    if (new_interval.count() == 0) {
                        data->active.store(false);
                        data->timer_active.store(false);
                    }
                    return static_cast <Uint64>(new_interval.count());
                }
            };

            std::unique_ptr <callback_data> data;

        public:
            timer_handle_ns() noexcept
                : id(0) {
            }

            timer_handle_ns(SDL_TimerID timer_id, std::unique_ptr <callback_data> cb_data) noexcept
                : id(timer_id), data(std::move(cb_data)) {
            }

            timer_handle_ns(timer_handle_ns&& other) noexcept
                : id(other.id), data(std::move(other.data)) {
                other.id = 0;
            }

            timer_handle_ns& operator=(timer_handle_ns&& other) noexcept {
                if (this != &other) {
                    cancel();
                    id = other.id;
                    data = std::move(other.data);
                    other.id = 0;
                }
                return *this;
            }

            ~timer_handle_ns() {
                cancel();
            }

            timer_handle_ns(const timer_handle_ns&) = delete;
            timer_handle_ns& operator=(const timer_handle_ns&) = delete;

            [[nodiscard]] bool is_active() const noexcept {
                return id != 0 && data && data->timer_active.load();
            }

            bool cancel() {
                if (id != 0) {
                    if (data) {
                        data->active.store(false);
                        data->timer_active.store(false);
                    }
                    [[maybe_unused]] bool removed = SDL_RemoveTimer(id);
                    id = 0;
                    data.reset();
                    return removed;
                }
                return false;
            }

            /**
             * @brief Create a high-precision timer
             * @param interval Initial interval
             * @param callback Callback function
             * @return Expected containing timer handle or error message
             */
            [[nodiscard]] static expected <timer_handle_ns, std::string> create(
                std::chrono::nanoseconds interval,
                timer_callback_ns callback) {
                auto data = std::make_unique <callback_data>();
                data->func = std::move(callback);

                SDL_TimerID id = SDL_AddTimerNS(
                    static_cast <Uint64>(interval.count()),
                    callback_data::sdl_callback,
                    data.get()
                );

                if (id == 0) {
                    return make_unexpected(get_error());
                }

                return timer_handle_ns(id, std::move(data));
            }
    };

    /**
     * @brief Scoped timer for measuring execution time
     *
     * Measures time from construction to destruction, useful for profiling.
     *
     * Example:
     * @code
     * {
     *     scoped_timer timer("Operation");
     *     // ... do work ...
     * } // Timer prints elapsed time
     * @endcode
     */
    class scoped_timer {
        private:
            std::string name;
            timer::performance_counter counter;
            std::function <void(const std::string&, std::chrono::nanoseconds)> callback;

        public:
            /**
             * @brief Construct a scoped timer
             * @param timer_name Name for identification
             * @param on_complete Optional callback when timer completes
             */
            explicit scoped_timer(
                std::string timer_name,
                std::function <void(const std::string&, std::chrono::nanoseconds)> on_complete = nullptr)
                : name(std::move(timer_name)), callback(std::move(on_complete)) {
            }

            /**
             * @brief Destructor - reports elapsed time
             */
            ~scoped_timer() {
                auto elapsed = counter.elapsed();
                if (callback) {
                    callback(name, elapsed);
                } else {
                    // Default: print to stdout
                    auto ms = static_cast <double>(std::chrono::duration_cast <std::chrono::microseconds>(elapsed).
                                  count()) / 1000.0;
                    printf("[Timer] %s: %.3f ms\n", name.c_str(), ms);
                }
            }

            /**
             * @brief Get elapsed time so far
             * @return Elapsed time
             */
            template<typename Duration = std::chrono::nanoseconds>
            [[nodiscard]] Duration elapsed() const {
                return counter.elapsed <Duration>();
            }
    };

    /**
     * @brief Frame rate limiter utility
     *
     * Helps maintain a consistent frame rate by calculating delays.
     */
    class frame_limiter {
        private:
            std::chrono::nanoseconds target_frame_time;
            timer::high_resolution_clock::time_point last_frame_time;

        public:
            /**
             * @brief Construct frame limiter for target FPS
             * @param target_fps Desired frames per second
             */
            explicit frame_limiter(double target_fps)
                : target_frame_time(static_cast <int64_t>(1'000'000'000.0 / target_fps)),
                  last_frame_time(timer::high_resolution_clock::now()) {
            }

            /**
             * @brief Construct frame limiter with target frame duration
             * @tparam Rep Duration representation type
             * @tparam Period Duration period type
             * @param frame_duration Target duration per frame
             */
            template<typename Rep, typename Period>
            explicit frame_limiter(std::chrono::duration <Rep, Period> frame_duration)
                : target_frame_time(std::chrono::duration_cast <std::chrono::nanoseconds>(frame_duration)),
                  last_frame_time(timer::high_resolution_clock::now()) {
            }

            /**
             * @brief Wait until next frame should begin
             *
             * Call this at the end of your frame loop to maintain consistent timing.
             */
            void wait_for_next_frame() {
                auto now = timer::high_resolution_clock::now();
                auto frame_time = now - last_frame_time;

                if (frame_time < target_frame_time) {
                    timer::delay_precise(target_frame_time - frame_time);
                    last_frame_time = timer::high_resolution_clock::now();
                } else {
                    last_frame_time = now;
                }
            }

            /**
             * @brief Get actual frame time of last frame
             * @return Duration of the last frame
             */
            [[nodiscard]] std::chrono::nanoseconds get_frame_time() const {
                auto now = timer::high_resolution_clock::now();
                return now - last_frame_time;
            }

            /**
             * @brief Get current FPS based on last frame
             * @return Current frames per second
             */
            [[nodiscard]] double get_fps() const {
                auto frame_time = get_frame_time();
                if (frame_time.count() > 0) {
                    return 1'000'000'000.0 / static_cast <double>(frame_time.count());
                }
                return 0.0;
            }

            /**
             * @brief Reset frame timing
             */
            void reset() {
                last_frame_time = timer::high_resolution_clock::now();
            }
    };
} // namespace sdlpp
