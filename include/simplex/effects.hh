#pragma once

#include <simplex/detail/export.hh>
#include <simplex/mesh.hh>
#include <euler/angles/radian.hh>
#include <vector>

namespace simplex {

    /**
     * @brief Common base class for stateful sprite mesh deformation effects.
     */
    class SIMPLEX_EXPORT mesh_effect {
    public:
        virtual ~mesh_effect() = default;

        /**
         * @brief Apply the effect to the given sprite mesh.
         * @param mesh The target mesh to deform (must be laid out first).
         */
        virtual void apply(sprite_mesh& mesh) = 0;
    };

    /**
     * @brief Apply a horizontal sine wave deformation (waving flag / water ripple).
     */
    class SIMPLEX_EXPORT wave_horizontal_effect : public mesh_effect {
    public:
        wave_horizontal_effect(float amplitude = 0.0f, float frequency = 1.0f, float phase = 0.0f) noexcept
            : m_amplitude(amplitude), m_frequency(frequency), m_phase(phase) {}

        void apply(sprite_mesh& mesh) override;

        void update(float dt, float speed = 1.0f) noexcept {
            m_phase += dt * speed;
        }

        void set_amplitude(float amp) noexcept { m_amplitude = amp; }
        [[nodiscard]] float amplitude() const noexcept { return m_amplitude; }

        void set_frequency(float freq) noexcept { m_frequency = freq; }
        [[nodiscard]] float frequency() const noexcept { return m_frequency; }

        void set_phase(float phase) noexcept { m_phase = phase; }
        [[nodiscard]] float phase() const noexcept { return m_phase; }

    private:
        float m_amplitude;
        float m_frequency;
        float m_phase;
    };

    /**
     * @brief Apply a vertical sine wave deformation.
     */
    class SIMPLEX_EXPORT wave_vertical_effect : public mesh_effect {
    public:
        wave_vertical_effect(float amplitude = 0.0f, float frequency = 1.0f, float phase = 0.0f) noexcept
            : m_amplitude(amplitude), m_frequency(frequency), m_phase(phase) {}

        void apply(sprite_mesh& mesh) override;

        void update(float dt, float speed = 1.0f) noexcept {
            m_phase += dt * speed;
        }

        void set_amplitude(float amp) noexcept { m_amplitude = amp; }
        [[nodiscard]] float amplitude() const noexcept { return m_amplitude; }

        void set_frequency(float freq) noexcept { m_frequency = freq; }
        [[nodiscard]] float frequency() const noexcept { return m_frequency; }

        void set_phase(float phase) noexcept { m_phase = phase; }
        [[nodiscard]] float phase() const noexcept { return m_phase; }

    private:
        float m_amplitude;
        float m_frequency;
        float m_phase;
        std::vector<float> m_scratch_offsets;
    };

    /**
     * @brief Skew the mesh horizontally.
     */
    class SIMPLEX_EXPORT skew_horizontal_effect : public mesh_effect {
    public:
        explicit skew_horizontal_effect(float skew_x = 0.0f) noexcept
            : m_skew_x(skew_x) {}

        void apply(sprite_mesh& mesh) override;

        void set_skew_x(float skew_x) noexcept { m_skew_x = skew_x; }
        [[nodiscard]] float skew_x() const noexcept { return m_skew_x; }

    private:
        float m_skew_x;
    };

    /**
     * @brief Skew the mesh vertically.
     */
    class SIMPLEX_EXPORT skew_vertical_effect : public mesh_effect {
    public:
        explicit skew_vertical_effect(float skew_y = 0.0f) noexcept
            : m_skew_y(skew_y) {}

        void apply(sprite_mesh& mesh) override;

        void set_skew_y(float skew_y) noexcept { m_skew_y = skew_y; }
        [[nodiscard]] float skew_y() const noexcept { return m_skew_y; }

    private:
        float m_skew_y;
        std::vector<float> m_scratch_offsets;
    };

