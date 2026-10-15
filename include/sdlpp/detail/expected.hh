#pragma once

/**
 * @file expected.hh
 * @brief Provides a unified interface for expected types via failsafe::expected
 * 
 * All sdlpp code should use sdlpp::expected instead of directly using tl::expected
 * or std::expected.
 */
#include <failsafe/expected.hh>

namespace sdlpp {
    /**
     * @brief Alias for failsafe::expected
     */
    template<typename T, typename E = failsafe::error_type>
    using expected = failsafe::expected<T, E>;

    /**
     * @brief Alias for failsafe::unexpected
     */
    template<typename E>
    using unexpected = failsafe::unexpected<E>;

    using failsafe::unexpect_t;
    using failsafe::unexpect;
    using failsafe::bad_expected_access;

    using failsafe::make_unexpected;
    using failsafe::make_unexpectedf;

    using failsafe::using_std_expected;
    using failsafe::expected_implementation;

    /**
     * @brief Type alias for error type used throughout sdlpp
     */
    using error_type = failsafe::error_type;

    /**
     * @brief Common result type for operations that don't return a value
     */
    using result = failsafe::result;
}
