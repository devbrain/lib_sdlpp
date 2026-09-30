//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file texture.hh
 * @brief Modern C++ wrapper for SDL3 texture functionality
 * 
 * This header provides RAII-managed wrappers around SDL3's texture system,
 * which represents images in GPU memory for fast rendering.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/detail/geometry_conversion.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/video/pixels.hh>
#include <sdlpp/video/blend_mode.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/video/palette.hh>
#include <string>

namespace sdlpp {
    /**
     * @brief Smart pointer type for SDL_Texture with automatic cleanup
     */
    using texture_ptr = pointer <SDL_Texture, SDL_DestroyTexture>;

    /**
     * @brief RAII wrapper for SDL_Texture
     *
     * This class provides a safe, RAII-managed interface to SDL's texture
     * functionality. Textures are GPU-resident images that can be rendered
     * quickly. The texture is automatically destroyed when the object goes
     * out of scope.
     */
    class texture {
        private:
            texture_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty texture
             */
            texture() = default;

            /**
             * @brief Construct from existing SDL_Texture pointer
             * @param t Existing SDL_Texture pointer (takes ownership)
             */
            explicit texture(SDL_Texture* t)
                : ptr(t) {
            }

            /**
             * @brief Move constructor
             */
            texture(texture&&) = default;

            /**
             * @brief Move assignment operator
             */
            texture& operator=(texture&&) = default;

            // Delete copy operations - textures are move-only
            texture(const texture&) = delete;
            texture& operator=(const texture&) = delete;

            /**
             * @brief Check if the texture is valid
             */
            [[nodiscard]] bool is_valid() const { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Texture pointer
             */
            [[nodiscard]] SDL_Texture* get() const { return ptr.get(); }

            /**
             * @brief Get texture properties
             * @return Expected containing properties ID, or error message
             */
            [[nodiscard]] expected <SDL_PropertiesID, std::string> get_properties() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                SDL_PropertiesID props = SDL_GetTextureProperties(ptr.get());
                if (!props) {
                    return make_unexpectedf(get_error());
                }

                return props;
            }

            /**
             * @brief Get texture size
             * @return Expected containing size, or error message
             */
            [[nodiscard]] expected <size_i, std::string> get_size() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                float w, h;
                if (!SDL_GetTextureSize(ptr.get(), &w, &h)) {
                    return make_unexpectedf(get_error());
                }

                return size_i{static_cast <int>(w), static_cast <int>(h)};
            }

            /**
             * @brief Set blend mode
             * @param mode Blend mode to set (defaults to none)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_blend_mode(blend_mode mode = blend_mode::none) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                if (!SDL_SetTextureBlendMode(ptr.get(), static_cast <SDL_BlendMode>(mode))) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get blend mode
             * @return Expected containing blend mode, or error message
             */
            [[nodiscard]] expected <blend_mode, std::string> get_blend_mode() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                SDL_BlendMode mode;
                if (!SDL_GetTextureBlendMode(ptr.get(), &mode)) {
                    return make_unexpectedf(get_error());
                }

