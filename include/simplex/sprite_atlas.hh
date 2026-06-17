#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <sdlpp/video/texture.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/detail/expected.hh>
#include <simplex/detail/export.hh>
#include <simplex/geometry.hh>

namespace simplex {
    /**
     * @brief A sprite atlas partitions a single texture into sub-rectangles (frames).
     *
     * It owns the underlying `sdlpp::texture` and maintains a list of source rectangles
     * representing individual frames of an animation or sprite sheet.
     */
    class SIMPLEX_EXPORT sprite_atlas {
        public:
            /**
             * @brief Construct an empty/invalid sprite atlas.
             */
            sprite_atlas() = default;

            /**
             * @brief Construct a sprite atlas from an existing texture, slicing it into a grid.
             * @param tex The texture to take ownership of.
             * @param frame_width Width of each frame in physical pixels.
             * @param frame_height Height of each frame in physical pixels.
             */
            sprite_atlas(sdlpp::texture tex, int frame_width, int frame_height);

            /**
             * @brief Construct a sprite atlas from an existing texture and explicit frame rectangles.
             * @param tex The texture to take ownership of.
             * @param frames The list of frame rectangles in physical pixels.
             */
            sprite_atlas(sdlpp::texture tex, std::vector<sdlpp::rect<int>> frames);

            // Move-only semantics (same as sdlpp::texture)
            sprite_atlas(sprite_atlas&&) noexcept = default;
            sprite_atlas& operator=(sprite_atlas&&) noexcept = default;

            sprite_atlas(const sprite_atlas&) = delete;
            sprite_atlas& operator=(const sprite_atlas&) = delete;

            /**
             * @brief Check if the atlas contains a valid texture.
             */
            [[nodiscard]] bool is_valid() const noexcept { return m_texture.is_valid(); }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying texture.
             */
            [[nodiscard]] const sdlpp::texture& texture() const noexcept { return m_texture; }

            /**
             * @brief Get the number of frames in this atlas.
             */
            [[nodiscard]] std::size_t frame_count() const noexcept { return m_frames.size(); }

            /**
             * @brief Get the physical pixel rectangle of a frame on the texture.
             * @param index Frame index (0-based).
             * @return The rectangle on the texture in physical pixels.
             */
            [[nodiscard]] sdlpp::rect<int> get_frame_rect(std::size_t index) const {
                return m_frames.at(index);
            }

            /**
             * @brief Get the design-pixel size of a frame.
             * @param index Frame index (0-based).
             * @return The size of the frame in dp (design pixels).
             */
            [[nodiscard]] size get_frame_size(std::size_t index) const {
                auto r = get_frame_rect(index);
                return size{dp{static_cast<float>(r.w)}, dp{static_cast<float>(r.h)}};
            }

            /**
             * @brief Load a texture from file and slice it into a grid.
             * @param r Renderer context to upload the texture to.
             * @param path File system path to the image.
             * @param frame_width Width of each frame in physical pixels.
             * @param frame_height Height of each frame in physical pixels.
             * @return Expected containing the sprite_atlas, or error message.
             */
            [[nodiscard]] static sdlpp::expected<sprite_atlas, std::string> load(
                sdlpp::renderer& r,
                const std::filesystem::path& path,
                int frame_width,
                int frame_height);

            /**
             * @brief Load a texture from file and define custom frame rectangles.
             * @param r Renderer context to upload the texture to.
             * @param path File system path to the image.
             * @param frames Custom rectangles on the texture in physical pixels.
             * @return Expected containing the sprite_atlas, or error message.
             */
            [[nodiscard]] static sdlpp::expected<sprite_atlas, std::string> load(
                sdlpp::renderer& r,
                const std::filesystem::path& path,
                std::vector<sdlpp::rect<int>> frames);

        private:
            void slice_grid(int frame_width, int frame_height);

            sdlpp::texture m_texture;
            std::vector<sdlpp::rect<int>> m_frames;
    };
} // namespace simplex