    /**
     * @brief Apply a perspective trapezoidal warp.
     */
    class SIMPLEX_EXPORT perspective_effect : public mesh_effect {
    public:
        perspective_effect(float top_taper = 1.0f, float bottom_taper = 1.0f) noexcept
            : m_top_taper(top_taper), m_bottom_taper(bottom_taper) {}

        void apply(sprite_mesh& mesh) override;

        void set_top_taper(float taper) noexcept { m_top_taper = taper; }
        [[nodiscard]] float top_taper() const noexcept { return m_top_taper; }

        void set_bottom_taper(float taper) noexcept { m_bottom_taper = taper; }
        [[nodiscard]] float bottom_taper() const noexcept { return m_bottom_taper; }

    private:
        float m_top_taper;
        float m_bottom_taper;
    };

    /**
     * @brief Apply a spherical pinch/punch deformation (bulge or sink).
     */
    class SIMPLEX_EXPORT pinch_punch_effect : public mesh_effect {
    public:
        pinch_punch_effect(float center_u = 0.5f, float center_v = 0.5f, float radius = 0.0f, float amount = 0.0f) noexcept
            : m_center_u(center_u), m_center_v(center_v), m_radius(radius), m_amount(amount) {}

        void apply(sprite_mesh& mesh) override;

        void set_center(float u, float v) noexcept { m_center_u = u; m_center_v = v; }
        [[nodiscard]] float center_u() const noexcept { return m_center_u; }
        [[nodiscard]] float center_v() const noexcept { return m_center_v; }

        void set_radius(float r) noexcept { m_radius = r; }
        [[nodiscard]] float radius() const noexcept { return m_radius; }

        void set_amount(float a) noexcept { m_amount = a; }
        [[nodiscard]] float amount() const noexcept { return m_amount; }

    private:
        float m_center_u;
        float m_center_v;
        float m_radius;
        float m_amount;
    };

    /**
     * @brief Apply a 3D rotation (pitch, yaw, roll) with perspective projection.
     */
    class SIMPLEX_EXPORT rotate_3d_effect : public mesh_effect {
    public:
        rotate_3d_effect(
            euler::radianf pitch = euler::radianf(0.0f),
            euler::radianf yaw = euler::radianf(0.0f),
            euler::radianf roll = euler::radianf(0.0f),
            float pivot_u = 0.5f,
            float pivot_v = 0.5f,
            float perspective_factor = 2.0f
        ) noexcept
            : m_pitch(pitch), m_yaw(yaw), m_roll(roll),
              m_pivot_u(pivot_u), m_pivot_v(pivot_v),
              m_perspective_factor(perspective_factor) {}

        void apply(sprite_mesh& mesh) override;

        void set_pitch(euler::radianf p) noexcept { m_pitch = p; }
        [[nodiscard]] euler::radianf pitch() const noexcept { return m_pitch; }

        void set_yaw(euler::radianf y) noexcept { m_yaw = y; }
        [[nodiscard]] euler::radianf yaw() const noexcept { return m_yaw; }

        void set_roll(euler::radianf r) noexcept { m_roll = r; }
        [[nodiscard]] euler::radianf roll() const noexcept { return m_roll; }

        void set_pivot(float u, float v) noexcept { m_pivot_u = u; m_pivot_v = v; }
        [[nodiscard]] float pivot_u() const noexcept { return m_pivot_u; }
        [[nodiscard]] float pivot_v() const noexcept { return m_pivot_v; }

        void set_perspective_factor(float f) noexcept { m_perspective_factor = f; }
        [[nodiscard]] float perspective_factor() const noexcept { return m_perspective_factor; }

    private:
        euler::radianf m_pitch;
        euler::radianf m_yaw;
        euler::radianf m_roll;
        float m_pivot_u;
        float m_pivot_v;
        float m_perspective_factor;
    };

} // namespace simplex
