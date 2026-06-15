#pragma once

/**
 * @file random.hh
 * @brief Pseudo-random number helpers backed by SDL3
 *
 * SDL provides a small pseudo-random generator that is useful for games,
 * simulations, tests, and other non-cryptographic uses. This header exposes
 * both SDL's implicit generator and an explicit-state engine that can be used
 * with standard C++ distributions.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/utility/geometry.hh>
#include <failsafe/enforce.hh>
#include <concepts>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

namespace sdlpp::random {
    using seed_type = std::uint64_t;
    using result_type = std::uint32_t;

    namespace detail {
        template<typename T>
        concept integer_type =
            std::integral<T> &&
            !std::same_as<std::remove_cv_t<T>, bool> &&
            (sizeof(T) <= sizeof(unsigned long long));

        template<typename T>
        using distribution_integer_t =
            std::conditional_t<std::is_signed_v<T>, long long, unsigned long long>;

        template<typename T>
        concept random_value = integer_type<T> || std::floating_point<T>;

        class global_engine {
            public:
                using result_type = sdlpp::random::result_type;

                [[nodiscard]] static constexpr result_type min() noexcept {
                    return std::numeric_limits<result_type>::min();
                }

                [[nodiscard]] static constexpr result_type max() noexcept {
                    return std::numeric_limits<result_type>::max();
                }

                [[nodiscard]] result_type operator()() const noexcept {
                    return SDL_rand_bits();
                }
        };

        template<integer_type T, typename Engine>
        [[nodiscard]] T uniform_int_with(Engine& engine, T min_inclusive, T max_inclusive) {
            ENFORCE_LE(min_inclusive, max_inclusive)("Invalid random integer range");

            using dist_type = distribution_integer_t<T>;
            std::uniform_int_distribution<dist_type> distribution{
                static_cast<dist_type>(min_inclusive),
                static_cast<dist_type>(max_inclusive)
            };
            return static_cast<T>(distribution(engine));
        }

        template<std::floating_point T, typename Engine>
        [[nodiscard]] T uniform_real_with(Engine& engine, T min_inclusive, T max_exclusive) {
            ENFORCE_LE(min_inclusive, max_exclusive)("Invalid random real range");

            if (min_inclusive == max_exclusive) {
                return min_inclusive;
            }

            std::uniform_real_distribution<T> distribution{min_inclusive, max_exclusive};
            return distribution(engine);
        }

        template<random_value T, typename Engine>
        [[nodiscard]] T value_with(Engine& engine, T min_inclusive, T max_inclusive_or_exclusive) {
            if constexpr (integer_type<T>) {
                return uniform_int_with(engine, min_inclusive, max_inclusive_or_exclusive);
            } else {
                return uniform_real_with(engine, min_inclusive, max_inclusive_or_exclusive);
            }
        }

        template<random_value T>
        [[nodiscard]] constexpr T last_contained_coord(T start, T extent) noexcept {
            if constexpr (integer_type<T>) {
                return start + extent - T{1};
            } else {
                return start + extent;
            }
        }

        template<random_value T>
        [[nodiscard]] constexpr T max_contained_origin(T start, T extent, T object_extent) noexcept {
            return start + extent - object_extent;
        }
    } // namespace detail

    /**
     * @brief Seed SDL's implicit pseudo-random generator.
     *
     * Passing 0 asks SDL to seed from SDL_GetPerformanceCounter().
     */
    inline void seed(seed_type value = 0) noexcept {
        SDL_srand(value);
    }

    /**
     * @brief Generate 32 pseudo-random bits from SDL's implicit generator.
     */
    [[nodiscard]] inline result_type bits() noexcept {
        return SDL_rand_bits();
    }

    /**
     * @brief Generate an integer in [0, upper_exclusive).
     */
    [[nodiscard]] inline std::int32_t uniform_int(std::int32_t upper_exclusive) {
        ENFORCE_GT(upper_exclusive, 0)("Random upper bound must be positive");
        return SDL_rand(upper_exclusive);
    }

    /**
     * @brief Generate an integer in [min_inclusive, max_inclusive].
     */
    template<detail::integer_type T>
    [[nodiscard]] T uniform_int(T min_inclusive, T max_inclusive) {
        auto engine = detail::global_engine{};
        return detail::uniform_int_with(engine, min_inclusive, max_inclusive);
    }

    /**
     * @brief Generate an integer in [0, upper_exclusive).
     */
    template<detail::integer_type T>
    [[nodiscard]] T uniform_int(T upper_exclusive) {
        ENFORCE_GT(upper_exclusive, T{0})("Random upper bound must be positive");
        return uniform_int(T{0}, static_cast<T>(upper_exclusive - T{1}));
    }

    /**
     * @brief Generate a float in [0.0, 1.0).
     */
    [[nodiscard]] inline float uniform_float() noexcept {
        return SDL_randf();
    }

    /**
     * @brief Generate a floating-point value in [0.0, 1.0).
     */
    template<std::floating_point T = float>
    [[nodiscard]] T uniform_real() {
        if constexpr (std::same_as<T, float>) {
            return SDL_randf();
        } else {
            auto engine = detail::global_engine{};
            return detail::uniform_real_with(engine, T{0}, T{1});
        }
    }

    /**
     * @brief Generate a floating-point value in [min_inclusive, max_exclusive).
     */
    template<std::floating_point T>
    [[nodiscard]] T uniform_real(T min_inclusive, T max_exclusive) {
        auto engine = detail::global_engine{};
        return detail::uniform_real_with(engine, min_inclusive, max_exclusive);
    }

    /**
     * @brief Explicit-state SDL random engine.
     *
     * This is useful when a random stream should be local, reproducible, or
     * passed to standard C++ distributions. It satisfies the C++20
     * uniform_random_bit_generator requirements.
     */
    class engine {
        public:
            using result_type = sdlpp::random::result_type;

            constexpr engine() noexcept = default;

            explicit constexpr engine(seed_type seed_value) noexcept
                : state_(seed_value) {
            }

            [[nodiscard]] static engine from_time() noexcept {
                return engine{SDL_GetPerformanceCounter()};
            }

            [[nodiscard]] static constexpr result_type min() noexcept {
                return std::numeric_limits<result_type>::min();
            }

            [[nodiscard]] static constexpr result_type max() noexcept {
                return std::numeric_limits<result_type>::max();
            }

            void seed(seed_type seed_value) noexcept {
                state_ = seed_value;
            }

            [[nodiscard]] seed_type state() const noexcept {
                return state_;
            }

            [[nodiscard]] result_type operator()() noexcept {
                return SDL_rand_bits_r(&state_);
            }

            [[nodiscard]] std::int32_t uniform_int(std::int32_t upper_exclusive) {
                ENFORCE_GT(upper_exclusive, 0)("Random upper bound must be positive");
                return SDL_rand_r(&state_, upper_exclusive);
            }

            template<detail::integer_type T>
            [[nodiscard]] T uniform_int(T upper_exclusive) {
                ENFORCE_GT(upper_exclusive, T{0})("Random upper bound must be positive");
                return uniform_int(T{0}, static_cast<T>(upper_exclusive - T{1}));
            }

            template<detail::integer_type T>
            [[nodiscard]] T uniform_int(T min_inclusive, T max_inclusive) {
                return detail::uniform_int_with(*this, min_inclusive, max_inclusive);
            }

            [[nodiscard]] float uniform_float() noexcept {
                return SDL_randf_r(&state_);
            }

            template<std::floating_point T = float>
            [[nodiscard]] T uniform_real() {
                if constexpr (std::same_as<T, float>) {
                    return SDL_randf_r(&state_);
                } else {
                    return detail::uniform_real_with(*this, T{0}, T{1});
                }
            }

            template<std::floating_point T>
            [[nodiscard]] T uniform_real(T min_inclusive, T max_exclusive) {
                return detail::uniform_real_with(*this, min_inclusive, max_exclusive);
            }

        private:
            seed_type state_{1};
    };

