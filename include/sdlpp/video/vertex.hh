//
// Created by igor on 2026-11-22.
//

#pragma once

/**
 * @file vertex.hh
 * @brief A corner of a triangle drawn by renderer::render_geometry().
 */

#include <cstddef>
#include <type_traits>

#include <sdlpp/core/sdl.hh>
#include <sdlpp/utility/coordinate_spaces.hh>
#include <sdlpp/video/color.hh>

namespace sdlpp {
    /**
     * @brief A corner of a triangle drawn by renderer::render_geometry(): where it is, its
     *        colour, and the point of the texture it shows.
     *
     * Laid out like SDL_Vertex (checked below), so a span of them goes to SDL without copying.
     */
    struct vertex {
        /// @brief Where the corner is, in render coordinates, like every draw call.
        render_pos position;
        /// @brief Multiplies the texture's colour; without a texture, the colour itself.
        fcolor color{colors::f::white};
        /// @brief The point of the texture shown at this corner, as fractions of its size
        ///        (texture::to_uv()). Ignored without a texture.
        uv_pos tex_coord;
    };

    static_assert(std::is_standard_layout_v <vertex> && std::is_trivially_copyable_v <vertex>);
    static_assert(sizeof(vertex) == sizeof(SDL_Vertex)
                  && offsetof(vertex, position) == offsetof(SDL_Vertex, position)
                  && offsetof(vertex, color) == offsetof(SDL_Vertex, color)
                  && offsetof(vertex, tex_coord) == offsetof(SDL_Vertex, tex_coord));
}
