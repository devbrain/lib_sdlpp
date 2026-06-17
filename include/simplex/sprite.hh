#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <functional>
#include <simplex/detail/export.hh>
#include <simplex/geometry.hh>

namespace simplex {
    /**
     * @brief Defines a sequence of frame indices and playback properties.
     */
    struct SIMPLEX_EXPORT animation_sequence {
        std::vector<std::size_t> frames; ///< Indices of the atlas frames
        float fps{10.0f};                ///< Frames per second
        bool loop{true};                 ///< Whether the animation loops
    };

    /**
     * @brief A single step in an automated sprite movement script.
     */
    struct SIMPLEX_EXPORT movement_step {
        point target_offset{};        ///< Relative offset to move from the start of the step.
        float duration{0.0f};         ///< Duration of this step in seconds.

        /**
         * @brief Optional easing function mapping normalized time [0.0, 1.0] to progress [0.0, 1.0].
         *
         * If nullptr, linear interpolation is used (constant velocity).
         * 
         * Easing functions alter the velocity profile. Examples:
         * - Quadratic Ease-In:     [](float t) { return t * t; }
         * - Quadratic Ease-Out:    [](float t) { return t * (2.0f - t); }
         * - Quadratic Ease-InOut:  [](float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f; }
         * - Back Out (Overshoot):  [](float t) { float t1 = t - 1.0f; return 1.0f + 2.70158f * std::pow(t1, 3.0f) + 1.70158f * std::pow(t1, 2.0f); }
         */
        std::function<float(float)> easing_fn{nullptr};
    };

    /**
     * @brief An automated animated sprite state machine, wrapping animation timers.
     */
    class SIMPLEX_EXPORT animated_sprite {
        public:
            point position{};
            bool visible{true};
            bool flip_horizontal{false};
            bool flip_vertical{false};
            bool flip_diagonal{false};

            animated_sprite() = default;

            /**
             * @brief Add a named animation sequence.
             */
            void add_animation(std::string name, animation_sequence seq);

            /**
             * @brief Play a named animation sequence.
             */
            void play(const std::string& name);

            /**
             * @brief Stop the current animation.
             */
            void stop() noexcept;

            /**
             * @brief Update the animation timeline.
             * @param dt Delta time in seconds.
             */
            void update(float dt);

            /**
             * @brief Get the currently active frame index of the atlas.
             */
            [[nodiscard]] std::size_t current_frame() const noexcept {
                return m_active_frame;
            }

            /**
             * @brief Get the name of the currently playing animation.
             */
            [[nodiscard]] const std::string& current_animation() const noexcept {
                return m_current_anim;
            }

            /**
             * @brief Queue a movement step.
             */
            void queue_movement(movement_step step);

            /**
             * @brief Clear all queued movements and stop active movement.
             */
            void clear_movements() noexcept;

            /**
             * @brief Check if the sprite is currently executing a movement script.
             */
            [[nodiscard]] bool is_moving() const noexcept {
                return !m_movement_queue.empty();
            }

        private:
            std::unordered_map<std::string, animation_sequence> m_animations;
            std::string m_current_anim;
            std::size_t m_current_frame_idx{0};
            std::size_t m_active_frame{0};
            float m_timer{0.0f};

            std::deque<movement_step> m_movement_queue;
            float m_move_timer{0.0f};
            point m_move_start_pos{};
    };
} // namespace simplex
