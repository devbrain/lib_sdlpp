/**
 * @file core.hh
 * @brief Core SDL initialization and management
 */
#pragma once

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>

#include <cstdint>
#include <utility>

namespace sdlpp {
    /**
     * @brief SDL initialization flags
     */
    enum class init_flags : std::uint32_t {
        none = 0,
        audio = SDL_INIT_AUDIO,
        video = SDL_INIT_VIDEO,
        joystick = SDL_INIT_JOYSTICK,
        haptic = SDL_INIT_HAPTIC,
        gamepad = SDL_INIT_GAMEPAD,
        events = SDL_INIT_EVENTS,
        sensor = SDL_INIT_SENSOR,
        camera = SDL_INIT_CAMERA,
    };

    [[nodiscard]] constexpr init_flags operator|(init_flags lhs, init_flags rhs) noexcept {
        return static_cast <init_flags>(
            static_cast <std::uint32_t>(lhs) | static_cast <std::uint32_t>(rhs)
        );
    }

    [[nodiscard]] constexpr init_flags operator&(init_flags lhs, init_flags rhs) noexcept {
        return static_cast <init_flags>(
            static_cast <std::uint32_t>(lhs) & static_cast <std::uint32_t>(rhs)
        );
    }

    [[nodiscard]] constexpr bool has_flag(init_flags flags, init_flags flag) noexcept {
        return (flags & flag) == flag;
    }

    /**
     * @brief RAII wrapper for SDL initialization
     *
     * This class provides automatic initialization and cleanup of SDL subsystems.
     * The destructor automatically calls SDL_Quit when the object goes out of scope.
     */
    class init {
        public:
            // Constructors
            init() = delete;
            init(const init&) = delete;
            init& operator=(const init&) = delete;

            init(init&& other) noexcept
                : initialized_(std::exchange(other.initialized_, false)) {
            }

            init& operator=(init&& other) noexcept {
                if (this != &other) {
                    if (initialized_) {
                        SDL_Quit();
                    }
                    initialized_ = std::exchange(other.initialized_, false);
                }
                return *this;
            }

            /**
             * @brief Initialize SDL with specified subsystems
             * @param flags Subsystems to initialize
             */
            explicit init(init_flags flags)
                : initialized_(false) {
                if (!SDL_Init(static_cast <SDL_InitFlags>(flags))) {
                    throw std::runtime_error("Failed to initialize SDL: " + get_error());
                }
                initialized_ = true;
            }

            ~init() {
                if (initialized_) {
                    SDL_Quit();
                }
            }

            /**
             * @brief Check if SDL was successfully initialized
             */
            [[nodiscard]] explicit operator bool() const noexcept {
                return initialized_;
            }

            /**
             * @brief Check if SDL was successfully initialized
             */
            [[nodiscard]] bool is_initialized() const noexcept {
                return initialized_;
            }

            /**
             * @brief Initialize additional subsystems
             * @param flags Subsystems to initialize
             * @return true on success, false on failure
             */
            [[nodiscard]] bool init_subsystem(init_flags flags) noexcept {
                return initialized_ && SDL_InitSubSystem(static_cast <SDL_InitFlags>(flags));
            }

            /**
             * @brief Quit specific subsystems
             * @param flags Subsystems to quit
             */
            void quit_subsystem(init_flags flags) noexcept {
                if (initialized_) {
                    SDL_QuitSubSystem(static_cast <SDL_InitFlags>(flags));
                }
            }

            /**
             * @brief Check if subsystems are initialized
             * @param flags Subsystems to check (defaults to none - checks if any subsystem is initialized)
             * @return true if all specified subsystems are initialized
             */
            [[nodiscard]] bool was_init(init_flags flags = init_flags::none) const noexcept {
                auto init_flags = SDL_WasInit(static_cast <SDL_InitFlags>(flags));
                return init_flags == static_cast <SDL_InitFlags>(flags);
            }

        private:
            bool initialized_;
    };
} // namespace sdlpp
