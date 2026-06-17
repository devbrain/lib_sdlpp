#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <utility>
#include <sdlpp/video/texture.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/detail/expected.hh>
#include <simplex/detail/export.hh>
#include <simplex/geometry.hh>

namespace simplex {
    /**
     * @brief A CPU-resident bitmask representing solid vs transparent pixels of a sprite frame.
     *
     * Used for per-pixel collision detection.
     */
    class SIMPLEX_EXPORT bitmask {
        public:
            bitmask() = default;

            /**
             * @brief Construct an empty bitmask with specified dimensions.
             */
            bitmask(int width, int height);

            [[nodiscard]] int width() const noexcept { return m_width; }
            [[nodiscard]] int height() const noexcept { return m_height; }

            /**
             * @brief Set the collision bit at coordinates (x, y).
             */
            void set(int x, int y, bool val);

            /**
             * @brief Get the collision bit at coordinates (x, y).
             */
            [[nodiscard]] bool get(int x, int y) const noexcept;

            /**
             * @brief Checks if this bitmask overlaps/collides with another bitmask.
             * @param other The other bitmask to test against.
             * @param dx Relative X offset of the other bitmask.
             * @param dy Relative Y offset of the other bitmask.
             * @return true if solid pixels overlap, false otherwise.
             */
            [[nodiscard]] bool overlaps(const bitmask& other, int dx, int dy) const noexcept;

        private:
            int m_width{0};
            int m_height{0};
            std::vector<bool> m_bits;
    };

    /**
     * @brief A sprite atlas partitions a single texture into sub-rectangles (frames).
     *
     * It owns the underlying `sdlpp::texture` and maintains a list of source rectangles
     * representing individual frames. Optionally, it can store CPU-resident per-frame
     * bitmasks for pixel-precise collision detection.
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

            /**
             * @brief Construct a sprite atlas from a surface, slicing it and optionally generating bitmasks.
             * @param r Renderer context to upload the texture to.
             * @param surf The source surface.
             * @param frame_width Width of each frame in physical pixels.
             * @param frame_height Height of each frame in physical pixels.
             * @param generate_masks If true, bitmasks are generated from the surface alpha channel.
             */
            sprite_atlas(
                sdlpp::renderer& r,
                const sdlpp::surface& surf,
                int frame_width,
                int frame_height,
                bool generate_masks = false);

            /**
             * @brief Construct a sprite atlas from a surface and explicit rects, optionally generating bitmasks.
             * @param r Renderer context to upload the texture to.
             * @param surf The source surface.
             * @param frames Frame rectangles in physical pixels.
             * @param generate_masks If true, bitmasks are generated from the surface alpha channel.
             */
            sprite_atlas(
                sdlpp::renderer& r,
                const sdlpp::surface& surf,
                std::vector<sdlpp::rect<int>> frames,
                bool generate_masks = false);

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
             * @brief Check if this atlas contains bitmasks.
             */
            [[nodiscard]] bool has_bitmasks() const noexcept { return !m_masks.empty(); }

            /**
             * @brief Get the bitmask of a frame.
             * @param index Frame index (0-based).
             * @return The bitmask representing solid pixels for this frame.
             */
            [[nodiscard]] const bitmask& get_frame_mask(std::size_t index) const {
                return m_masks.at(index);
            }

            /**
             * @brief Set or update the bitmask of a frame.
             * @param index Frame index (0-based).
             * @param mask The user-supplied bitmask.
             */
            void set_frame_mask(std::size_t index, bitmask mask) {
                if (m_masks.size() < m_frames.size()) {
                    m_masks.resize(m_frames.size());
                }
                m_masks.at(index) = std::move(mask);
            }

            // Static factories/loaders

            /**
             * @brief Load a texture from file, slice it into a grid, and optionally generate bitmasks.
             * @param r Renderer context to upload the texture to.
             * @param path File system path to the image.
             * @param frame_width Width of each frame in physical pixels.
             * @param frame_height Height of each frame in physical pixels.
             * @param generate_masks If true, bitmasks are generated from the loaded image's alpha channel.
             * @return Expected containing the sprite_atlas, or error message.
             */
            [[nodiscard]] static sdlpp::expected<sprite_atlas, std::string> load(
                sdlpp::renderer& r,
                const std::filesystem::path& path,
                int frame_width,
                int frame_height,
                bool generate_masks = false);

            /**
             * @brief Load a texture from file, define custom frame rectangles, and optionally generate bitmasks.
             * @param r Renderer context to upload the texture to.
             * @param path File system path to the image.
             * @param frames Custom rectangles on the texture in physical pixels.
             * @param generate_masks If true, bitmasks are generated from the loaded image's alpha channel.
             * @return Expected containing the sprite_atlas, or error message.
             */
            [[nodiscard]] static sdlpp::expected<sprite_atlas, std::string> load(
                sdlpp::renderer& r,
                const std::filesystem::path& path,
                std::vector<sdlpp::rect<int>> frames,
                bool generate_masks = false);

            /**
             * @brief Load a vintage WGT .spr file and automatically generate a sprite atlas.
             * @param r Renderer context to upload the texture to.
             * @param path File system path to the .spr file.
             * @param generate_masks If true, bitmasks are generated from the sprite alpha channel.
             * @return Expected containing the sprite_atlas, or error message.
             */
            [[nodiscard]] static sdlpp::expected<sprite_atlas, std::string> load_wgt_spr(
                sdlpp::renderer& r,
                const std::filesystem::path& path,
                bool generate_masks = false);

        private:
            void slice_grid(int frame_width, int frame_height);
            void generate_bitmasks_from_surface(const sdlpp::surface& surf);

            sdlpp::texture m_texture;
            std::vector<sdlpp::rect<int>> m_frames;
            std::vector<bitmask> m_masks;
    };
} // namespace simplex
