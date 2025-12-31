#pragma once

/**
 * @file expected.hh
 * @brief Provides a unified interface for expected types
 * 
 * This header provides a type alias for either std::expected (C++23) or 
 * tl::expected (fallback) depending on availability. All sdlpp code should
 * use sdlpp::expected instead of directly using tl::expected.
 */
#include <sdlpp/detail/compiler.hh>
#include <version>
#include <string>
#include <type_traits>
#include <failsafe/detail/string_utils.hh>

// Check for std::expected availability (C++23 feature)
#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L
    #include <expected>
    namespace sdlpp {
        /**
         * @brief Alias for std::expected when available (C++23)
         */
        template<typename T, typename E>
        using expected = std::expected<T, E>;
        
        /**
         * @brief Alias for std::unexpected
         */
        template<typename E>
        using unexpected = std::unexpected<E>;
        
        /**
         * @brief Create an unexpected value
         */
        template<typename E>
        constexpr auto make_unexpected(E&& e) {
            return std::unexpected(std::forward<E>(e));
        }
        
        inline constexpr bool using_std_expected = true;
    }
#else
// Fallback to tl::expected
#if defined(SDLPP_COMPILER_MSVC)
#pragma warning( push )

#elif defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wundef"
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic push
#elif defined(SDLPP_COMPILER_WASM)
# pragma clang diagnostic push
#endif
#include <tl/expected.hpp>
#if defined(SDLPP_COMPILER_MSVC)
#pragma warning( pop )
#elif defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic pop
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic pop
#elif defined(SDLPP_COMPILER_WASM)
# pragma clang diagnostic pop
#endif
namespace sdlpp {
    /**
     * @brief Alias for tl::expected when std::expected is not available
     */
    template<typename T, typename E>
    using expected = tl::expected <T, E>;

    /**
     * @brief Alias for tl::unexpected
     */
    template<typename E>
    using unexpected = tl::unexpected <E>;

    /**
     * @brief Create an unexpected value
     */
    template<typename E>
    constexpr auto make_unexpected(E&& e) {
        return tl::unexpected<std::decay_t<E>>(std::forward<E>(e));
    }

    inline constexpr bool using_std_expected = false;
}
#endif

namespace sdlpp {
    /**
     * @brief Type alias for error type used throughout sdlpp
     * 
     * Most sdlpp functions return expected<T, error_type> where
     * error_type is a string describing what went wrong.
     */
    using error_type = std::string;

    /**
     * @brief Common result type for operations that don't return a value
     */
    using result = expected <void, error_type>;

    /**
     * @brief Create an unexpected value with formatted message arguments
     */
    template<typename... Args>
    constexpr auto make_unexpectedf(Args&&... args) {
        return make_unexpected(
            failsafe::detail::build_message(std::forward<Args>(args)...));
    }

    /**
     * @brief Check which expected implementation is being used
     * 
     * This can be useful for conditional compilation or debugging.
     * 
     * @code
     * if constexpr (sdlpp::using_std_expected) {
     *     // Code specific to std::expected
     * } else {
     *     // Code specific to tl::expected
     * }
     * @endcode
     */
    inline constexpr const char* expected_implementation() noexcept {
        return using_std_expected ? "std::expected" : "tl::expected";
    }
}
