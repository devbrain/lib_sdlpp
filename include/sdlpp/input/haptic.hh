#pragma once

/**
 * @file haptic.hh
 * @brief Haptic (force feedback) functionality
 * 
 * This header provides C++ wrappers around SDL3's haptic API, offering
 * force feedback support for game controllers and other haptic devices.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/input/joystick.hh>

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <optional>
#include <variant>

namespace sdlpp {
    /**
     * @brief Haptic device ID type
     */
    using haptic_id = SDL_HapticID;

    /**
     * @brief Infinity constant for haptic effects
     * Use this to play an effect for an infinite number of times
     */
    inline constexpr uint32_t haptic_infinity = SDL_HAPTIC_INFINITY;

    /**
     * @brief Haptic effect types and features
     */
    enum class haptic_feature : uint32_t {
        // Effect types
        constant = SDL_HAPTIC_CONSTANT, // Constant force effect
        sine = SDL_HAPTIC_SINE, // Sine wave effect
        square = SDL_HAPTIC_SQUARE, // Square wave effect
        triangle = SDL_HAPTIC_TRIANGLE, // Triangle wave effect
        sawtoothup = SDL_HAPTIC_SAWTOOTHUP, // Sawtooth up wave effect
        sawtoothdown = SDL_HAPTIC_SAWTOOTHDOWN, // Sawtooth down wave effect
        ramp = SDL_HAPTIC_RAMP, // Ramp effect
        spring = SDL_HAPTIC_SPRING, // Spring effect (condition)
        damper = SDL_HAPTIC_DAMPER, // Damper effect (condition)
        inertia = SDL_HAPTIC_INERTIA, // Inertia effect (condition)
        friction = SDL_HAPTIC_FRICTION, // Friction effect (condition)
        leftright = SDL_HAPTIC_LEFTRIGHT, // Left/Right motor effect
        custom = SDL_HAPTIC_CUSTOM, // Custom effect

        // Device capabilities
        gain = SDL_HAPTIC_GAIN, // Device can set global gain
        autocenter = SDL_HAPTIC_AUTOCENTER, // Device can set autocenter
        status = SDL_HAPTIC_STATUS, // Device can query effect status
        pause = SDL_HAPTIC_PAUSE // Device can be paused
    };

    /**
     * @brief Enable bitwise operations for haptic_feature
     */
    [[nodiscard]] inline constexpr haptic_feature operator|(haptic_feature a, haptic_feature b) noexcept {
        return static_cast <haptic_feature>(static_cast <uint32_t>(a) | static_cast <uint32_t>(b));
    }

    [[nodiscard]] inline constexpr haptic_feature operator&(haptic_feature a, haptic_feature b) noexcept {
        return static_cast <haptic_feature>(static_cast <uint32_t>(a) & static_cast <uint32_t>(b));
    }

    [[nodiscard]] inline constexpr bool has_flag(haptic_feature flags, haptic_feature flag) noexcept {
        return (flags & flag) == flag;
    }

    /**
     * @brief Direction coordinate systems
     */
    enum class haptic_direction_type : uint8_t {
        polar = SDL_HAPTIC_POLAR, // Polar coordinates
        cartesian = SDL_HAPTIC_CARTESIAN, // Cartesian coordinates
        spherical = SDL_HAPTIC_SPHERICAL, // Spherical coordinates
        steering_axis = SDL_HAPTIC_STEERING_AXIS // Steering wheel axis
    };

    /**
     * @brief Haptic direction
     */
    struct haptic_direction {
        haptic_direction_type type = haptic_direction_type::polar;
        std::array <int32_t, 3> dir = {0, 0, 0};

        /**
         * @brief Create a polar direction
         * @param degrees Direction in hundredths of a degree (0-36000)
         */
        static haptic_direction polar(int32_t degrees) noexcept {
            return {haptic_direction_type::polar, {degrees, 0, 0}};
        }

        /**
         * @brief Create a cartesian direction
         * @param x X component
         * @param y Y component
         * @param z Z component (optional, default 0)
         */
        static haptic_direction cartesian(int32_t x, int32_t y, int32_t z = 0) noexcept {
            return {haptic_direction_type::cartesian, {x, y, z}};
        }

        /**
         * @brief Create a spherical direction
         * @param azimuth Degrees from (1,0) rotated towards (0,1)
         * @param elevation Degrees towards (0,0,1)
         */
        static haptic_direction spherical(int32_t azimuth, int32_t elevation) noexcept {
            return {haptic_direction_type::spherical, {azimuth, elevation, 0}};
        }

        /**
         * @brief Create a steering wheel axis direction
         */
        static haptic_direction steering() noexcept {
            return {haptic_direction_type::steering_axis, {0, 0, 0}};
        }

        // Convert to SDL type
        [[nodiscard]] SDL_HapticDirection to_sdl() const noexcept {
            SDL_HapticDirection haptic_dir;
            haptic_dir.type = static_cast <Uint8>(type);
            haptic_dir.dir[0] = dir[0];
            haptic_dir.dir[1] = dir[1];
            haptic_dir.dir[2] = dir[2];
            return haptic_dir;
        }
    };

    /**
     * @brief Base for all haptic effects
     */
    struct haptic_effect_base {
        uint32_t length = 1000; // Duration in milliseconds
        uint16_t delay = 0; // Delay before starting
        uint16_t button = 0; // Button that triggers effect (0 = no button)
        uint16_t interval = 0; // Minimum time between triggers

        // Envelope (not used for condition effects)
        uint16_t attack_length = 0; // Attack duration
        uint16_t attack_level = 0; // Attack start level
        uint16_t fade_length = 0; // Fade duration
        uint16_t fade_level = 0; // Fade end level
    };

    /**
     * @brief Constant force effect
     */
    struct haptic_constant : haptic_effect_base {
        haptic_direction direction;
        int16_t level = 0; // Strength of constant effect
    };

    /**
     * @brief Periodic effect (sine, square, triangle, sawtooth)
     */
    struct haptic_periodic : haptic_effect_base {
        haptic_feature wave_type = haptic_feature::sine; // Wave type
        haptic_direction direction;
        uint16_t period = 1000; // Period of wave in ms
        int16_t magnitude = 0; // Peak value
        int16_t offset = 0; // Mean value of wave
        uint16_t phase = 0; // Phase shift (hundredths of degree)
    };

    /**
     * @brief Condition effect (spring, damper, inertia, friction)
     */
    struct haptic_condition : haptic_effect_base {
        haptic_feature condition_type = haptic_feature::spring;
        haptic_direction direction;

        // Per-axis parameters (up to 3 axes)
        std::array <uint16_t, 3> right_sat = {0xFFFF, 0xFFFF, 0xFFFF}; // Positive saturation
        std::array <uint16_t, 3> left_sat = {0xFFFF, 0xFFFF, 0xFFFF}; // Negative saturation
        std::array <int16_t, 3> right_coeff = {0, 0, 0}; // Positive coefficient
        std::array <int16_t, 3> left_coeff = {0, 0, 0}; // Negative coefficient
        std::array <uint16_t, 3> deadband = {0, 0, 0}; // Dead zone size
        std::array <int16_t, 3> center = {0, 0, 0}; // Dead zone center
    };

    /**
     * @brief Ramp effect
     */
    struct haptic_ramp : haptic_effect_base {
        haptic_direction direction;
        int16_t start = 0; // Starting strength
        int16_t end = 0; // Ending strength
    };

    /**
     * @brief Left/Right motor effect
     */
    struct haptic_leftright {
        uint32_t length = 1000; // Duration in milliseconds
        uint16_t large_magnitude = 0; // Large motor strength
        uint16_t small_magnitude = 0; // Small motor strength
    };

    /**
     * @brief Custom effect
     */
    struct haptic_custom : haptic_effect_base {
        haptic_direction direction;
        uint8_t channels = 1; // Number of channels
        uint16_t period = 1000; // Sample period
        std::vector <uint16_t> data; // Sample data
    };

    /**
     * @brief Variant type for all haptic effects
     */
    using haptic_effect = std::variant <
        haptic_constant,
        haptic_periodic,
        haptic_condition,
        haptic_ramp,
        haptic_leftright,
        haptic_custom
    >;

    /**
     * @brief Convert our effect to SDL effect
     */
    namespace detail {
        inline SDL_HapticEffect to_sdl_effect(const haptic_effect& effect) {
            SDL_HapticEffect sdl_effect{};

            std::visit([&sdl_effect](const auto& e) {
                using T = std::decay_t <decltype(e)>;

                if constexpr (std::is_same_v <T, haptic_constant>) {
                    sdl_effect.type = SDL_HAPTIC_CONSTANT;
                    auto& c = sdl_effect.constant;
                    c.type = SDL_HAPTIC_CONSTANT;
                    c.direction = e.direction.to_sdl();
                    c.length = e.length;
                    c.delay = e.delay;
                    c.button = e.button;
                    c.interval = e.interval;
                    c.level = e.level;
                    c.attack_length = e.attack_length;
                    c.attack_level = e.attack_level;
                    c.fade_length = e.fade_length;
                    c.fade_level = e.fade_level;
                } else if constexpr (std::is_same_v <T, haptic_periodic>) {
                    sdl_effect.type = static_cast <Uint16>(e.wave_type);
                    auto& p = sdl_effect.periodic;
                    p.type = static_cast <Uint16>(e.wave_type);
                    p.direction = e.direction.to_sdl();
                    p.length = e.length;
                    p.delay = e.delay;
                    p.button = e.button;
                    p.interval = e.interval;
                    p.period = e.period;
                    p.magnitude = e.magnitude;
                    p.offset = e.offset;
                    p.phase = e.phase;
                    p.attack_length = e.attack_length;
                    p.attack_level = e.attack_level;
                    p.fade_length = e.fade_length;
                    p.fade_level = e.fade_level;
                } else if constexpr (std::is_same_v <T, haptic_condition>) {
                    sdl_effect.type = static_cast <Uint16>(e.condition_type);
                    auto& c = sdl_effect.condition;
                    c.type = static_cast <Uint16>(e.condition_type);
                    c.direction = e.direction.to_sdl();
                    c.length = e.length;
                    c.delay = e.delay;
                    c.button = e.button;
                    c.interval = e.interval;
                    for (size_t i = 0; i < 3; ++i) {
                        c.right_sat[i] = e.right_sat[i];
                        c.left_sat[i] = e.left_sat[i];
                        c.right_coeff[i] = e.right_coeff[i];
                        c.left_coeff[i] = e.left_coeff[i];
                        c.deadband[i] = e.deadband[i];
                        c.center[i] = e.center[i];
                    }
                } else if constexpr (std::is_same_v <T, haptic_ramp>) {
                    sdl_effect.type = SDL_HAPTIC_RAMP;
                    auto& r = sdl_effect.ramp;
                    r.type = SDL_HAPTIC_RAMP;
                    r.direction = e.direction.to_sdl();
                    r.length = e.length;
                    r.delay = e.delay;
                    r.button = e.button;
                    r.interval = e.interval;
                    r.start = e.start;
                    r.end = e.end;
                    r.attack_length = e.attack_length;
                    r.attack_level = e.attack_level;
                    r.fade_length = e.fade_length;
                    r.fade_level = e.fade_level;
                } else if constexpr (std::is_same_v <T, haptic_leftright>) {
                    sdl_effect.type = SDL_HAPTIC_LEFTRIGHT;
                    auto& lr = sdl_effect.leftright;
                    lr.type = SDL_HAPTIC_LEFTRIGHT;
                    lr.length = e.length;
                    lr.large_magnitude = e.large_magnitude;
                    lr.small_magnitude = e.small_magnitude;
                } else if constexpr (std::is_same_v <T, haptic_custom>) {
                    sdl_effect.type = SDL_HAPTIC_CUSTOM;
                    auto& c = sdl_effect.custom;
                    c.type = SDL_HAPTIC_CUSTOM;
                    c.direction = e.direction.to_sdl();
                    c.length = e.length;
                    c.delay = e.delay;
                    c.button = e.button;
                    c.interval = e.interval;
                    c.channels = e.channels;
                    c.period = e.period;
                    c.samples = static_cast <Uint16>(e.data.size() / e.channels);
                    c.data = const_cast <Uint16*>(e.data.data()); // SDL doesn't modify
                    c.attack_length = e.attack_length;
                    c.attack_level = e.attack_level;
                    c.fade_length = e.fade_length;
                    c.fade_level = e.fade_level;
                }
            }, effect);

            return sdl_effect;
        }
    }

    /**
     * @brief Smart pointer type for SDL_Haptic with automatic cleanup
     */
    using haptic_ptr = pointer <SDL_Haptic, SDL_CloseHaptic>;

    /**
     * @brief Get list of available haptic devices
     *
     * @return Vector of haptic device IDs
     */
    [[nodiscard]] inline std::vector <haptic_id> get_haptics() {
        int count = 0;
        SDL_HapticID* haptics = SDL_GetHaptics(&count);
        if (!haptics || count <= 0) {
            return {};
        }
        std::vector <haptic_id> haptic_ids(haptics, haptics + count);
        SDL_free(haptics);
        return haptic_ids;
    }

    /**
     * @brief Get the name of a haptic device
     *
     * This can be called before the device is opened.
     *
     * @param instance_id The haptic device instance ID
     * @return Device name, or empty string if invalid
     */
    [[nodiscard]] inline std::string get_haptic_name_for_id(haptic_id instance_id) {
        const char* name = SDL_GetHapticNameForID(instance_id);
        return name ? name : "";
    }

    /**
     * @brief Check if the mouse has haptic capabilities
     *
     * @return true if the mouse is haptic capable
     */
    [[nodiscard]] inline bool is_mouse_haptic() noexcept {
        return SDL_IsMouseHaptic();
    }

    /**
     * @brief RAII wrapper for SDL_Haptic
     *
     * This class provides a safe, RAII-managed interface to SDL's haptic functionality.
     * The haptic device is automatically closed when the object goes out of scope.
     */
    class haptic {
        private:
            haptic_ptr ptr;

        public:
            /**
             * @brief Effect handle type
             */
            using effect_id = int;

            /**
             * @brief Default constructor - creates an empty haptic device
             */
            haptic() = default;

            /**
             * @brief Construct from existing SDL_Haptic pointer
             * @param h Existing SDL_Haptic pointer (takes ownership)
             */
            explicit haptic(SDL_Haptic* h)
                : ptr(h) {
            }

            /**
             * @brief Move constructor
             */
            haptic(haptic&&) = default;

            /**
             * @brief Move assignment operator
             */
            haptic& operator=(haptic&&) = default;

            // Delete copy operations - haptic devices are move-only
            haptic(const haptic&) = delete;
            haptic& operator=(const haptic&) = delete;

            /**
             * @brief Check if the haptic device is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Haptic pointer
             */
            [[nodiscard]] SDL_Haptic* get() const noexcept { return ptr.get(); }

            /**
             * @brief Open a haptic device
             *
             * @param instance_id The haptic device instance ID
             * @return Expected containing haptic device, or error message
             */
            static expected <haptic, std::string> open(haptic_id instance_id) {
                SDL_Haptic* h = SDL_OpenHaptic(instance_id);
                if (!h) {
                    return make_unexpectedf(get_error());
                }
                return haptic(h);
            }

            /**
             * @brief Open haptic device from mouse
             *
             * @return Expected containing haptic device, or error message
             */
            static expected <haptic, std::string> open_from_mouse() {
                SDL_Haptic* h = SDL_OpenHapticFromMouse();
                if (!h) {
                    return make_unexpectedf(get_error());
                }
                return haptic(h);
            }

            /**
             * @brief Open haptic device from joystick
             *
             * @param joy The joystick to open haptic from
             * @return Expected containing haptic device, or error message
             */
            static expected <haptic, std::string> open_from_joystick(const joystick& joy) {
                if (!joy.is_valid()) {
                    return make_unexpectedf("Invalid joystick");
                }
                SDL_Haptic* h = SDL_OpenHapticFromJoystick(joy.get());
                if (!h) {
                    return make_unexpectedf(get_error());
                }
                return haptic(h);
            }

            /**
             * @brief Get haptic device from ID
             *
             * @param instance_id The haptic device instance ID
             * @return Non-owning pointer to haptic device, or nullptr if not found
             */
            static SDL_Haptic* get_from_id(haptic_id instance_id) noexcept {
                return SDL_GetHapticFromID(instance_id);
            }

            /**
             * @brief Get the instance ID of this haptic device
             *
             * @return Instance ID, or 0 if invalid
             */
            [[nodiscard]] haptic_id get_id() const noexcept {
                return ptr ? SDL_GetHapticID(ptr.get()) : 0;
            }

            /**
             * @brief Get the name of this haptic device
             *
             * @return Device name, or empty string if invalid
             */
            [[nodiscard]] std::string get_name() const {
                if (!ptr) return "";
                const char* name = SDL_GetHapticName(ptr.get());
                return name ? name : "";
            }

            /**
             * @brief Get maximum number of effects that can be stored
             *
             * @return Maximum effects
             */
            [[nodiscard]] expected<size_t, std::string> get_max_effects() const {
                if (!ptr) return make_unexpectedf("Invalid haptic device");
                int max = SDL_GetMaxHapticEffects(ptr.get());
                if (max < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(max);
            }

            /**
             * @brief Get maximum number of effects that can play simultaneously
             *
             * @return Maximum simultaneous effects
             */
            [[nodiscard]] expected<size_t, std::string> get_max_effects_playing() const {
                if (!ptr) return make_unexpectedf("Invalid haptic device");
                int max = SDL_GetMaxHapticEffectsPlaying(ptr.get());
                if (max < 0) {
                    return make_unexpectedf(get_error());
                }
                return static_cast<size_t>(max);
            }

            /**
             * @brief Get supported features
             *
             * @return Bitmask of supported features
             */
            [[nodiscard]] haptic_feature get_features() const noexcept {
                return ptr
                           ? static_cast <haptic_feature>(SDL_GetHapticFeatures(ptr.get()))
                           : static_cast <haptic_feature>(0);
            }

            /**
             * @brief Get number of haptic axes
             *
             * @return Number of axes, or 0 on error
             */
            [[nodiscard]] size_t get_num_axes() const noexcept {
                if (!ptr) return 0;
                int count = SDL_GetNumHapticAxes(ptr.get());
                return count > 0 ? static_cast<size_t>(count) : 0;
            }

            /**
             * @brief Check if an effect is supported
             *
             * @param effect The effect to check
             * @return true if supported, false otherwise
             */
            [[nodiscard]] bool is_effect_supported(const haptic_effect& effect) const {
                if (!ptr) return false;
                SDL_HapticEffect sdl_effect = detail::to_sdl_effect(effect);
                return SDL_HapticEffectSupported(ptr.get(), &sdl_effect);
            }

            /**
             * @brief Create a haptic effect
             *
             * @param effect The effect to create
             * @return Expected containing effect ID, or error message
             */
            [[nodiscard]] expected <effect_id, std::string> create_effect(const haptic_effect& effect) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                SDL_HapticEffect sdl_effect = detail::to_sdl_effect(effect);
                int id = SDL_CreateHapticEffect(ptr.get(), &sdl_effect);
                if (id < 0) {
                    return make_unexpectedf(get_error());
                }
                return id;
            }

            /**
             * @brief Update a haptic effect
             *
             * @param id The effect ID to update
             * @param effect The new effect data
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> update_effect(effect_id id, const haptic_effect& effect) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                SDL_HapticEffect sdl_effect = detail::to_sdl_effect(effect);
                if (!SDL_UpdateHapticEffect(ptr.get(), id, &sdl_effect)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Run a haptic effect
             *
             * @param id The effect ID to run
             * @param iterations Number of iterations (use haptic_infinity for infinite)
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> run_effect(effect_id id, uint32_t iterations = 1) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_RunHapticEffect(ptr.get(), id, iterations)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Stop a running haptic effect
             *
             * @param id The effect ID to stop
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> stop_effect(effect_id id) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_StopHapticEffect(ptr.get(), id)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Destroy a haptic effect
             *
             * @param id The effect ID to destroy
             */
            void destroy_effect(effect_id id) const noexcept {
                if (ptr && id >= 0) {
                    SDL_DestroyHapticEffect(ptr.get(), id);
                }
            }

            /**
             * @brief Get the status of a haptic effect
             *
             * @param id The effect ID to query
             * @return true if playing, false if not playing or on error
             */
            [[nodiscard]] bool get_effect_status(effect_id id) const noexcept {
                return ptr ? SDL_GetHapticEffectStatus(ptr.get(), id) : false;
            }

            /**
             * @brief Set the global gain of the haptic device
             *
             * @param gain The gain value (0-100)
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> set_gain(int gain) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_SetHapticGain(ptr.get(), gain)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Set the autocenter of the haptic device
             *
             * @param autocenter The autocenter value (0-100)
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> set_autocenter(int autocenter) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_SetHapticAutocenter(ptr.get(), autocenter)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Pause the haptic device
             *
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> pause() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_PauseHaptic(ptr.get())) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Resume the haptic device
             *
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> resume() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_ResumeHaptic(ptr.get())) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Stop all haptic effects
             *
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> stop_all_effects() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_StopHapticEffects(ptr.get())) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Check if rumble is supported
             *
             * @return true if rumble is supported
             */
            [[nodiscard]] bool is_rumble_supported() const noexcept {
                return ptr ? SDL_HapticRumbleSupported(ptr.get()) : false;
            }

            /**
             * @brief Initialize simple rumble
             *
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> init_rumble() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_InitHapticRumble(ptr.get())) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Play simple rumble effect
             *
             * @param strength Strength of rumble (0.0 to 1.0)
             * @param length Duration in milliseconds
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> play_rumble(float strength, uint32_t length) const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_PlayHapticRumble(ptr.get(), strength, length)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Play simple rumble effect with std::chrono duration
             *
             * @param strength Strength of rumble (0.0 to 1.0)
             * @param duration Duration of rumble
             * @return Expected<void> - empty on success, error message on failure
             */
            template<typename Rep, typename Period>
            [[nodiscard]] expected <void, std::string> play_rumble(float strength,
                                                                   std::chrono::duration <Rep, Period> duration) const {
                auto ms = std::chrono::duration_cast <std::chrono::milliseconds>(duration).count();
                return play_rumble(strength, static_cast <uint32_t>(ms));
            }

            /**
             * @brief Stop simple rumble
             *
             * @return Expected<void> - empty on success, error message on failure
             */
            [[nodiscard]] expected <void, std::string> stop_rumble() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid haptic device");
                }
                if (!SDL_StopHapticRumble(ptr.get())) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }
    };

    /**
     * @brief RAII helper for haptic effects
     *
     * This class manages the lifetime of a haptic effect, automatically
     * destroying it when the object goes out of scope.
     */
    class haptic_effect_handle {
        private:
            const haptic* device = nullptr;
            haptic::effect_id id = -1;

        public:
            haptic_effect_handle() = default;

            haptic_effect_handle(const haptic* dev, haptic::effect_id effect_id)
                : device(dev), id(effect_id) {
            }

            ~haptic_effect_handle() {
                if (device && id >= 0) {
                    device->destroy_effect(id);
                }
            }

            // Move only
            haptic_effect_handle(haptic_effect_handle&& other) noexcept
                : device(other.device), id(other.id) {
                other.device = nullptr;
                other.id = -1;
            }

            haptic_effect_handle& operator=(haptic_effect_handle&& other) noexcept {
                if (this != &other) {
                    if (device && id >= 0) {
                        device->destroy_effect(id);
                    }
                    device = other.device;
                    id = other.id;
                    other.device = nullptr;
                    other.id = -1;
                }
                return *this;
            }

            // Delete copy
            haptic_effect_handle(const haptic_effect_handle&) = delete;
            haptic_effect_handle& operator=(const haptic_effect_handle&) = delete;

            [[nodiscard]] bool is_valid() const noexcept { return device && id >= 0; }
            [[nodiscard]] haptic::effect_id get() const noexcept { return id; }

            [[nodiscard]] expected <void, std::string> run(uint32_t iterations = 1) const {
                if (!device || id < 0) {
                    return make_unexpectedf("Invalid effect handle");
                }
                return device->run_effect(id, iterations);
            }

            [[nodiscard]] expected <void, std::string> stop() const {
                if (!device || id < 0) {
                    return make_unexpectedf("Invalid effect handle");
                }
                return device->stop_effect(id);
            }

            [[nodiscard]] bool is_playing() const noexcept {
                return device && id >= 0 ? device->get_effect_status(id) : false;
            }
    };

    /**
     * @brief Check if a joystick has haptic capabilities
     *
     * @param joy The joystick to check
     * @return true if the joystick is haptic capable
     */
    [[nodiscard]] inline bool is_joystick_haptic(const joystick& joy) noexcept {
        return joy.is_valid() ? SDL_IsJoystickHaptic(joy.get()) : false;
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for haptic_feature
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, haptic_feature value);

/**
 * @brief Stream input operator for haptic_feature
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, haptic_feature& value);

/**
 * @brief Stream output operator for haptic_direction_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, haptic_direction_type value);

/**
 * @brief Stream input operator for haptic_direction_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, haptic_direction_type& value);

}