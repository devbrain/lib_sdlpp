#pragma once

#include <simplex/detail/export.hh>
#include <simplex/mesh.hh>

namespace simplex::effects {
    /**
     * @brief Apply a horizontal sine wave deformation (waving flag / water ripple).
     * @param mesh The sprite mesh to deform (must be laid out first).
     * @param time The current animation time in seconds.
     * @param amplitude The wave amplitude in physical pixels.
     * @param frequency The wave frequency (cycles per grid height).
     */
    void SIMPLEX_EXPORT wave_horizontal(sprite_mesh& mesh, float time, float amplitude, float frequency) noexcept;

    /**
     * @brief Apply a vertical sine wave deformation.
     * @param mesh The sprite mesh to deform (must be laid out first).
     * @param time The current animation time in seconds.
     * @param amplitude The wave amplitude in physical pixels.
     * @param frequency The wave frequency (cycles per grid width).
     */
    void SIMPLEX_EXPORT wave_vertical(sprite_mesh& mesh, float time, float amplitude, float frequency) noexcept;

    /**
     * @brief Skew the mesh horizontally.
     * @param mesh The sprite mesh to deform.
     * @param skew_x The skew offset in physical pixels.
     */
    void SIMPLEX_EXPORT skew_horizontal(sprite_mesh& mesh, float skew_x) noexcept;

    /**
     * @brief Skew the mesh vertically.
     * @param mesh The sprite mesh to deform.
     * @param skew_y The skew offset in physical pixels.
     */
    void SIMPLEX_EXPORT skew_vertical(sprite_mesh& mesh, float skew_y) noexcept;

    /**
     * @brief Apply a perspective trapezoidal warp (e.g. Mode 7 / road shrink).
     * @param mesh The sprite mesh to deform.
     * @param top_taper Tapering factor at the top (0.0f = shrink to center point, 1.0f = no taper).
     * @param bottom_taper Tapering factor at the bottom.
     */
    void SIMPLEX_EXPORT perspective(sprite_mesh& mesh, float top_taper, float bottom_taper) noexcept;

    /**
     * @brief Apply a spherical pinch/punch deformation (bulge or sink).
     * @param mesh The sprite mesh to deform.
     * @param center_u Normalized center X coordinate [0, 1].
     * @param center_v Normalized center Y coordinate [0, 1].
     * @param radius Normalized radius [0, 1].
     * @param amount The deformation intensity (negative for pinch, positive for bulge/punch).
     */
    void SIMPLEX_EXPORT pinch_punch(sprite_mesh& mesh, float center_u, float center_v, float radius, float amount) noexcept;
} // namespace simplex::effects
