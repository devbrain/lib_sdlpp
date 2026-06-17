#pragma once

#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/utility/geometry_concepts.hh>
#include <simplex/detail/export.hh>
#include <simplex/geometry.hh>
#include <simplex/sprite_atlas.hh>

namespace simplex {
    /**
     * @brief A subdividable 2D quad mesh for sprite deformation, warping, and skewing.
     */
    class SIMPLEX_EXPORT sprite_mesh {
        public:
            sprite_mesh() = default;

            /**
             * @brief Construct a mesh with specified column and row subdivisions.
             */
            sprite_mesh(int cols, int rows);

            /**
             * @brief Position grid vertices mapping to a destination rectangle.
             * 
             * Automatically detects coordinate types: scales dp to physical pixels,
             * and keeps raw float/int values as-is.
             */
            template<sdlpp::rectangle_like DstRect, sdlpp::rectangle_like SrcRect>
            void layout_rect(const DstRect& dst, const SrcRect& src) {
                using DstValType = typename sdlpp::geometry_value_type<DstRect>::type;

                float dst_x = to_physical<DstValType>(sdlpp::get_x(dst));
                float dst_y = to_physical<DstValType>(sdlpp::get_y(dst));
                float dst_w = to_physical<DstValType>(sdlpp::get_width(dst));
                float dst_h = to_physical<DstValType>(sdlpp::get_height(dst));

                float src_x = static_cast<float>(sdlpp::get_x(src));
                float src_y = static_cast<float>(sdlpp::get_y(src));
                float src_w = static_cast<float>(sdlpp::get_width(src));
                float src_h = static_cast<float>(sdlpp::get_height(src));

                layout_rect_impl(dst_x, dst_y, dst_w, dst_h, src_x, src_y, src_w, src_h);
            }

            /**
             * @brief Position grid vertices mapping to a destination rectangle.
             * 
             * Overloaded for sdlpp::rect<float> to support default argument resolution.
             */
            template<sdlpp::rectangle_like DstRect>
            void layout_rect(const DstRect& dst, const sdlpp::rect<float>& src = {0.0f, 0.0f, 1.0f, 1.0f}) {
                using DstValType = typename sdlpp::geometry_value_type<DstRect>::type;

                float dst_x = to_physical<DstValType>(sdlpp::get_x(dst));
                float dst_y = to_physical<DstValType>(sdlpp::get_y(dst));
                float dst_w = to_physical<DstValType>(sdlpp::get_width(dst));
                float dst_h = to_physical<DstValType>(sdlpp::get_height(dst));

                layout_rect_impl(dst_x, dst_y, dst_w, dst_h, src.x, src.y, src.w, src.h);
            }

            /**
             * @brief Position grid vertices mapping to a specific frame of a sprite atlas.
             */
            template<sdlpp::rectangle_like DstRect>
            void layout_rect(const DstRect& dst, const sprite_atlas& atlas, std::size_t frame_index) {
                auto frame_rect = atlas.get_frame_rect(frame_index); // In pixels
                auto tex_size_res = atlas.texture().get_size();
                
                float tex_w = 1.0f;
                float tex_h = 1.0f;
                if (tex_size_res) {
                    tex_w = static_cast<float>(tex_size_res->width);
                    tex_h = static_cast<float>(tex_size_res->height);
                }

                sdlpp::rect<float> src_normalized{
                    static_cast<float>(frame_rect.x) / tex_w,
                    static_cast<float>(frame_rect.y) / tex_h,
                    static_cast<float>(frame_rect.w) / tex_w,
                    static_cast<float>(frame_rect.h) / tex_h
                };

                layout_rect(dst, src_normalized);
            }

            /**
             * @brief Apply a custom transformation callback to deform the physical mesh vertices.
             */
            void transform_vertices(std::function<void(int col, int row, float u, float v, float& px, float& py)> callback);

            /**
             * @brief Render the deformed mesh using the given texture.
             */
            sdlpp::expected<void, std::string> render(sdlpp::renderer& r, const sdlpp::texture& tex) const;

            [[nodiscard]] std::size_t vertex_count() const noexcept { return m_vertices.size(); }
            [[nodiscard]] std::size_t index_count() const noexcept { return m_indices.size(); }

            [[nodiscard]] int cols() const noexcept { return m_cols; }
            [[nodiscard]] int rows() const noexcept { return m_rows; }
            [[nodiscard]] std::vector<SDL_Vertex>& vertices() noexcept { return m_vertices; }
            [[nodiscard]] const std::vector<SDL_Vertex>& vertices() const noexcept { return m_vertices; }

        private:
            template<typename T>
            static constexpr float to_physical(T val) noexcept {
                if constexpr (std::is_same_v<T, simplex::dp>) {
                    return val.px();
                } else {
                    return static_cast<float>(val);
                }
            }

            void layout_rect_impl(float dx, float dy, float dw, float dh, float sx, float sy, float sw, float sh);

            void generate_mesh_indices();

            int m_cols{0};
            int m_rows{0};
            std::vector<SDL_Vertex> m_vertices;
            std::vector<int> m_indices;
    };
} // namespace simplex
