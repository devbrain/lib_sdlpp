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
#include <sdlpp/video/color.hh>
#include <sdlpp/video/pixels.hh>
#include <sdlpp/video/blend_mode.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/surface.hh>
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
                    return make_unexpected("Invalid texture");
                }

                SDL_PropertiesID props = SDL_GetTextureProperties(ptr.get());
                if (!props) {
                    return make_unexpected(get_error());
                }

                return props;
            }

            /**
             * @brief Get texture size
             * @return Expected containing size, or error message
             */
            [[nodiscard]] expected <size_i, std::string> get_size() const {
                if (!ptr) {
                    return make_unexpected("Invalid texture");
                }

                float w, h;
                if (!SDL_GetTextureSize(ptr.get(), &w, &h)) {
                    return make_unexpected(get_error());
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
                    return make_unexpected("Invalid texture");
                }

                if (!SDL_SetTextureBlendMode(ptr.get(), static_cast <SDL_BlendMode>(mode))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get blend mode
             * @return Expected containing blend mode, or error message
             */
            [[nodiscard]] expected <blend_mode, std::string> get_blend_mode() const {
                if (!ptr) {
                    return make_unexpected("Invalid texture");
                }

                SDL_BlendMode mode;
                if (!SDL_GetTextureBlendMode(ptr.get(), &mode)) {
                    return make_unexpected(get_error());
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
                    return make_unexpected("Invalid texture");
                }

                if (!SDL_SetTextureColorMod(ptr.get(), c.r, c.g, c.b)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get color modulation
             * @return Expected containing color, or error message
             */
            [[nodiscard]] expected <color, std::string> get_color_mod() const {
                if (!ptr) {
                    return make_unexpected("Invalid texture");
                }

                uint8_t r, g, b;
                if (!SDL_GetTextureColorMod(ptr.get(), &r, &g, &b)) {
                    return make_unexpected(get_error());
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
                    return make_unexpected("Invalid texture");
                }

                if (!SDL_SetTextureAlphaMod(ptr.get(), alpha)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get alpha modulation
             * @return Expected containing alpha value, or error message
             */
            [[nodiscard]] expected <uint8_t, std::string> get_alpha_mod() const {
                if (!ptr) {
                    return make_unexpected("Invalid texture");
                }

                uint8_t alpha;
                if (!SDL_GetTextureAlphaMod(ptr.get(), &alpha)) {
                    return make_unexpected(get_error());
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
                    return make_unexpected("Invalid texture");
                }

                if (!SDL_SetTextureScaleMode(ptr.get(), static_cast <SDL_ScaleMode>(mode))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get scale mode
             * @return Expected containing scale mode, or error message
             */
            [[nodiscard]] expected <scale_mode, std::string> get_scale_mode() const {
                if (!ptr) {
                    return make_unexpected("Invalid texture");
                }

                SDL_ScaleMode mode;
                if (!SDL_GetTextureScaleMode(ptr.get(), &mode)) {
                    return make_unexpected(get_error());
                }

                return static_cast <scale_mode>(mode);
            }

            /**
             * @brief Update texture with new pixel data
             * @param rect Area to update (nullopt for entire texture)
             * @param pixels Pixel data
             * @param pitch Number of bytes per row
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R = void>
            expected <void, std::string> update(const std::optional <R>& update_rect,
                                                const void* pixels, int pitch) {
                if (!ptr) {
                    return make_unexpected("Invalid texture");
                }

                if (!pixels) {
                    return make_unexpected("Invalid pixel data");
                }

                if (update_rect) {
                    SDL_Rect sdl_rect{
                        static_cast<int>(get_x(*update_rect)),
                        static_cast<int>(get_y(*update_rect)),
                        static_cast<int>(get_width(*update_rect)),
                        static_cast<int>(get_height(*update_rect))
                    };
                    if (!SDL_UpdateTexture(ptr.get(), &sdl_rect, pixels, pitch)) {
                        return make_unexpected(get_error());
                    }
                } else {
                    if (!SDL_UpdateTexture(ptr.get(), nullptr, pixels, pitch)) {
                        return make_unexpected(get_error());
                    }
                }

                return {};
            }

            /**
             * @brief Lock texture for direct pixel access
             * @param rect Area to lock (nullopt for entire texture)
             * @return Expected containing pixels pointer and pitch, or error message
             * @note Only works for streaming textures
             */
            template<rect_like R = void>
            expected <std::pair <void*, int>, std::string> lock(const std::optional <R>& lock_rect = std::nullopt) {
                if (!ptr) {
                    return make_unexpected("Invalid texture");
                }

                void* pixels;
                int pitch;

                if (lock_rect) {
                    SDL_Rect sdl_rect{
                        static_cast<int>(get_x(*lock_rect)),
                        static_cast<int>(get_y(*lock_rect)),
                        static_cast<int>(get_width(*lock_rect)),
                        static_cast<int>(get_height(*lock_rect))
                    };
                    if (!SDL_LockTexture(ptr.get(), &sdl_rect, &pixels, &pitch)) {
                        return make_unexpected(get_error());
                    }
                } else {
                    if (!SDL_LockTexture(ptr.get(), nullptr, &pixels, &pitch)) {
                        return make_unexpected(get_error());
                    }
                }

                return std::make_pair(pixels, pitch);
            }

            /**
             * @brief Unlock texture after pixel access
             */
            void unlock() {
                if (ptr) {
                    SDL_UnlockTexture(ptr.get());
                }
            }

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

                    template<rect_like R = void>
                    explicit lock_guard(texture& t, const std::optional <R>& area = std::nullopt)
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
                    return make_unexpected("Invalid renderer");
                }

                SDL_Texture* t = SDL_CreateTexture(
                    renderer.get(),
                    static_cast <SDL_PixelFormat>(format),
                    static_cast <SDL_TextureAccess>(access),
                    width, height
                );

                if (!t) {
                    return make_unexpected(get_error());
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
                    return make_unexpected("Invalid renderer");
                }

                if (!surface) {
                    return make_unexpected("Invalid surface");
                }

                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer.get(), surface.get());
                if (!t) {
                    return make_unexpected(get_error());
                }

                return texture(t);
            }
    };

    // Now add texture-related methods to renderer
    template<rect_like R>
    inline expected <void, std::string> renderer::copy(
        const texture& texture,
        const std::optional <R>& src_rect,
        const std::optional <R>& dst_rect) {
        if (!ptr) {
            return make_unexpected("Invalid renderer");
        }

        if (!texture) {
            return make_unexpected("Invalid texture");
        }

        SDL_FRect src, dst;
        SDL_FRect* src_ptr = nullptr;
        SDL_FRect* dst_ptr = nullptr;

        if (src_rect) {
            src = {
                static_cast <float>(get_x(*src_rect)), static_cast <float>(get_y(*src_rect)),
                static_cast <float>(get_width(*src_rect)), static_cast <float>(get_height(*src_rect))
            };
            src_ptr = &src;
        }

        if (dst_rect) {
            dst = {
                static_cast <float>(get_x(*dst_rect)), static_cast <float>(get_y(*dst_rect)),
                static_cast <float>(get_width(*dst_rect)), static_cast <float>(get_height(*dst_rect))
            };
            dst_ptr = &dst;
        }

        if (!SDL_RenderTexture(ptr.get(), texture.get(), src_ptr, dst_ptr)) {
            return make_unexpected(get_error());
        }

        return {};
    }

    template<rect_like R>
    requires std::is_floating_point_v<typename R::value_type>
    inline expected <void, std::string> renderer::copy(
        const texture& texture,
        const std::optional <R>& src_rect,
        const std::optional <R>& dst_rect) {
        if (!ptr) {
            return make_unexpected("Invalid renderer");
        }

        if (!texture) {
            return make_unexpected("Invalid texture");
        }

        SDL_FRect src, dst;
        SDL_FRect* src_ptr = nullptr;
        SDL_FRect* dst_ptr = nullptr;

        if (src_rect) {
            src = detail::to_sdl_frect(*src_rect);
            src_ptr = &src;
        }

        if (dst_rect) {
            dst = detail::to_sdl_frect(*dst_rect);
            dst_ptr = &dst;
        }

        if (!SDL_RenderTexture(ptr.get(), texture.get(), src_ptr, dst_ptr)) {
            return make_unexpected(get_error());
        }

        return {};
    }

    template<rect_like R, point_like P>
    inline expected <void, std::string> renderer::copy_ex(
        const texture& texture,
        const std::optional <R>& src_rect,
        const std::optional <R>& dst_rect,
        double angle,
        const std::optional <P>& center,
        flip_mode flip) {
        if (!ptr) {
            return make_unexpected("Invalid renderer");
        }

        if (!texture) {
            return make_unexpected("Invalid texture");
        }

        SDL_FRect src, dst;
        SDL_FRect* src_ptr = nullptr;
        SDL_FRect* dst_ptr = nullptr;
        SDL_FPoint cnt;
        SDL_FPoint* cnt_ptr = nullptr;

        if (src_rect) {
            src = detail::to_sdl_frect(*src_rect);
            src_ptr = &src;
        }

        if (dst_rect) {
            dst = detail::to_sdl_frect(*dst_rect);
            dst_ptr = &dst;
        }

        if (center) {
            cnt = detail::to_sdl_fpoint(*center);
            cnt_ptr = &cnt;
        }

        if (!SDL_RenderTextureRotated(ptr.get(), texture.get(),
                                      src_ptr, dst_ptr,
                                      angle, cnt_ptr,
                                      static_cast <SDL_FlipMode>(flip))) {
            return make_unexpected(get_error());
        }

        return {};
    }

    template<rect_like R, point_like P>
    requires (std::is_floating_point_v<typename R::value_type> && 
             std::is_floating_point_v<typename P::value_type>)
    inline expected <void, std::string> renderer::copy_ex(
        const texture& texture,
        const std::optional <R>& src_rect,
        const std::optional <R>& dst_rect,
        double angle,
        const std::optional <P>& center,
        flip_mode flip) {
        if (!ptr) {
            return make_unexpected("Invalid renderer");
        }

        if (!texture) {
            return make_unexpected("Invalid texture");
        }

        SDL_FRect src, dst;
        SDL_FRect* src_ptr = nullptr;
        SDL_FRect* dst_ptr = nullptr;
        SDL_FPoint cnt;
        SDL_FPoint* cnt_ptr = nullptr;

        if (src_rect) {
            src = detail::to_sdl_frect(*src_rect);
            src_ptr = &src;
        }

        if (dst_rect) {
            dst = detail::to_sdl_frect(*dst_rect);
            dst_ptr = &dst;
        }

        if (center) {
            cnt = detail::to_sdl_fpoint(*center);
            cnt_ptr = &cnt;
        }

        if (!SDL_RenderTextureRotated(ptr.get(), texture.get(),
                                      src_ptr, dst_ptr,
                                      angle, cnt_ptr,
                                      static_cast <SDL_FlipMode>(flip))) {
            return make_unexpected(get_error());
        }

        return {};
    }

    inline expected <texture, std::string> renderer::get_target() const {
        if (!ptr) {
            return make_unexpected("Invalid renderer");
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
            return make_unexpected("Invalid renderer");
        }

        // nullptr means render to default target (window)
        SDL_Texture* tex_ptr = target ? target.get() : nullptr;

        if (!SDL_SetRenderTarget(ptr.get(), tex_ptr)) {
            return make_unexpected(get_error());
        }

        return {};
    }
} // namespace sdlpp
