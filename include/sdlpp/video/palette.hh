//
// Created by igor on 7/13/25.
//

#pragma once

/**
 * @file palette.hh
 * @brief Palette management for SDL3 with proper ownership semantics
 * 
 * This header provides palette wrappers that correctly handle the complex
 * ownership model where palettes can be either owned (created by us) or
 * borrowed (owned by a surface).
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/core/error.hh>
#include <string>
#include <span>
#include <memory>

namespace sdlpp {
    /**
     * @brief Smart pointer type for owned SDL_Palette
     */
    using palette_ptr = pointer <SDL_Palette, SDL_DestroyPalette>;

    /**
     * @brief Non-owning const reference to a palette
     *
     * This class represents a palette that is owned by something else
     * (typically a surface). It provides read-only access to the palette
     * without managing its lifetime.
     */
    class const_palette_ref {
        protected:
            const SDL_Palette* ptr = nullptr;

        public:
            /**
             * @brief Default constructor - creates null reference
             */
            const_palette_ref() = default;

            /**
             * @brief Construct from SDL_Palette pointer
             * @param p SDL_Palette pointer (does not take ownership)
             */
            explicit const_palette_ref(const SDL_Palette* p)
                : ptr(p) {
            }

            /**
             * @brief Check if reference is valid
             */
            [[nodiscard]] bool is_valid() const { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Palette pointer
             */
            [[nodiscard]] const SDL_Palette* get() const { return ptr; }

            /**
             * @brief Get number of colors in the palette
             */
            [[nodiscard]] size_t size() const {
                return ptr ? static_cast<size_t>(ptr->ncolors) : 0;
            }

            /**
             * @brief Get color at index
             * @param index Color index
             * @return Color at index, or black if index is out of bounds
             */
            [[nodiscard]] color get_color(size_t index) const {
                if (!ptr || index >= static_cast<size_t>(ptr->ncolors)) {
                    return color{0, 0, 0, 255};
                }
                return color::from_sdl(ptr->colors[index]);
            }

            /**
             * @brief Get all colors as a span
             * @return Span of SDL_Color entries
             */
            [[nodiscard]] std::span <const SDL_Color> colors() const {
                if (!ptr || !ptr->colors) return {};
                return {ptr->colors, static_cast <size_t>(ptr->ncolors)};
            }

            /**
             * @brief Copy colors to a vector
             * @return Vector of colors
             */
            [[nodiscard]] std::vector <color> to_vector() const {
                std::vector <color> colors_vec;
                if (ptr && ptr->colors) {
                    colors_vec.reserve(static_cast <size_t>(ptr->ncolors));
                    for (int i = 0; i < ptr->ncolors; ++i) {
                        colors_vec.push_back(color::from_sdl(ptr->colors[i]));
                    }
                }
                return colors_vec;
            }
    };

    /**
     * @brief Non-owning mutable reference to a palette
     *
     * This class represents a palette that is owned by something else
     * but allows modification. Use with caution as the palette lifetime
     * is not managed by this reference.
     */
    class palette_ref : public const_palette_ref {
        public:
            /**
             * @brief Default constructor - creates null reference
             */
            palette_ref() = default;

            /**
             * @brief Construct from SDL_Palette pointer
             * @param p SDL_Palette pointer (does not take ownership)
             */
            explicit palette_ref(SDL_Palette* p)
                : const_palette_ref(p) {
            }

            /**
             * @brief Get the underlying SDL_Palette pointer (mutable)
             */
            [[nodiscard]] SDL_Palette* get() const {
                return const_cast <SDL_Palette*>(ptr);
            }

            /**
             * @brief Set color at index
             * @param index Color index
             * @param c Color to set
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_color(size_t index, const color& c) {
                if (!ptr) {
                    return make_unexpected("Invalid palette");
                }
                if (index >= static_cast<size_t>(ptr->ncolors)) {
                    return make_unexpected("Index out of bounds");
                }

                SDL_Color colors[1] = {c.to_sdl()};
                if (!SDL_SetPaletteColors(get(), colors, static_cast<int>(index), 1)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Set multiple colors
             * @param colors Colors to set
             * @param first_index Starting index
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_colors(std::span <const color> colors,
                                                    int first_index = 0) {
                if (!ptr) {
                    return make_unexpected("Invalid palette");
                }
                if (first_index < 0 || first_index + static_cast <int>(colors.size()) > ptr->ncolors) {
                    return make_unexpected("Index out of bounds");
                }

                // Convert to SDL_Color array
                std::vector <SDL_Color> sdl_colors;
                sdl_colors.reserve(colors.size());
                for (const auto& c : colors) {
                    sdl_colors.push_back(c.to_sdl());
                }

                if (!SDL_SetPaletteColors(get(), sdl_colors.data(),
                                          first_index, static_cast <int>(colors.size()))) {
                    return make_unexpected(get_error());
                }

                return {};
            }
    };

    /**
     * @brief Owning palette wrapper with RAII semantics
     *
     * This class represents a palette that we own and must free.
     * It provides full read/write access to the palette.
     */
    class palette {
        private:
            palette_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates empty palette
             */
            palette() = default;

            /**
             * @brief Construct from existing SDL_Palette pointer
             * @param p SDL_Palette pointer (takes ownership)
             */
            explicit palette(SDL_Palette* p)
                : ptr(p) {
            }

            /**
             * @brief Move constructor
             */
            palette(palette&&) = default;

            /**
             * @brief Move assignment
             */
            palette& operator=(palette&&) = default;

            // Delete copy operations
            palette(const palette&) = delete;
            palette& operator=(const palette&) = delete;

            /**
             * @brief Check if palette is valid
             */
            [[nodiscard]] bool is_valid() const { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Palette pointer
             */
            [[nodiscard]] SDL_Palette* get() const { return ptr.get(); }

            /**
             * @brief Get a non-owning mutable reference to this palette
             */
            [[nodiscard]] palette_ref ref() { return palette_ref(ptr.get()); }

            /**
             * @brief Get a non-owning const reference to this palette
             */
            [[nodiscard]] const_palette_ref ref() const { return const_palette_ref(ptr.get()); }

            /**
             * @brief Get a non-owning const reference to this palette
             */
            [[nodiscard]] const_palette_ref cref() const { return const_palette_ref(ptr.get()); }

            /**
             * @brief Implicit conversion to palette_ref for convenience
             */
            [[nodiscard]] operator palette_ref() { return ref(); }

            /**
             * @brief Implicit conversion to const_palette_ref for convenience
             */
            [[nodiscard]] operator const_palette_ref() const { return cref(); }

            /**
             * @brief Get number of colors in the palette
             */
            [[nodiscard]] size_t size() const {
                return ptr ? static_cast<size_t>(ptr->ncolors) : 0;
            }

            /**
             * @brief Get color at index
             * @param index Color index
             * @return Color at index, or black if index is out of bounds
             */
            [[nodiscard]] color get_color(size_t index) const {
                if (!ptr || index >= static_cast<size_t>(ptr->ncolors)) {
                    return color{0, 0, 0, 255};
                }
                return color::from_sdl(ptr->colors[index]);
            }

            /**
             * @brief Set color at index
             * @param index Color index
             * @param c Color to set
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_color(size_t index, const color& c) {
                if (!ptr) {
                    return make_unexpected("Invalid palette");
                }
                if (index >= static_cast<size_t>(ptr->ncolors)) {
                    return make_unexpected("Index out of bounds");
                }

                SDL_Color colors[1] = {c.to_sdl()};
                if (!SDL_SetPaletteColors(ptr.get(), colors, static_cast<int>(index), 1)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Set multiple colors
             * @param colors Colors to set
             * @param first_index Starting index
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_colors(std::span <const color> colors,
                                                    int first_index = 0) {
                if (!ptr) {
                    return make_unexpected("Invalid palette");
                }
                if (first_index < 0 || first_index + static_cast <int>(colors.size()) > ptr->ncolors) {
                    return make_unexpected("Index out of bounds");
                }

                // Convert to SDL_Color array
                std::vector <SDL_Color> sdl_colors;
                sdl_colors.reserve(colors.size());
                for (const auto& c : colors) {
                    sdl_colors.push_back(c.to_sdl());
                }

                if (!SDL_SetPaletteColors(ptr.get(), sdl_colors.data(),
                                          first_index, static_cast <int>(colors.size()))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get all colors as a span
             * @return Span of SDL_Color entries
             */
            [[nodiscard]] std::span <const SDL_Color> colors() const {
                if (!ptr || !ptr->colors) return {};
                return {ptr->colors, static_cast <size_t>(ptr->ncolors)};
            }

            /**
             * @brief Copy colors to a vector
             * @return Vector of colors
             */
            [[nodiscard]] std::vector <color> to_vector() const {
                std::vector <color> colors_vec;
                if (ptr && ptr->colors) {
                    colors_vec.reserve(static_cast <size_t>(ptr->ncolors));
                    for (int i = 0; i < ptr->ncolors; ++i) {
                        colors_vec.push_back(color::from_sdl(ptr->colors[i]));
                    }
                }
                return colors_vec;
            }

            /**
             * @brief Create a palette with specified number of colors
             * @param ncolors Number of colors (typically 256 for 8-bit)
             * @return Expected containing palette, or error message
             */
            static expected <palette, std::string> create(int ncolors) {
                SDL_Palette* p = SDL_CreatePalette(ncolors);
                if (!p) {
                    return make_unexpected(get_error());
                }
                return palette(p);
            }

            /**
             * @brief Create a standard grayscale palette
             * @param bits Bits per pixel (1-8)
             * @return Expected containing palette, or error message
             */
            static expected <palette, std::string> create_grayscale(int bits = 8) {
                if (bits < 1 || bits > 8) {
                    return make_unexpected("Bits must be between 1 and 8");
                }

                int ncolors = 1 << bits;
                auto pal_result = create(ncolors);
                if (!pal_result) {
                    return make_unexpected(pal_result.error());
                }

                auto& pal = *pal_result;
                std::vector <color> colors;
                colors.reserve(static_cast <size_t>(ncolors));

                for (int i = 0; i < ncolors; ++i) {
                    uint8_t gray = static_cast <uint8_t>((i * 255) / (ncolors - 1));
                    colors.emplace_back(gray, gray, gray);
                }

                auto set_result = pal.set_colors(colors);
                if (!set_result) {
                    return make_unexpected(set_result.error());
                }

                return std::move(pal);
            }

            /**
             * @brief Create a palette with a color ramp
             * @param start Start color
             * @param end End color
             * @param steps Number of steps (must be >= 2)
             * @return Expected containing palette, or error message
             */
            static expected <palette, std::string> create_ramp(const color& start,
                                                               const color& end,
                                                               int steps = 256) {
                if (steps < 2) {
                    return make_unexpected("Steps must be at least 2");
                }

                auto pal_result = create(steps);
                if (!pal_result) {
                    return make_unexpected(pal_result.error());
                }

                auto& pal = *pal_result;
                std::vector <color> colors;
                colors.reserve(static_cast <size_t>(steps));

                for (int i = 0; i < steps; ++i) {
                    float t = static_cast <float>(i) / static_cast <float>(steps - 1);
                    colors.push_back(lerp(start, end, t));
                }

                auto set_result = pal.set_colors(colors);
                if (!set_result) {
                    return make_unexpected(set_result.error());
                }

                return std::move(pal);
            }
    };
} // namespace sdlpp
