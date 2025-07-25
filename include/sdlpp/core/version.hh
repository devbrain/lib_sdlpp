//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file version.hh
 * @brief Modern C++ wrapper for SDL3 version information
 * 
 * This header provides a type-safe interface to query and compare SDL versions,
 * both at compile-time and runtime.
 */

#include <sdlpp/core/sdl.hh>
#include <SDL3/SDL_version.h>
#include <string>
#include <sstream>
#include <ostream>
#include <tuple>

namespace sdlpp {
    /**
     * @brief Represents an SDL version with major, minor, and micro components
     *
     * This class provides a type-safe representation of SDL version numbers
     * with support for comparison operators and string conversion.
     */
    class version {
        private:
            int major_;
            int minor_;
            int micro_;

        public:
            /**
             * @brief Default constructor - initializes to 0.0.0
             */
            constexpr version() noexcept
                : major_(0), minor_(0), micro_(0) {
            }

            /**
             * @brief Construct from version components
             * @param major Major version number
             * @param minor Minor version number
             * @param micro Micro/patch version number
             */
            constexpr version(int major, int minor, int micro) noexcept
                : major_(major), minor_(minor), micro_(micro) {
            }

            /**
             * @brief Construct from numeric version (as returned by SDL_GetVersion)
             * @param version_num Numeric version in format MMMNNNCCC
             */
            constexpr explicit version(int version_num) noexcept
                : major_(SDL_VERSIONNUM_MAJOR(version_num))
                  , minor_(SDL_VERSIONNUM_MINOR(version_num))
                  , micro_(SDL_VERSIONNUM_MICRO(version_num)) {
            }

            // Accessors
            [[nodiscard]] constexpr int major() const noexcept { return major_; }
            [[nodiscard]] constexpr int minor() const noexcept { return minor_; }
            [[nodiscard]] constexpr int micro() const noexcept { return micro_; }
            [[nodiscard]] constexpr int patch() const noexcept { return micro_; } // Alias

            /**
             * @brief Convert to numeric representation
             * @return Version as a single integer (MMMNNNCCC format)
             */
            [[nodiscard]] constexpr int to_number() const noexcept {
                return SDL_VERSIONNUM(major_, minor_, micro_);
            }

            /**
             * @brief Convert to string representation
             * @return Version string in "major.minor.micro" format
             */
            [[nodiscard]] std::string to_string() const {
                return std::to_string(major_) + "." +
                       std::to_string(minor_) + "." +
                       std::to_string(micro_);
            }

            /**
             * @brief Check if this version is at least the specified version
             * @param major Required major version
             * @param minor Required minor version
             * @param micro Required micro version
             * @return true if this version >= specified version
             */
            [[nodiscard]] constexpr bool at_least(int major, int minor, int micro) const noexcept {
                return to_number() >= SDL_VERSIONNUM(major, minor, micro);
            }

            /**
             * @brief Check if this version is at least the specified version
             * @param other Version to compare against
             * @return true if this version >= other
             */
            [[nodiscard]] constexpr bool at_least(const version& other) const noexcept {
                return to_number() >= other.to_number();
            }

            // Comparison operators (C++20 spaceship operator)
            [[nodiscard]] constexpr auto operator<=>(const version& other) const noexcept {
                return to_number() <=> other.to_number();
            }

            [[nodiscard]] constexpr bool operator==(const version& other) const noexcept {
                return to_number() == other.to_number();
            }

            // Tuple conversion for structured bindings
            [[nodiscard]] constexpr std::tuple <int, int, int> to_tuple() const noexcept {
                return {major_, minor_, micro_};
            }

            // Support for structured bindings
            template<std::size_t N>
            [[nodiscard]] constexpr int get() const noexcept {
                if constexpr (N == 0) return major_;
                else if constexpr (N == 1) return minor_;
                else if constexpr (N == 2) return micro_;
            }

            // Stream output
            friend std::ostream& operator<<(std::ostream& os, const version& v) {
                return os << v.to_string();
            }
    };

    /**
     * @brief Version information and utilities
     *
     * This namespace provides compile-time and runtime version information
     * for SDL, along with utilities for version comparison.
     */
    namespace version_info {
        /**
         * @brief Compile-time SDL version (headers)
         *
         * This is the version of SDL headers your application was compiled against.
         */
        inline constexpr version compile_time{
            SDL_MAJOR_VERSION,
            SDL_MINOR_VERSION,
            SDL_MICRO_VERSION
        };

        /**
         * @brief Get runtime SDL version (linked library)
         *
         * This is the version of the SDL library currently linked to your application.
         * It may differ from compile_time if using dynamic linking.
         *
         * @return Current SDL library version
         */
        inline version runtime() noexcept {
            return version{SDL_GetVersion()};
        }