                return static_cast <blend_mode>(mode);
            }

            /**
             * @brief Set color modulation
             * @param c Color for modulation (RGB components used)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_color_mod(const color& c) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                if (!SDL_SetTextureColorMod(ptr.get(), c.r, c.g, c.b)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get color modulation
             * @return Expected containing color, or error message
             */
            [[nodiscard]] expected <color, std::string> get_color_mod() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                uint8_t r, g, b;
                if (!SDL_GetTextureColorMod(ptr.get(), &r, &g, &b)) {
                    return make_unexpectedf(get_error());
                }

                return color{r, g, b, 255};
            }

            /**
             * @brief Set alpha modulation
             * @param alpha Alpha value (0-255)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_alpha_mod(uint8_t alpha) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                if (!SDL_SetTextureAlphaMod(ptr.get(), alpha)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get alpha modulation
             * @return Expected containing alpha value, or error message
             */
            [[nodiscard]] expected <uint8_t, std::string> get_alpha_mod() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                uint8_t alpha;
                if (!SDL_GetTextureAlphaMod(ptr.get(), &alpha)) {
                    return make_unexpectedf(get_error());
                }

                return alpha;
            }

            /**
             * @brief Set scale mode
             * @param mode Scale mode to use
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_scale_mode(scale_mode mode) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                if (!SDL_SetTextureScaleMode(ptr.get(), static_cast <SDL_ScaleMode>(mode))) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get scale mode
             * @return Expected containing scale mode, or error message
             */
            [[nodiscard]] expected <scale_mode, std::string> get_scale_mode() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                SDL_ScaleMode mode;
                if (!SDL_GetTextureScaleMode(ptr.get(), &mode)) {
                    return make_unexpectedf(get_error());
                }

                return static_cast <scale_mode>(mode);
            }

            /**
             * @brief Update texture with new pixel data (entire texture)
             * @param pixels Pixel data
             * @param pitch Number of bytes per row
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> update(const void* pixels, int pitch) {
                return update(std::nullopt, pixels, pitch);
            }

            /**
             * @brief Update texture with new pixel data (entire texture, explicit nullopt)
             * @param pixels Pixel data
             * @param pitch Number of bytes per row
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> update(std::nullopt_t, const void* pixels, int pitch) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                if (!pixels) {
                    return make_unexpectedf("Invalid pixel data");
                }

                if (!SDL_UpdateTexture(ptr.get(), nullptr, pixels, pitch)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Update texture sub-rectangle with new pixel data
             * @param update_rect Area to update
             * @param pixels Pixel data
             * @param pitch Number of bytes per row
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R>
            expected <void, std::string> update(const R& update_rect,
                                                const void* pixels, int pitch) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                if (!pixels) {
                    return make_unexpectedf("Invalid pixel data");
                }

                SDL_Rect sdl_rect = detail::to_sdl_rect(update_rect);
                if (!SDL_UpdateTexture(ptr.get(), &sdl_rect, pixels, pitch)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Update texture with new pixel data (optional sub-rectangle)
             * @param update_rect Area to update (nullopt for entire texture)
             * @param pixels Pixel data
             * @param pitch Number of bytes per row
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R>
            expected <void, std::string> update(const std::optional <R>& update_rect,
                                                const void* pixels, int pitch) {
                if (update_rect) {
                    return update(*update_rect, pixels, pitch);
                }
                return update(std::nullopt, pixels, pitch);
            }

            /**
             * @brief Lock texture for direct pixel access (entire texture)
             * @return Expected containing pixels pointer and pitch, or error message
             * @note Only works for streaming textures
             */
            expected <std::pair <void*, int>, std::string> lock(std::nullopt_t = std::nullopt) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                void* pixels = nullptr;
                int pitch = 0;

                if (!SDL_LockTexture(ptr.get(), nullptr, &pixels, &pitch)) {
                    return make_unexpectedf(get_error());
                }

                return std::make_pair(pixels, pitch);
            }

            /**
             * @brief Lock texture for direct pixel access (sub-rectangle)
             * @param lock_rect Area to lock
             * @return Expected containing pixels pointer and pitch, or error message
             * @note Only works for streaming textures
             */
            template<rect_like R>
            expected <std::pair <void*, int>, std::string> lock(const R& lock_rect) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                void* pixels = nullptr;
                int pitch = 0;

                SDL_Rect sdl_rect = detail::to_sdl_rect(lock_rect);
                if (!SDL_LockTexture(ptr.get(), &sdl_rect, &pixels, &pitch)) {
                    return make_unexpectedf(get_error());
                }

                return std::make_pair(pixels, pitch);
            }

            /**
             * @brief Lock texture for direct pixel access (optional sub-rectangle)
             * @param lock_rect Area to lock (nullopt for entire texture)
             * @return Expected containing pixels pointer and pitch, or error message
             * @note Only works for streaming textures
             */
            template<rect_like R>
            expected <std::pair <void*, int>, std::string> lock(const std::optional <R>& lock_rect) {
                if (lock_rect) {
                    return lock(*lock_rect);
                }
                return lock(std::nullopt);
            }

            /**
             * @brief Unlock texture after pixel access
             */
            void unlock() {
                if (ptr) {
                    SDL_UnlockTexture(ptr.get());
                }
            }