#ifndef SDLPP_NO_BUILTIN_GEOMETRY
    /**
     * @brief Generate a value in [min, max] for integers, or [min, max) for floats.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] T value(Engine& engine, T min_inclusive, T max_inclusive_or_exclusive) {
        return detail::value_with(engine, min_inclusive, max_inclusive_or_exclusive);
    }

    /**
     * @brief Generate a value in [min, max] for integers, or [min, max) for floats.
     */
    template<detail::random_value T>
    [[nodiscard]] T value(T min_inclusive, T max_inclusive_or_exclusive) {
        auto engine = detail::global_engine{};
        return detail::value_with(engine, min_inclusive, max_inclusive_or_exclusive);
    }

    /**
     * @brief Generate a point from coordinate ranges.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::point<T> point(
        Engine& engine,
        T min_x,
        T max_x,
        T min_y,
        T max_y) {
        return {
            detail::value_with(engine, min_x, max_x),
            detail::value_with(engine, min_y, max_y)
        };
    }

    /**
     * @brief Generate a point from coordinate ranges.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::point<T> point(T min_x, T max_x, T min_y, T max_y) {
        auto engine = detail::global_engine{};
        return point(engine, min_x, max_x, min_y, max_y);
    }

    /**
     * @brief Generate a point contained by a rectangle.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::point<T> point(Engine& engine, const sdlpp::rect<T>& bounds) {
        ENFORCE_GT(bounds.w, T{0})("Random point bounds width must be positive");
        ENFORCE_GT(bounds.h, T{0})("Random point bounds height must be positive");

        return point(
            engine,
            bounds.x,
            detail::last_contained_coord(bounds.x, bounds.w),
            bounds.y,
            detail::last_contained_coord(bounds.y, bounds.h));
    }

    /**
     * @brief Generate a point contained by a rectangle.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::point<T> point(const sdlpp::rect<T>& bounds) {
        auto engine = detail::global_engine{};
        return point(engine, bounds);
    }

    /**
     * @brief Generate a size from width and height ranges.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::size<T> size(
        Engine& engine,
        T min_width,
        T max_width,
        T min_height,
        T max_height) {
        return {
            detail::value_with(engine, min_width, max_width),
            detail::value_with(engine, min_height, max_height)
        };
    }

    /**
     * @brief Generate a size from width and height ranges.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::size<T> size(T min_width, T max_width, T min_height, T max_height) {
        auto engine = detail::global_engine{};
        return size(engine, min_width, max_width, min_height, max_height);
    }

    /**
     * @brief Generate a size from min and max dimensions.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::size<T> size(
        Engine& engine,
        const sdlpp::size<T>& min_size,
        const sdlpp::size<T>& max_size) {
        return size(engine, min_size.width, max_size.width, min_size.height, max_size.height);
    }

    /**
     * @brief Generate a size from min and max dimensions.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::size<T> size(const sdlpp::size<T>& min_size, const sdlpp::size<T>& max_size) {
        auto engine = detail::global_engine{};
        return size(engine, min_size, max_size);
    }

    /**
     * @brief Generate a rectangle with a fixed size contained by bounds.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::rect<T> rect(
        Engine& engine,
        const sdlpp::rect<T>& bounds,
        const sdlpp::size<T>& dimensions) {
        ENFORCE_GE(bounds.w, dimensions.width)("Random rectangle width must fit inside bounds");
        ENFORCE_GE(bounds.h, dimensions.height)("Random rectangle height must fit inside bounds");

        const auto x = detail::value_with(
            engine,
            bounds.x,
            detail::max_contained_origin(bounds.x, bounds.w, dimensions.width));
        const auto y = detail::value_with(
            engine,
            bounds.y,
            detail::max_contained_origin(bounds.y, bounds.h, dimensions.height));

        return {x, y, dimensions.width, dimensions.height};
    }

    /**
     * @brief Generate a rectangle with a fixed size contained by bounds.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::rect<T> rect(const sdlpp::rect<T>& bounds, const sdlpp::size<T>& dimensions) {
        auto engine = detail::global_engine{};
        return rect(engine, bounds, dimensions);
    }

    /**
     * @brief Generate a rectangle with a random size contained by bounds.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::rect<T> rect(
        Engine& engine,
        const sdlpp::rect<T>& bounds,
        const sdlpp::size<T>& min_size,
        const sdlpp::size<T>& max_size) {
        ENFORCE_LE(min_size.width, max_size.width)("Invalid random rectangle width range");
        ENFORCE_LE(min_size.height, max_size.height)("Invalid random rectangle height range");
        ENFORCE_LE(max_size.width, bounds.w)("Random rectangle maximum width must fit inside bounds");
        ENFORCE_LE(max_size.height, bounds.h)("Random rectangle maximum height must fit inside bounds");

        return rect(engine, bounds, size(engine, min_size, max_size));
    }

    /**
     * @brief Generate a rectangle with a random size contained by bounds.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::rect<T> rect(
        const sdlpp::rect<T>& bounds,
        const sdlpp::size<T>& min_size,
        const sdlpp::size<T>& max_size) {
        auto engine = detail::global_engine{};
        return rect(engine, bounds, min_size, max_size);
    }

    /**
     * @brief Generate a line whose endpoints are contained by bounds.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::line<T> line(Engine& engine, const sdlpp::rect<T>& bounds) {
        return {point(engine, bounds), point(engine, bounds)};
    }

    /**
     * @brief Generate a line whose endpoints are contained by bounds.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::line<T> line(const sdlpp::rect<T>& bounds) {
        auto engine = detail::global_engine{};
        return line(engine, bounds);
    }

    /**
     * @brief Generate a circle contained by bounds.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::circle<T> circle(
        Engine& engine,
        const sdlpp::rect<T>& bounds,
        T min_radius,
        T max_radius) {
        ENFORCE_LE(min_radius, max_radius)("Invalid random circle radius range");
        ENFORCE_LE(max_radius * T{2}, bounds.w)("Random circle maximum diameter must fit inside bounds width");
        ENFORCE_LE(max_radius * T{2}, bounds.h)("Random circle maximum diameter must fit inside bounds height");

        const auto radius = detail::value_with(engine, min_radius, max_radius);
        const auto center = point(
            engine,
            bounds.x + radius,
            bounds.x + bounds.w - radius,
            bounds.y + radius,
            bounds.y + bounds.h - radius);

        return {center, radius};
    }

    /**
     * @brief Generate a circle contained by bounds.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::circle<T> circle(const sdlpp::rect<T>& bounds, T min_radius, T max_radius) {
        auto engine = detail::global_engine{};
        return circle(engine, bounds, min_radius, max_radius);
    }

    /**
     * @brief Generate a triangle whose vertices are contained by bounds.
     */
    template<detail::random_value T, std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::triangle<T> triangle(Engine& engine, const sdlpp::rect<T>& bounds) {
        return {
            point(engine, bounds),
            point(engine, bounds),
            point(engine, bounds)
        };
    }

    /**
     * @brief Generate a triangle whose vertices are contained by bounds.
     */
    template<detail::random_value T>
    [[nodiscard]] sdlpp::triangle<T> triangle(const sdlpp::rect<T>& bounds) {
        auto engine = detail::global_engine{};
        return triangle(engine, bounds);
    }

    [[nodiscard]] inline sdlpp::point_i point_i(const sdlpp::rect_i& bounds) {
        return point(bounds);
    }

    [[nodiscard]] inline sdlpp::point_f point_f(const sdlpp::rect_f& bounds) {
        return point(bounds);
    }

    [[nodiscard]] inline sdlpp::point_d point_d(const sdlpp::rect_d& bounds) {
        return point(bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::point_i point_i(Engine& engine, const sdlpp::rect_i& bounds) {
        return point(engine, bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::point_f point_f(Engine& engine, const sdlpp::rect_f& bounds) {
        return point(engine, bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::point_d point_d(Engine& engine, const sdlpp::rect_d& bounds) {
        return point(engine, bounds);
    }

    [[nodiscard]] inline sdlpp::size_i size_i(const sdlpp::size_i& min_size, const sdlpp::size_i& max_size) {
        return size(min_size, max_size);
    }

    [[nodiscard]] inline sdlpp::size_f size_f(const sdlpp::size_f& min_size, const sdlpp::size_f& max_size) {
        return size(min_size, max_size);
    }

    [[nodiscard]] inline sdlpp::size_d size_d(const sdlpp::size_d& min_size, const sdlpp::size_d& max_size) {
        return size(min_size, max_size);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::size_i size_i(
        Engine& engine,
        const sdlpp::size_i& min_size,
        const sdlpp::size_i& max_size) {
        return size(engine, min_size, max_size);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::size_f size_f(
        Engine& engine,
        const sdlpp::size_f& min_size,
        const sdlpp::size_f& max_size) {
        return size(engine, min_size, max_size);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::size_d size_d(
        Engine& engine,
        const sdlpp::size_d& min_size,
        const sdlpp::size_d& max_size) {
        return size(engine, min_size, max_size);
    }

    [[nodiscard]] inline sdlpp::rect_i rect_i(
        const sdlpp::rect_i& bounds,
        const sdlpp::size_i& min_size,
        const sdlpp::size_i& max_size) {
        return rect(bounds, min_size, max_size);
    }

    [[nodiscard]] inline sdlpp::rect_f rect_f(
        const sdlpp::rect_f& bounds,
        const sdlpp::size_f& min_size,
        const sdlpp::size_f& max_size) {
        return rect(bounds, min_size, max_size);
    }

    [[nodiscard]] inline sdlpp::rect_d rect_d(
        const sdlpp::rect_d& bounds,
        const sdlpp::size_d& min_size,
        const sdlpp::size_d& max_size) {
        return rect(bounds, min_size, max_size);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::rect_i rect_i(
        Engine& engine,
        const sdlpp::rect_i& bounds,
        const sdlpp::size_i& min_size,
        const sdlpp::size_i& max_size) {
        return rect(engine, bounds, min_size, max_size);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::rect_f rect_f(
        Engine& engine,
        const sdlpp::rect_f& bounds,
        const sdlpp::size_f& min_size,
        const sdlpp::size_f& max_size) {
        return rect(engine, bounds, min_size, max_size);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::rect_d rect_d(
        Engine& engine,
        const sdlpp::rect_d& bounds,
        const sdlpp::size_d& min_size,
        const sdlpp::size_d& max_size) {
        return rect(engine, bounds, min_size, max_size);
    }

    [[nodiscard]] inline sdlpp::line_i line_i(const sdlpp::rect_i& bounds) {
        return line(bounds);
    }

    [[nodiscard]] inline sdlpp::line_f line_f(const sdlpp::rect_f& bounds) {
        return line(bounds);
    }

    [[nodiscard]] inline sdlpp::line_d line_d(const sdlpp::rect_d& bounds) {
        return line(bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::line_i line_i(Engine& engine, const sdlpp::rect_i& bounds) {
        return line(engine, bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::line_f line_f(Engine& engine, const sdlpp::rect_f& bounds) {
        return line(engine, bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::line_d line_d(Engine& engine, const sdlpp::rect_d& bounds) {
        return line(engine, bounds);
    }

    [[nodiscard]] inline sdlpp::circle_i circle_i(const sdlpp::rect_i& bounds, int min_radius, int max_radius) {
        return circle(bounds, min_radius, max_radius);
    }

    [[nodiscard]] inline sdlpp::circle_f circle_f(const sdlpp::rect_f& bounds, float min_radius, float max_radius) {
        return circle(bounds, min_radius, max_radius);
    }

    [[nodiscard]] inline sdlpp::circle_d circle_d(const sdlpp::rect_d& bounds, double min_radius, double max_radius) {
        return circle(bounds, min_radius, max_radius);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::circle_i circle_i(
        Engine& engine,
        const sdlpp::rect_i& bounds,
        int min_radius,
        int max_radius) {
        return circle(engine, bounds, min_radius, max_radius);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::circle_f circle_f(
        Engine& engine,
        const sdlpp::rect_f& bounds,
        float min_radius,
        float max_radius) {
        return circle(engine, bounds, min_radius, max_radius);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::circle_d circle_d(
        Engine& engine,
        const sdlpp::rect_d& bounds,
        double min_radius,
        double max_radius) {
        return circle(engine, bounds, min_radius, max_radius);
    }

    [[nodiscard]] inline sdlpp::triangle_i triangle_i(const sdlpp::rect_i& bounds) {
        return triangle(bounds);
    }

    [[nodiscard]] inline sdlpp::triangle_f triangle_f(const sdlpp::rect_f& bounds) {
        return triangle(bounds);
    }

    [[nodiscard]] inline sdlpp::triangle_d triangle_d(const sdlpp::rect_d& bounds) {
        return triangle(bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::triangle_i triangle_i(Engine& engine, const sdlpp::rect_i& bounds) {
        return triangle(engine, bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::triangle_f triangle_f(Engine& engine, const sdlpp::rect_f& bounds) {
        return triangle(engine, bounds);
    }

    template<std::uniform_random_bit_generator Engine>
    [[nodiscard]] sdlpp::triangle_d triangle_d(Engine& engine, const sdlpp::rect_d& bounds) {
        return triangle(engine, bounds);
    }
#endif
} // namespace sdlpp::random