        /**
         * @brief Get SDL revision string
         *
         * Returns a string uniquely identifying the exact revision of SDL in use.
         * This is typically a git hash when SDL was built from source control.
         *
         * @return Revision string (may be empty if not available)
         */
        inline std::string revision() {
            const char* rev = SDL_GetRevision();
            return rev ? rev : "";
        }

        /**
         * @brief Check if runtime version matches compile-time version
         *
         * @return true if versions match exactly
         */
        inline bool versions_match() noexcept {
            return runtime() == compile_time;
        }

        /**
         * @brief Check if runtime version is at least compile-time version
         *
         * This is useful to ensure the linked library is not older than expected.
         *
         * @return true if runtime >= compile-time version
         */
        inline bool runtime_at_least_compiled() noexcept {
            return runtime() >= compile_time;
        }

        /**
         * @brief Version compatibility checker
         *
         * Provides utilities to check version compatibility and requirements.
         */
        class compatibility {
            public:
                /**
                 * @brief Check if SDL version meets minimum requirement at compile time
                 * @tparam Major Required major version
                 * @tparam Minor Required minor version
                 * @tparam Micro Required micro version
                 * @return true if SDL headers meet requirement
                 */
                template<int Major, int Minor, int Micro>
                static constexpr bool compile_time_at_least() noexcept {
                    return SDL_VERSION_ATLEAST(Major, Minor, Micro);
                }

                /**
                 * @brief Check if SDL version meets minimum requirement at runtime
                 * @param major Required major version
                 * @param minor Required minor version
                 * @param micro Required micro version
                 * @return true if linked SDL library meets requirement
                 */
                static bool runtime_at_least(int major, int minor, int micro) noexcept {
                    return runtime().at_least(major, minor, micro);
                }

                /**
                 * @brief Require minimum SDL version at compile time
                 *
                 * This will cause a compile error if SDL headers are too old.
                 *
                 * @tparam Major Required major version
                 * @tparam Minor Required minor version
                 * @tparam Micro Required micro version
                 */
                template<int Major, int Minor, int Micro>
                static constexpr void require_compile_time() {
                    static_assert(SDL_VERSION_ATLEAST(Major, Minor, Micro),
                                  "SDL headers version is too old. Please update SDL.");
                }

                /**
                 * @brief Get a human-readable compatibility report
                 * @return String describing version compatibility status
                 */
                static std::string report() {
                    std::ostringstream oss;
                    oss << "SDL Version Report:\n";
                    oss << "  Compiled against: " << compile_time << "\n";
                    oss << "  Runtime version:  " << runtime() << "\n";
                    oss << "  Revision:         " << revision() << "\n";
                    oss << "  Status:           ";

                    if (versions_match()) {
                        oss << "Exact match";
                    } else if (runtime_at_least_compiled()) {
                        oss << "Compatible (runtime newer)";
                    } else {
                        oss << "WARNING: Runtime older than compile-time";
                    }

                    return oss.str();
                }
        };

        /**
         * @brief Version feature detection
         *
         * Helpers to detect specific SDL features based on version.
         */
        namespace features {
            /**
             * @brief Check if properties API is available
             * Properties were added in SDL 3.2.0
             */
            inline constexpr bool has_properties = SDL_VERSION_ATLEAST(3, 2, 0);

            /**
             * @brief Check if GPU API is available
             * GPU API was added in SDL 3.2.0
             */
            inline constexpr bool has_gpu = SDL_VERSION_ATLEAST(3, 2, 0);

            /**
             * @brief Check if HID API is available
             * HID API can be disabled at compile time with SDL_HIDAPI_DISABLED
             */
#ifdef SDL_HIDAPI_DISABLED
            inline constexpr bool has_hidapi = false;
#else
            inline constexpr bool has_hidapi = SDL_VERSION_ATLEAST(3, 2, 0);
#endif

            /**
             * @brief Runtime feature check
             * @param major Version where feature was introduced
             * @param minor Version where feature was introduced
             * @param micro Version where feature was introduced
             * @return true if runtime SDL has this feature
             */
            inline bool available_at_runtime(int major, int minor, int micro) {
                return runtime().at_least(major, minor, micro);
            }
        }
    }

    // Convenience aliases
    using version_compat = version_info::compatibility;

    /**
     * @brief User-defined literal for version creation
     *
     * Allows syntax like: auto v = 3.2.1_v;
     *
     * Note: Only works for single-digit version components
     */
    namespace literals {
        constexpr version operator""_v(unsigned long long version_num) {
            int v = static_cast <int>(version_num);
            int micro = v % 10;
            v /= 10;
            int minor = v % 10;
            v /= 10;
            int major = v;
            return version{major, minor, micro};
        }
    }
} // namespace sdlpp

// Structured binding support
namespace std {
    template<>
    struct tuple_size <sdlpp::version> : std::integral_constant <std::size_t, 3> {
    };

    template<std::size_t N>
    struct tuple_element <N, sdlpp::version> {
        using type = int;
    };
}