#if SDL_VERSION_ATLEAST(3, 4, 0)
            /**
             * @brief Set the palette used by this texture.
             * @param palette The palette structure to use.
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_palette(const const_palette_ref& palette) {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                SDL_Palette* pal_ptr = palette.is_valid() ? const_cast<SDL_Palette*>(palette.get()) : nullptr;
                if (!SDL_SetTexturePalette(ptr.get(), pal_ptr)) {
                    return make_unexpectedf(get_error());
                }

                return {};
            }

            /**
             * @brief Get the palette used by this texture.
             * @return Expected containing const_palette_ref, or error/empty
             */
            [[nodiscard]] expected <const_palette_ref, std::string> get_palette() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid texture");
                }

                SDL_Palette* pal = SDL_GetTexturePalette(ptr.get());
                return const_palette_ref(pal);
            }
#endif

            /**
             * @brief RAII lock guard for texture pixel access
             */
            class lock_guard {
                private:
                    texture* tex;
                    bool locked;

                public:
                    void* pixels = nullptr;
                    int pitch = 0;

                    explicit lock_guard(texture& t, std::nullopt_t = std::nullopt)
                        : tex(&t), locked(false) {
                        if (auto lock_result = tex->lock(std::nullopt)) {
                            std::tie(pixels, pitch) = *lock_result;
                            locked = true;
                        }
                    }

                    template<rect_like R>
                    explicit lock_guard(texture& t, const R& area)
                        : tex(&t), locked(false) {
                        if (auto lock_result = tex->lock(area)) {
                            std::tie(pixels, pitch) = *lock_result;
                            locked = true;
                        }
                    }

                    template<rect_like R>
                    explicit lock_guard(texture& t, const std::optional <R>& area)
                        : tex(&t), locked(false) {
                        if (auto lock_result = tex->lock(area)) {
                            std::tie(pixels, pitch) = *lock_result;
                            locked = true;
                        }
                    }

                    ~lock_guard() {
                        if (locked) {
                            tex->unlock();
                        }
                    }

                    [[nodiscard]] bool is_locked() const { return locked; }

                    // Non-copyable, non-movable
                    lock_guard(const lock_guard&) = delete;
                    lock_guard& operator=(const lock_guard&) = delete;
                    lock_guard(lock_guard&&) = delete;
                    lock_guard& operator=(lock_guard&&) = delete;
            };

            // Static factory methods

            /**
             * @brief Create a texture
             * @param renderer Renderer to create texture for
             * @param format Pixel format
             * @param access Access pattern
             * @param width Texture width
             * @param height Texture height
             * @return Expected containing new texture, or error message
             */
            static expected <texture, std::string> create(
                const renderer& renderer,
                pixel_format_enum format,
                texture_access access,
                int width, int height) {
                if (!renderer) {
                    return make_unexpectedf("Invalid renderer");
                }

                SDL_Texture* t = SDL_CreateTexture(
                    renderer.get(),
                    static_cast <SDL_PixelFormat>(format),
                    static_cast <SDL_TextureAccess>(access),
                    width, height
                );

                if (!t) {
                    return make_unexpectedf(get_error());
                }

                return texture(t);
            }

            /**
             * @brief Create a texture from a surface
             * @param renderer Renderer to create texture for
             * @param surface Surface to create texture from
             * @return Expected containing new texture, or error message
             */
            static expected <texture, std::string> create(
                const renderer& renderer,
                const surface& surface) {
                if (!renderer) {
                    return make_unexpectedf("Invalid renderer");
                }

                if (!surface) {
                    return make_unexpectedf("Invalid surface");
                }

                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer.get(), surface.get());
                if (!t) {
                    return make_unexpectedf(get_error());
                }

                return texture(t);
            }

            class blend_mode_guard {
                private:
                    texture& t_;
                    blend_mode prev_mode_;
                public:
                    blend_mode_guard(texture& t, blend_mode mode) : t_(t) {
                        auto prev = t_.get_blend_mode();
                        prev_mode_ = prev ? *prev : blend_mode::none;
                        t_.set_blend_mode(mode);
                    }
                    ~blend_mode_guard() {
                        t_.set_blend_mode(prev_mode_);
                    }
                    blend_mode_guard(const blend_mode_guard&) = delete;
                    blend_mode_guard& operator=(const blend_mode_guard&) = delete;
                    blend_mode_guard(blend_mode_guard&&) = delete;
                    blend_mode_guard& operator=(blend_mode_guard&&) = delete;
            };

            class color_mod_guard {
                private:
                    texture& t_;
                    color prev_color_;
                public:
                    color_mod_guard(texture& t, const color& c) : t_(t) {
                        auto prev = t_.get_color_mod();
                        prev_color_ = prev ? *prev : color{255, 255, 255, 255};
                        t_.set_color_mod(c);
                    }
                    ~color_mod_guard() {
                        t_.set_color_mod(prev_color_);
                    }
                    color_mod_guard(const color_mod_guard&) = delete;
                    color_mod_guard& operator=(const color_mod_guard&) = delete;
                    color_mod_guard(color_mod_guard&&) = delete;
                    color_mod_guard& operator=(color_mod_guard&&) = delete;
            };

            class alpha_mod_guard {
                private:
                    texture& t_;
                    uint8_t prev_alpha_;
                public:
                    alpha_mod_guard(texture& t, uint8_t alpha) : t_(t) {
                        auto prev = t_.get_alpha_mod();
                        prev_alpha_ = prev ? *prev : 255;
                        t_.set_alpha_mod(alpha);
                    }
                    ~alpha_mod_guard() {
                        t_.set_alpha_mod(prev_alpha_);
                    }
                    alpha_mod_guard(const alpha_mod_guard&) = delete;
                    alpha_mod_guard& operator=(const alpha_mod_guard&) = delete;
                    alpha_mod_guard(alpha_mod_guard&&) = delete;
                    alpha_mod_guard& operator=(alpha_mod_guard&&) = delete;
            };

            class scale_mode_guard {
                private:
                    texture& t_;
                    scale_mode prev_mode_;
                public:
                    scale_mode_guard(texture& t, scale_mode mode) : t_(t) {
                        auto prev = t_.get_scale_mode();
                        prev_mode_ = prev ? *prev : scale_mode::linear;
                        t_.set_scale_mode(mode);
                    }
                    ~scale_mode_guard() {
                        t_.set_scale_mode(prev_mode_);
                    }
                    scale_mode_guard(const scale_mode_guard&) = delete;
                    scale_mode_guard& operator=(const scale_mode_guard&) = delete;
                    scale_mode_guard(scale_mode_guard&&) = delete;
                    scale_mode_guard& operator=(scale_mode_guard&&) = delete;
            };
    };

    // Now add texture-related methods to renderer
    inline expected <void, std::string> renderer::copy(const texture& texture) {
        if (!ptr) {
            return make_unexpectedf("Invalid renderer");
        }

        if (!texture) {
            return make_unexpectedf("Invalid texture");
        }

        if (!SDL_RenderTexture(ptr.get(), texture.get(), nullptr, nullptr)) {
            return make_unexpectedf(get_error());
        }

        return {};
    }

    template<rect_param R1, rect_param R2>
    inline expected <void, std::string> renderer::copy(
        const texture& texture,
        const R1& src_rect,
        const R2& dst_rect) {
        if (!ptr) {
            return make_unexpectedf("Invalid renderer");
        }

        if (!texture) {
            return make_unexpectedf("Invalid texture");
        }

        auto src_opt = detail::to_optional_sdl_frect(src_rect);
        auto dst_opt = detail::to_optional_sdl_frect(dst_rect);

        const SDL_FRect* src_ptr = src_opt ? &*src_opt : nullptr;
        const SDL_FRect* dst_ptr = dst_opt ? &*dst_opt : nullptr;

        if (!SDL_RenderTexture(ptr.get(), texture.get(), src_ptr, dst_ptr)) {
            return make_unexpectedf(get_error());
        }

        return {};
    }

    template<rect_param R1, rect_param R2, point_param P>
    inline expected <void, std::string> renderer::copy_ex(
        const texture& texture,
        const R1& src_rect,
        const R2& dst_rect,
        double angle,
        const P& center,
        flip_mode flip) {
        if (!ptr) {
            return make_unexpectedf("Invalid renderer");
        }

        if (!texture) {
            return make_unexpectedf("Invalid texture");
        }

        auto src_opt = detail::to_optional_sdl_frect(src_rect);
        auto dst_opt = detail::to_optional_sdl_frect(dst_rect);
        auto cnt_opt = detail::to_optional_sdl_fpoint(center);

        const SDL_FRect* src_ptr = src_opt ? &*src_opt : nullptr;
        const SDL_FRect* dst_ptr = dst_opt ? &*dst_opt : nullptr;
        const SDL_FPoint* cnt_ptr = cnt_opt ? &*cnt_opt : nullptr;

        if (!SDL_RenderTextureRotated(ptr.get(), texture.get(),
                                      src_ptr, dst_ptr,
                                      angle, cnt_ptr,
                                      static_cast <SDL_FlipMode>(flip))) {
            return make_unexpectedf(get_error());
        }

        return {};
    }

    template<rect_param R1, rect_like R2>
    inline expected<void, std::string> renderer::copy_9grid_tiled(
        const texture& texture,
        const R1& src_rect,
        float left_width, float right_width,
        float top_height, float bottom_height,
        float scale,
        const R2& dst_rect,
        float tile_scale) {
        if (!ptr) {
            return make_unexpectedf("Invalid renderer");
        }

        if (!texture) {
            return make_unexpectedf("Invalid texture");
        }

        auto src_opt = detail::to_optional_sdl_frect(src_rect);
        const SDL_FRect* src_ptr = src_opt ? &*src_opt : nullptr;

        SDL_FRect dst = detail::to_sdl_frect(dst_rect);

        if (!SDL_RenderTexture9GridTiled(ptr.get(), texture.get(),
                                         src_ptr,
                                         left_width, right_width,
                                         top_height, bottom_height,
                                         scale, &dst, tile_scale)) {
            return make_unexpectedf(get_error());
        }

        return {};
    }

    inline expected <texture, std::string> renderer::get_target() const {
        if (!ptr) {
            return make_unexpectedf("Invalid renderer");
        }

        SDL_Texture* target = SDL_GetRenderTarget(ptr.get());
        if (!target) {
            // No target is not an error - it means rendering to default target
            return texture();
        }

        // We don't own this texture, but we need to be careful about lifetime
        return texture(target);
    }

    inline expected <void, std::string> renderer::set_target(const texture& target) {
        if (!ptr) {
            return make_unexpectedf("Invalid renderer");
        }

        // nullptr means render to default target (window)
        SDL_Texture* tex_ptr = target ? target.get() : nullptr;

        if (!SDL_SetRenderTarget(ptr.get(), tex_ptr)) {
            return make_unexpectedf(get_error());
        }

        return {};
    }

    class renderer::target_guard {
        private:
            renderer& r_;
            SDL_Texture* prev_target_;
        public:
            target_guard(renderer& r, const texture& target) : r_(r) {
                prev_target_ = SDL_GetRenderTarget(r_.get());
                r_.set_target(target);
            }
            ~target_guard() {
                SDL_SetRenderTarget(r_.get(), prev_target_);
            }
            target_guard(const target_guard&) = delete;
            target_guard& operator=(const target_guard&) = delete;
            target_guard(target_guard&&) = delete;
            target_guard& operator=(target_guard&&) = delete;
    };
} // namespace sdlpp
