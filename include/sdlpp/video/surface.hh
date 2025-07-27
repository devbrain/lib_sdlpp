//
// Created by igor on 7/13/25.
//

#pragma once

/**
 * @file surface.hh
 * @brief Modern C++ wrapper for SDL3 surface functionality
 * 
 * This header provides RAII-managed wrappers around SDL3's surface system,
 * which represents images in system memory. Surfaces can be created, loaded,
 * manipulated, and converted between different pixel formats.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/video/pixels.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/io/iostream.hh>
#include <sdlpp/video/palette.hh>
#include <sdlpp/video/blend_mode.hh>
#include <string>
#include <optional>
#include <sdlpp/utility/dimension.hh>

namespace sdlpp {
    /**
     * @brief Smart pointer type for SDL_Surface with automatic cleanup
     */
    using surface_ptr = pointer <SDL_Surface, SDL_DestroySurface>;

    /**
     * @brief RAII wrapper for SDL_Surface
     *
     * This class provides a safe, RAII-managed interface to SDL's surface
     * functionality. Surfaces are automatically freed when the object
     * goes out of scope.
     *
     * Example usage:
     * @code
     * auto surf_result = surface::create_rgb(800, 600, pixel_format_enum::RGBA8888);
     * if (surf_result) {
     *     auto& surf = *surf_result;
     *     surf.fill(colors::blue);
     *     // Surface is automatically freed when surf goes out of scope
     * }
     * @endcode
     */
    class surface {
        private:
            surface_ptr ptr;
            
            // Fast pixel access function pointers
            void (*put_pixel_fast)(void* pixels, int pitch, int x, int y, uint32_t pixel) = nullptr;
            uint32_t (*get_pixel_fast)(const void* pixels, int pitch, int x, int y) = nullptr;
            
            SDLPP_EXPORT void setup_pixel_functions();

        public:
            /**
             * @brief Default constructor - creates an empty surface
             */
            surface() = default;

            /**
             * @brief Construct from existing SDL_Surface pointer
             * @param surf Existing SDL_Surface pointer (takes ownership)
             */
            explicit surface(SDL_Surface* surf)
                : ptr(surf) {
                if (ptr) {
                    setup_pixel_functions();
                }
            }

            /**
             * @brief Move constructor
             */
            surface(surface&&) = default;

            /**
             * @brief Move assignment operator
             */
            surface& operator=(surface&&) = default;

            // Delete copy operations - surfaces are move-only
            surface(const surface&) = delete;
            surface& operator=(const surface&) = delete;

            /**
             * @brief Check if the surface is valid
             * @return true if the surface is valid, false otherwise
             */
            [[nodiscard]] bool is_valid() const { return ptr != nullptr; }

            /**
             * @brief Check if the surface is valid (conversion operator)
             */
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Surface pointer
             * @return Raw SDL_Surface pointer (does not transfer ownership)
             */
            [[nodiscard]] SDL_Surface* get() const { return ptr.get(); }

            /**
             * @brief Get surface dimensions
             * @tparam S Size type to return (defaults to built-in size if available)
             * @return Surface size
             */
            template<size_like S = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                size_i
#else
                void
#endif
            >
            [[nodiscard]] S dimensions() const
                requires (!std::is_void_v<S>) {
                if (!ptr) return S{0, 0};
                return S{ptr->w, ptr->h};
            }

            /**
             * @brief Get surface width
             * @return Width in pixels
             */
            [[nodiscard]] size_t width() const {
                return (ptr && ptr->w > 0) ? static_cast<size_t>(ptr->w) : 0;
            }

            /**
             * @brief Get surface height
             * @return Height in pixels
             */
            [[nodiscard]] size_t height() const {
                return (ptr && ptr->h > 0) ? static_cast<size_t>(ptr->h) : 0;
            }

            /**
             * @brief Get surface pitch (bytes per row)
             * @return Pitch in bytes
             */
            [[nodiscard]] size_t pitch() const {
                return (ptr && ptr->pitch > 0) ? static_cast<size_t>(ptr->pitch) : 0;
            }

            /**
             * @brief Get pixel format
             * @return Pixel format enum
             */
            [[nodiscard]] pixel_format_enum format() const {
                return ptr
                           ? static_cast <pixel_format_enum>(ptr->format)
                           : pixel_format_enum::unknown;
            }

            // Note: pixels() methods commented out due to std::span issues in some environments
            // TODO: Uncomment when span is properly available

            // /**
            //  * @brief Get direct access to pixel data
            //  * @return Span of pixel data bytes
            //  * @warning The surface must be locked before accessing pixels
            //  */
            // [[nodiscard]] std::span<uint8_t> pixels() {
            //     if (!ptr || !ptr->pixels) return {};
            //     return {static_cast<uint8_t*>(ptr->pixels),
            //             static_cast<size_t>(ptr->h * ptr->pitch)};
            // }

            // /**
            //  * @brief Get read-only access to pixel data
            //  * @return Span of pixel data bytes
            //  * @warning The surface must be locked before accessing pixels
            //  */
            // [[nodiscard]] std::span<const uint8_t> pixels() const {
            //     if (!ptr || !ptr->pixels) return {};
            //     return {static_cast<const uint8_t*>(ptr->pixels),
            //             static_cast<size_t>(ptr->h * ptr->pitch)};
            // }

            /**
             * @brief Lock surface for direct pixel access
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> lock() {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }
                if (SDL_LockSurface(ptr.get()) != 0) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Unlock surface after pixel access
             */
            void unlock() {
                if (ptr) {
                    SDL_UnlockSurface(ptr.get());
                }
            }

            /**
             * @brief RAII lock guard for surface pixel access
             */
            class lock_guard {
                private:
                    surface* surf;
                    bool locked;

                public:
                    explicit lock_guard(surface& s)
                        : surf(&s), locked(false) {
                        if (auto lock_result = surf->lock()) {
                            locked = true;
                        }
                    }

                    ~lock_guard() {
                        if (locked) {
                            surf->unlock();
                        }
                    }

                    [[nodiscard]] bool is_locked() const { return locked; }

                    // Non-copyable, non-movable
                    lock_guard(const lock_guard&) = delete;
                    lock_guard& operator=(const lock_guard&) = delete;
                    lock_guard(lock_guard&&) = delete;
                    lock_guard& operator=(lock_guard&&) = delete;
            };

            /**
             * @brief Fill the entire surface with a color
             * @param c Color to fill with
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> fill(const color& c) {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                uint32_t mapped = SDL_MapSurfaceRGBA(ptr.get(), c.r, c.g, c.b, c.a);

                if (!SDL_FillSurfaceRect(ptr.get(), nullptr, mapped)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Fill a rectangle with a color
             * @param area Rectangle to fill
             * @param c Color to fill with
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R>
            expected <void, std::string> fill_rect(const R& area, const color& c) {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                uint32_t mapped = SDL_MapSurfaceRGBA(ptr.get(), c.r, c.g, c.b, c.a);

                SDL_Rect sdl_rect{
                    static_cast <int>(get_x(area)), static_cast <int>(get_y(area)),
                    static_cast <int>(get_width(area)), static_cast <int>(get_height(area))
                };
                if (!SDL_FillSurfaceRect(ptr.get(), &sdl_rect, mapped)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get pixel color at specified coordinates
             * @param x X coordinate
             * @param y Y coordinate
             * @return Expected containing color at (x,y), or error message
             * @note The surface must be locked before calling this method
             */
            expected <color, std::string> get_pixel(int x, int y) const {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                // Bounds check
                if (x < 0 || x >= ptr->w || y < 0 || y >= ptr->h) {
                    return make_unexpected("Coordinates out of bounds");
                }

                // Check if surface is locked
                if (!SDL_MUSTLOCK(ptr.get()) || SDL_SurfaceHasRLE(ptr.get())) {
                    // Surface doesn't require locking or has RLE encoding
                } else if (!ptr->pixels) {
                    return make_unexpected("Surface must be locked before accessing pixels");
                }

                // Get pixel based on format
                const auto* pixels = static_cast <const uint8_t*>(ptr->pixels);
                const int bpp = SDL_BYTESPERPIXEL(ptr->format);
                const uint8_t* p = pixels + y * ptr->pitch + x * bpp;

                uint32_t pixel = 0;
                switch (bpp) {
                    case 1:
                        pixel = *p;
                        break;
                    case 2:
                        pixel = *reinterpret_cast <const uint16_t*>(p);
                        break;
                    case 3:
                        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                            pixel = p[0] << 16 | p[1] << 8 | p[2];
                        } else {
                            pixel = p[0] | p[1] << 8 | p[2] << 16;
                        }
                        break;
                    case 4:
                        pixel = *reinterpret_cast <const uint32_t*>(p);
                        break;
                    default:
                        return make_unexpected("Unsupported pixel format");
                }

                // Convert to RGBA
                uint8_t r, g, b, a;
                SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(ptr->format),
                            nullptr, &r, &g, &b, &a);

                return color{r, g, b, a};
            }

            /**
             * @brief Set pixel color at specified coordinates
             * @param x X coordinate
             * @param y Y coordinate
             * @param c Color to set
             * @return Expected<void> - empty on success, error message on failure
             * @note The surface must be locked before calling this method
             */
            expected <void, std::string> put_pixel(int x, int y, const color& c) {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                // Bounds check
                if (x < 0 || x >= ptr->w || y < 0 || y >= ptr->h) {
                    return make_unexpected("Coordinates out of bounds");
                }

                // Check if surface is locked
                if (!SDL_MUSTLOCK(ptr.get()) || SDL_SurfaceHasRLE(ptr.get())) {
                    // Surface doesn't require locking or has RLE encoding
                } else if (!ptr->pixels) {
                    return make_unexpected("Surface must be locked before accessing pixels");
                }

                // Map color to pixel format
                uint32_t pixel = SDL_MapSurfaceRGBA(ptr.get(), c.r, c.g, c.b, c.a);

                // Set pixel based on format
                auto* pixels = static_cast <uint8_t*>(ptr->pixels);
                const int bpp = SDL_BYTESPERPIXEL(ptr->format);
                uint8_t* p = pixels + y * ptr->pitch + x * bpp;

                switch (bpp) {
                    case 1:
                        *p = static_cast <uint8_t>(pixel);
                        break;
                    case 2:
                        *reinterpret_cast <uint16_t*>(p) = static_cast <uint16_t>(pixel);
                        break;
                    case 3:
                        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                            p[0] = (pixel >> 16) & 0xff;
                            p[1] = (pixel >> 8) & 0xff;
                            p[2] = pixel & 0xff;
                        } else {
                            p[0] = pixel & 0xff;
                            p[1] = (pixel >> 8) & 0xff;
                            p[2] = (pixel >> 16) & 0xff;
                        }
                        break;
                    case 4:
                        *reinterpret_cast <uint32_t*>(p) = pixel;
                        break;
                    default:
                        return make_unexpected("Unsupported pixel format");
                }

                return {};
            }

            /**
             * @brief Get pixel color at specified point
             * @tparam P Point type (must satisfy point_like)
             * @param p Point coordinates
             * @return Expected containing color at point, or error message
             * @note The surface must be locked before calling this method
             */
            template<point_like P>
            expected <color, std::string> get_pixel(const P& p) const {
                return get_pixel(static_cast<int>(get_x(p)), static_cast<int>(get_y(p)));
            }

            /**
             * @brief Set pixel color at specified point
             * @tparam P Point type (must satisfy point_like)
             * @param p Point coordinates
             * @param c Color to set
             * @return Expected<void> - empty on success, error message on failure
             * @note The surface must be locked before calling this method
             */
            template<point_like P>
            expected <void, std::string> put_pixel(const P& p, const color& c) {
                return put_pixel(static_cast<int>(get_x(p)), static_cast<int>(get_y(p)), c);
            }

            /**
             * @brief Set blend mode
             * @param mode Blend mode to set (defaults to none)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_blend_mode(blend_mode mode = blend_mode::none) {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                if (!SDL_SetSurfaceBlendMode(ptr.get(), static_cast <SDL_BlendMode>(mode))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get current blend mode
             * @return Expected containing blend mode, or error message
             */
            expected <blend_mode, std::string> get_blend_mode() const {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                SDL_BlendMode mode;
                if (!SDL_GetSurfaceBlendMode(ptr.get(), &mode)) {
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
                    return make_unexpected("Invalid surface");
                }

                if (!SDL_SetSurfaceColorMod(ptr.get(), c.r, c.g, c.b)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Set alpha modulation
             * @param alpha Alpha value (0-255)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_alpha_mod(uint8_t alpha) {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                if (!SDL_SetSurfaceAlphaMod(ptr.get(), alpha)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Convert surface to a different pixel format
             * @param format Target pixel format
             * @return Expected containing new surface, or error message
             */
            expected <surface, std::string> convert(pixel_format_enum format) const {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                SDL_Surface* converted = SDL_ConvertSurface(ptr.get(),
                                                            static_cast <SDL_PixelFormat>(format));
                if (!converted) {
                    return make_unexpected(get_error());
                }

                return surface(converted);
            }

            /**
             * @brief Create a duplicate of this surface
             * @return Expected containing duplicated surface, or error message
             */
            expected <surface, std::string> duplicate() const {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                SDL_Surface* dup = SDL_DuplicateSurface(ptr.get());
                if (!dup) {
                    return make_unexpected(get_error());
                }

                return surface(dup);
            }

            /**
             * @brief Blit this surface to another surface
             * @tparam R Rectangle type (must satisfy rect_like)
             * @tparam P Point type (must satisfy point_like)
             * @param dst Destination surface
             * @param src_rect Source rectangle (nullopt for entire surface)
             * @param dst_pos Destination position
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R = void, point_like P = void>
            expected <void, std::string> blit_to(
                surface& dst,
                std::optional <R> src_rect = std::nullopt,
                P dst_pos = P{0, 0}) const {
                if (!ptr || !dst.ptr) {
                    return make_unexpected("Invalid surface");
                }

                SDL_Rect src_r;
                SDL_Rect* src_ptr = nullptr;
                if (src_rect) {
                    src_r = SDL_Rect{
                        static_cast<int>(get_x(*src_rect)),
                        static_cast<int>(get_y(*src_rect)),
                        static_cast<int>(get_width(*src_rect)),
                        static_cast<int>(get_height(*src_rect))
                    };
                    src_ptr = &src_r;
                }

                SDL_Rect dst_r = {static_cast<int>(get_x(dst_pos)), 
                                  static_cast<int>(get_y(dst_pos)), 0, 0};

                if (!SDL_BlitSurface(ptr.get(), src_ptr, dst.ptr.get(), &dst_r)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Scaled blit to another surface
             * @tparam R Rectangle type (must satisfy rect_like)
             * @param dst Destination surface
             * @param src_rect Source rectangle (nullopt for entire surface)
             * @param dst_rect Destination rectangle (nullopt for entire surface)
             * @param mode Scale mode to use
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R = void>
            expected <void, std::string> blit_scaled_to(
                surface& dst,
                std::optional <R> src_rect = std::nullopt,
                std::optional <R> dst_rect = std::nullopt,
                scale_mode mode = scale_mode::linear) const {
                if (!ptr || !dst.ptr) {
                    return make_unexpected("Invalid surface");
                }

                SDL_Rect src_r, dst_r;
                SDL_Rect* src_ptr = nullptr;
                SDL_Rect* dst_ptr = nullptr;

                if (src_rect) {
                    src_r = SDL_Rect{
                        static_cast<int>(get_x(*src_rect)),
                        static_cast<int>(get_y(*src_rect)),
                        static_cast<int>(get_width(*src_rect)),
                        static_cast<int>(get_height(*src_rect))
                    };
                    src_ptr = &src_r;
                }

                if (dst_rect) {
                    dst_r = SDL_Rect{
                        static_cast<int>(get_x(*dst_rect)),
                        static_cast<int>(get_y(*dst_rect)),
                        static_cast<int>(get_width(*dst_rect)),
                        static_cast<int>(get_height(*dst_rect))
                    };
                    dst_ptr = &dst_r;
                }

                if (!SDL_BlitSurfaceScaled(ptr.get(), src_ptr, dst.ptr.get(), dst_ptr,
                                           static_cast <SDL_ScaleMode>(mode))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            // Static factory functions

            /**
             * @brief Create an RGB surface with any dimensions-like type
             * @param dims Surface dimensions (must satisfy dimensions_like concept)
             * @param format Pixel format
             * @return Expected containing new surface, or error message
             */
            template<dimensions_like D>
            static expected <surface, std::string> create_rgb(
                const D& dims,
                pixel_format_enum format) {
                auto [w, h] = to_sdl_dimensions(sdlpp::dimensions <int>(
                    static_cast <int>(dims.width.value()),
                    static_cast <int>(dims.height.value())
                ));

                SDL_Surface* surf = SDL_CreateSurface(w, h,
                                                      static_cast <SDL_PixelFormat>(format));
                if (!surf) {
                    return make_unexpected(get_error());
                }

                return surface(surf);
            }

            /**
             * @brief Create an RGB surface (compatibility overload)
             * @param width Surface width
             * @param height Surface height
             * @param format Pixel format
             * @return Expected containing new surface, or error message
             * @note Negative dimensions will be clamped to 0
             */
            static expected <surface, std::string> create_rgb(
                int width, int height,
                pixel_format_enum format) {
                return create_rgb(sdlpp::dimensions <int>(width, height), format);
            }

            /**
             * @brief Create a surface from existing pixel data
             * @param pixels Pixel data
             * @param width Surface width
             * @param height Surface height
             * @param pitch Bytes per row
             * @param format Pixel format
             * @return Expected containing new surface, or error message
             * @note The surface does not own the pixel data
             */
            static expected <surface, std::string> create_from_pixels(
                void* pixels, int width, int height, int pitch,
                pixel_format_enum format) {
                SDL_Surface* surf = SDL_CreateSurfaceFrom(width, height,
                                                          static_cast <SDL_PixelFormat>(format),
                                                          pixels, pitch);
                if (!surf) {
                    return make_unexpected(get_error());
                }

                return surface(surf);
            }

            /**
             * @brief Get the surface's palette (if any) - const version
             * @return Non-owning const reference to palette, or invalid reference if no palette
             * @note The returned const_palette_ref does NOT own the palette - the surface does
             */
            [[nodiscard]] const_palette_ref get_palette() const {
                if (!ptr) return {};
                return const_palette_ref(SDL_GetSurfacePalette(ptr.get()));
            }

            /**
             * @brief Get the surface's palette (if any) - mutable version
             * @return Non-owning mutable reference to palette, or invalid reference if no palette
             * @note The returned palette_ref does NOT own the palette - the surface does
             * @warning Modifying the palette will affect the surface's appearance
             */
            [[nodiscard]] palette_ref get_palette() {
                if (!ptr) return {};
                return palette_ref(SDL_GetSurfacePalette(ptr.get()));
            }

            /**
             * @brief Check if surface has a palette
             * @return true if surface has a palette
             */
            [[nodiscard]] bool has_palette() const {
                return get_palette().is_valid();
            }

            /**
             * @brief Set a palette for the surface
             * @param pal Palette to set (can be owning palette, palette_ref, or const_palette_ref)
             * @return Expected<void> - empty on success, error message on failure
             * @note The surface will reference the palette but not own it
             */
            expected <void, std::string> set_palette(const const_palette_ref& pal) {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }
                if (!pal) {
                    return make_unexpected("Invalid palette");
                }

                // SDL_SetSurfacePalette needs non-const pointer, but doesn't modify
                if (!SDL_SetSurfacePalette(ptr.get(), const_cast <SDL_Palette*>(pal.get()))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Save surface to BMP format using an iostream
             * @param stream Output stream to write to
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> save_bmp(iostream& stream) const {
                if (!ptr) {
                    return make_unexpected("Invalid surface");
                }

                if (!SDL_SaveBMP_IO(ptr.get(), stream.get(), false)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Load surface from BMP format using an iostream
             * @param stream Input stream to read from
             * @return Expected containing loaded surface, or error message
             */
            static expected <surface, std::string> load_bmp(iostream& stream) {
                SDL_Surface* surf = SDL_LoadBMP_IO(stream.get(), false);
                if (!surf) {
                    return make_unexpected(get_error());
                }

                return surface(surf);
            }
            
            /**
             * @brief Get fast pixel put function pointer
             * @return Function pointer for fast pixel writing, or nullptr if not available
             */
            [[nodiscard]] auto get_put_pixel_fast() const {
                return put_pixel_fast;
            }
            
            /**
             * @brief Get fast pixel get function pointer
             * @return Function pointer for fast pixel reading, or nullptr if not available
             */
            [[nodiscard]] auto get_get_pixel_fast() const {
                return get_pixel_fast;
            }
            
            /**
             * @brief Get raw pixel data pointer
             * @return Pointer to pixel data, or nullptr if surface is invalid
             */
            [[nodiscard]] void* get_pixels() const {
                return ptr ? ptr->pixels : nullptr;
            }
            
            /**
             * @brief Get pitch (bytes per row)
             * @return Pitch value, or 0 if surface is invalid
             */
            [[nodiscard]] int get_pitch() const {
                return ptr ? ptr->pitch : 0;
            }
    };

    /**
     * @brief Load surface from file
     * @param file Path to image file
     * @return Expected containing loaded surface, or error message
     * @note Requires SDL_image to be initialized for formats other than BMP
     */
    inline expected <surface, std::string> load_surface(const std::string& file) {
        SDL_Surface* surf = SDL_LoadBMP(file.c_str());
        if (!surf) {
            return make_unexpected(get_error());
        }

        return surface(surf);
    }

    /**
     * @brief Save surface to BMP file
     * @param surf Surface to save
     * @param file Path to output file
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> save_bmp(const surface& surf,
                                                 const std::string& file) {
        if (!surf) {
            return make_unexpected("Invalid surface");
        }

        if (!SDL_SaveBMP(surf.get(), file.c_str())) {
            return make_unexpected(get_error());
        }

        return {};
    }

    /**
     * @brief Save surface to BMP format in a standard output stream
     * @param surf Surface to save
     * @param stream Standard output stream to write BMP data to
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> save_bmp(const surface& surf,
                                                 std::ostream& stream) {
        if (!surf) {
            return make_unexpected("Invalid surface");
        }

        auto io_result = from_ostream(stream);
        if (!io_result) {
            return make_unexpected(io_result.error());
        }

        return surf.save_bmp(*io_result);
    }

    /**
     * @brief Save surface to BMP format in memory
     * @param surf Surface to save
     * @return Expected containing vector with BMP data, or error message
     */
    inline expected <std::vector <uint8_t>, std::string> save_bmp(const surface& surf) {
        if (!surf) {
            return make_unexpected("Invalid surface");
        }

        // Create a dynamic memory stream
        auto io_result = from_dynamic_memory();
        if (!io_result) {
            return make_unexpected(io_result.error());
        }

        // Save to the stream
        auto save_result = surf.save_bmp(*io_result);
        if (!save_result) {
            return make_unexpected(save_result.error());
        }

        // Get the size of the data
        auto size_result = io_result->size();
        if (!size_result) {
            return make_unexpected(size_result.error());
        }

        // Read all the data
        auto seek_result = io_result->seek(0, io_seek_pos::set);
        if (!seek_result) {
            return make_unexpected(seek_result.error());
        }

        return io_result->read(static_cast <size_t>(*size_result));
    }

    /**
     * @brief Load surface from BMP data in a standard input stream
     * @param stream Standard input stream containing BMP data
     * @return Expected containing loaded surface, or error message
     */
    inline expected <surface, std::string> load_bmp(std::istream& stream) {
        auto io_result = from_istream(stream);
        if (!io_result) {
            return make_unexpected(io_result.error());
        }

        return surface::load_bmp(*io_result);
    }

    /**
     * @brief Load surface from BMP data in memory
     * @param data Memory buffer containing BMP data
     * @param size Size of the buffer in bytes
     * @return Expected containing loaded surface, or error message
     */
    inline expected <surface, std::string> load_bmp(const void* data, size_t data_size) {
        auto io_result = from_memory(data, data_size);
        if (!io_result) {
            return make_unexpected(io_result.error());
        }

        return surface::load_bmp(*io_result);
    }
} // namespace sdlpp
